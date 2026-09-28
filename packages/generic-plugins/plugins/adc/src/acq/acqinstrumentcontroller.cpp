/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "acqinstrumentcontroller.h"

#include "acqinstrument.h"
#include "acqplotmanager.h"
#include "filesourceblock.h"
#include "sourceregistry.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <core/acq_engine/SourceBlock.h>
#include <gui/instrumenttemplate.h>
#include <gui/widgets/genalyzerpanel.h>

#include <QMap>
#include <QPointer>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <memory>
#include <vector>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Wraps a block's own settings widget in a menu page with an owner pill, which is what
// every rail row's page is made of.
QWidget *blockPage(InstrumentTemplate *it, scopy::acq::Block *block, const QString &title)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_CH, page);
	section->add(block->settingsWidget(section));
	lay->addWidget(section);
	lay->addStretch();
	return page;
}

// The engine's registered sources are the only record of what setupBlocks() built, so the
// code below asks it rather than holding a pointer per source type.
scopy::acq::SourceBlock *sourceById(scopy::acq::AcquisitionEngine *engine, const QString &id)
{
	for(scopy::acq::SourceBlock *src : engine->sources()) {
		if(src->id() == id) {
			return src;
		}
	}
	return nullptr;
}

} // namespace

AcqInstrumentController::AcqInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_kPlutoSampleRate(2.4e6)
	, m_tme(tme)
{
}

AcqInstrumentController::~AcqInstrumentController() { stop(); }

void AcqInstrumentController::init(iio_context *ctx)
{
	if(m_ui) {
		return;
	}

	// Parentless: the ToolMenuEntry takes it, and the plugin deletes it in
	// deleteInstrument(). Same ownership as the other controllers here.
	m_ui = new AcqInstrument(nullptr);

	setupBlocks(ctx);
	setupProcessors();
	setupPlots();
	// Last: it sits in a slot around the center widget setupPlots() installs.
	setupAnalysisPanel();

	// The tool menu's own run button and the instrument's stay in step. No loop: setRunning()
	// only changes state, so the echo back is a no-op.
	connect(m_tme, &ToolMenuEntry::runToggled, m_ui, [this](bool on) {
		if(on) {
			m_ui->run();
		} else {
			m_ui->stop();
		}
	});
	connect(m_ui, &AcqInstrument::started, this, [this]() { m_tme->setRunning(true); });
	connect(m_ui, &AcqInstrument::stopped, this, [this]() { m_tme->setRunning(false); });
}

void AcqInstrumentController::setupBlocks(iio_context *ctx)
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	MenuSectionCollapseWidget     *sources = m_ui->shell()->addChannelGroup("Sources");

	for(const AcqSourceFactory &make : AcqSourceRegistry::instance().all()) {
		// Constructed to be asked: a source holds the context and decides for itself whether
		// its device is in it. Cheap — an unavailable source's constructor is a no-op.
		std::unique_ptr<scopy::acq::SourceBlock> src(make(ctx, engine));
		if(!src || !src->isAvailable()) {
			continue;
		}
		addBlockRow(sources, src.get(), src->id(), src->id().toUpper(), src->id());
		engine->addSource(src.release());
	}
}

void AcqInstrumentController::setupProcessors()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();

	if(!sourceById(engine, QStringLiteral("pluto"))) {
		return;
	}

	// Two watched keys, so this is the complex path: I on voltage0, Q on voltage1, nfft
	// tied to the engine's buffer size.
	m_fftProc = new scopy::acq::GenalyzerFFTProcessor(scopy::acq::DataKey::raw("pluto", "voltage0"),
							  scopy::acq::DataKey::raw("pluto", "voltage1"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "fft"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "freq"),
							  static_cast<int>(engine->bufferSize()), m_kPlutoSampleRate,
							  GnWindowHann, engine);
	// Lets the block claim chunk history when averaging is turned on: navg frames are navg
	// past chunks.
	m_fftProc->setAveragingStore(m_ui->store());
	engine->addProcessor(m_fftProc);

	MenuSectionCollapseWidget *procs = m_ui->shell()->addChannelGroup("Processors");
	addBlockRow(procs, m_fftProc, QStringLiteral("FFT"), QStringLiteral("GENALYZER FFT"),
		    QStringLiteral("fft"));
}

CollapsableMenuControlButton *AcqInstrumentController::addBlockRow(MenuSectionCollapseWidget *group,
								   scopy::acq::Block *block, const QString &label,
								   const QString &pageTitle, const QString &menuId)
{
	if(!group || !block) {
		return nullptr;
	}
	InstrumentTemplate *it = m_ui->shell();

	// No colour: a coloured swatch on the rail means "this is the curve you see in that
	// colour", and only plot channels have one.
	CollapsableMenuControlButton *row = it->addExpandableChannelRow(group, label, QColor(), menuId);
	it->addMenuPage(menuId, blockPage(it, block, pageTitle));

	// A source's channels hang under the row as a subtree. A block that declares none — every
	// processor, and the snapshot source until a slot is captured — gets an empty one.
	if(auto *src = qobject_cast<scopy::acq::SourceBlock *>(block)) {
		addSourceChannelRows(row, src);
	}

	return row;
}

void AcqInstrumentController::addSourceChannelRows(CollapsableMenuControlButton *parentRow,
						  scopy::acq::SourceBlock *src)
{
	if(!parentRow || !src) {
		return;
	}
	InstrumentTemplate *it = m_ui->shell();

	// id -> its switch, so the source flipping a channel itself can find the row again.
	// Shared rather than a member: both lambdas below outlive this call.
	auto switches = std::make_shared<QMap<QString, QPointer<SmallOnOffSwitch>>>();

	auto rebuild = [this, it, parentRow, src, switches]() {
		const QList<MenuControlButton *> old = parentRow->findChildren<MenuControlButton *>();
		for(MenuControlButton *row : old) {
			// The header is a child too, and removing it would take the whole row's
			// selection and page with it.
			if(row == parentRow->getControlBtn()) {
				continue;
			}
			it->removeChannelRow(parentRow, row, QString());
		}
		switches->clear();

		const QList<QString> ids = src->channelIds();
		for(const QString &id : ids) {
			// No page — the source's own settings cover the whole device — and no
			// colour, because a raw channel is not a curve until someone adds one from
			// the key picker.
			MenuControlButton *row = it->addChannelSwitchRow(parentRow, id, QColor(), QString(), 1);
			SmallOnOffSwitch *sw = InstrumentTemplate::rowSwitch(row);
			if(!sw) {
				continue;
			}
			QSignalBlocker b(sw);
			sw->setChecked(src->isChannelEnabled(id));
			switches->insert(id, sw);
			connect(sw, &QAbstractButton::toggled, src, [src, id](bool en) { src->enableChannel(id, en); });
		}
	};

	rebuild();
	// Queued, both of them: a source can add channels or flip one from the worker
	// thread (onStart reading the device, disableAllChannels on a failed start), and
	// these touch widgets.
	connect(src, &scopy::acq::SourceBlock::channelsChanged, this, rebuild, Qt::QueuedConnection);
	// Without this the switch would keep claiming a channel is on after the source
	// turned it off by itself.
	connect(
		src, &scopy::acq::SourceBlock::channelEnabledChanged, this,
		[switches](const QString &id, bool en) {
			SmallOnOffSwitch *sw = switches->value(id).data();
			if(!sw) {
				return;
			}
			// Blocked: this reflects what the source already did, so echoing it
			// back through enableChannel() would be a round trip for nothing.
			QSignalBlocker b(sw);
			sw->setChecked(en);
		},
		Qt::QueuedConnection);
}

void AcqInstrumentController::setupPlots()
{
	InstrumentTemplate *it = m_ui->shell();

	m_plots = new AcqPlotManager(m_ui->store(), m_ui->engine(), it, m_ui);
	it->setCenterWidget(m_plots);

	// Direct, not queued: AcqInstrument already mirrors the engine's worker-thread
	// signals onto the GUI thread, and these are its GUI-thread re-emissions.
	connect(m_ui, &AcqInstrument::cycleComplete, m_plots, &AcqPlotManager::onCycleComplete);
	connect(m_ui, &AcqInstrument::started, m_plots, &AcqPlotManager::onStarted);
	connect(m_ui, &AcqInstrument::stopped, m_plots, &AcqPlotManager::onStopped);

	// The file source publishes outside a cycle, and the frame timer only runs while
	// acquiring — this is what makes a file draw with the engine stopped.
	if(auto *fileSrc = qobject_cast<FileSourceBlock *>(sourceById(m_ui->engine(), QStringLiteral("file")))) {
		connect(fileSrc, &FileSourceBlock::published, m_plots, [this]() {
			m_plots->onCycleComplete();
			m_plots->replot();
		});
	}

	// How much history is drawn, independent of how much arrives per cycle: a window wider
	// than one buffer is stitched from several chunks, which is the store's business.
	gui::MenuSpinbox *plotSpin = new gui::MenuSpinbox("Plot window", m_plots->plotSize(), "samples", 16, 1 << 20,
							  true, false, false, it);
	plotSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	it->addEngineControl(plotSpin);
	connect(plotSpin, &gui::MenuSpinbox::valueChanged, m_plots,
		[this](double v) { m_plots->setPlotSize(static_cast<int>(v)); });

	// The engine's sample-index ramp has to be at least as long as the widest plot, or a plot
	// wider than it reads a short X window and draws a truncated curve. The manager states
	// the requirement and this connection applies it.
	if(scopy::acq::AcquisitionEngine *engine = m_ui->engine()) {
		connect(m_plots, &AcqPlotManager::maxWindowSizeChanged, engine,
			[engine](int n) { engine->setIndexRampLength(static_cast<std::size_t>(n)); });
		// Once now: a channel reads X on its very first pull.
		engine->setIndexRampLength(static_cast<std::size_t>(m_plots->plotSize()));
	}

	if(m_fftProc) {
		// The timeline for channels whose producer declared no rate — Pluto's raw ones,
		// since PlutoIIOSource does not read the device rate back. A stream carrying its
		// own rate ignores this.
		m_plots->setFallbackSampleRate(m_fftProc->sampleRate());
	}
}

void AcqInstrumentController::setupAnalysisPanel()
{
	if(!m_fftProc) {
		return;
	}

	InstrumentTemplate *it = m_ui->shell();

	// PS_RIGHT rather than the right menu: this read-out annotates the spectrum, so it
	// belongs beside the plot and stays visible while a channel's menu page is open.
	m_genalyzerPanel = new GenalyzerPanel(it);
	it->addToSlot(PS_RIGHT, m_genalyzerPanel);
	m_genalyzerPanel->setVisible(m_fftProc->config().enabled);

	const QString channelName = m_fftProc->outputKey().toString();

	// Queued: the engine runs the processor on its own thread. The snapshot is a registered
	// metatype, so it deep-copies across the connection.
	connect(
		m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisReady, this,
		[this, channelName](const scopy::acq::GenalyzerResultsSnapshot &snap) {
			if(!m_genalyzerPanel) {
				return;
			}
			// GenalyzerPanel::updateResults takes genalyzer's raw char**/double*
			// arrays, so the snapshot is unpacked into views that outlive the call.
			const int               n = snap.keys.size();
			std::vector<QByteArray> keyBytes;
			keyBytes.reserve(n);
			std::vector<char *> keyPtrs;
			keyPtrs.reserve(n);
			for(const QString &k : snap.keys) {
				keyBytes.emplace_back(k.toUtf8());
				keyPtrs.push_back(keyBytes.back().data());
			}
			std::vector<double> values(snap.values.begin(), snap.values.end());
			m_genalyzerPanel->updateResults(channelName, QColor(0x4a, 0xb8, 0xff), static_cast<size_t>(n),
							keyPtrs.empty() ? nullptr : keyPtrs.data(),
							values.empty() ? nullptr : values.data());
		},
		Qt::QueuedConnection);

	// Cleared rather than just hidden: the next enable should not flash stale values before
	// the first snapshot arrives.
	connect(
		m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisEnabledChanged, this,
		[this](bool en) {
			if(!m_genalyzerPanel) {
				return;
			}
			if(!en) {
				m_genalyzerPanel->clear();
			}
			m_genalyzerPanel->setVisible(en);
		},
		Qt::QueuedConnection);
}

void AcqInstrumentController::stop()
{
	if(m_ui) {
		m_ui->stop();
	}
}

AcqInstrument *AcqInstrumentController::ui() const { return m_ui; }

#include "moc_acqinstrumentcontroller.cpp"
