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

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/block.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/genalyzerfftprocessor.h>
#include <core/acq_engine/pipelineinspector.h>
#include <core/acq_engine/sourceblock.h>
#include <core/acq_engine/triggerbinder.h>
#include <core/acq_engine/triggermarker.h>
#include <core/acq_engine/triggerprocessor.h>
#include <core/acq_engine/triggerprocessorwidget.h>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>
#include <core/decoder/sigrokclibackendfactory.h>
#include <core/decoder/sigrokclicatalog.h>
#include <gui/instrumenttemplate.h>
#include <gui/widgets/genalyzerpanel.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuspinbox.h>

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QMap>
#include <QVBoxLayout>

#include <algorithm>
#include <memory>
#include <vector>

using namespace scopy;
using namespace scopy::adc;

namespace {

// A rail row's right-menu page: any widget inside a section with an owner pill.
QWidget *widgetPage(InstrumentTemplate *it, QWidget *body, const QString &title)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_CH, page);
	section->add(body);
	lay->addWidget(section);
	lay->addStretch();
	return page;
}

// The engine's registered sources are the only record of what setupBlocks() built, so the
// code below asks it rather than holding a pointer per source type.
template <typename T>
T *sourceOfType(scopy::acq::AcquisitionEngine *engine)
{
	for(scopy::acq::SourceBlock *src : engine->sources()) {
		if(auto *hit = qobject_cast<T *>(src)) {
			return hit;
		}
	}
	return nullptr;
}

// The FFT's fallback rate: no source reads its rate back for the processor, so it has to be
// told one.
constexpr double kFallbackSampleRate = 2.4e6;

// Where the FFT writes. Not derived from any source: the block's inputs are the reader's pick
// and can be retargeted at a different source entirely, while the output keys are fixed for its
// life because a plot channel is created against them.
const QString kFFTSourceId = QStringLiteral("genalyzer");
const QString kFFTChannelId = QStringLiteral("spectrum");

// The rail group every non-source block's row goes into. addChannelGroup() returns the
// existing group for a repeat title, so all three call sites land in one section.
const QString kProcessorsGroup = QStringLiteral("Processors");

// The analysis table's accent, for its one and only channel.
const QColor kAnalysisPanelColor(0x4a, 0xb8, 0xff);

} // namespace

AcqInstrumentController::AcqInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_tme(tme)
{
	// Parentless: the ToolMenuEntry takes it, and the plugin deletes it in
	// deleteInstrument(). Same ownership as the other controllers here. Built here so the
	// caller can parent it through setTool() before init() constructs the sub-widgets.
	m_ui = new AcqInstrument(nullptr);
}

AcqInstrumentController::~AcqInstrumentController()
{
	stop();
	// Both before ~QObject: the manager is a child of this and holds the factory non-owning,
	// so a member unique_ptr would otherwise be reset first; the instrument is parentless
	// unless a ToolMenuEntry took it, and QPointer tells those two cases apart.
	delete m_decoderMgr;
	delete m_ui;
}

void AcqInstrumentController::init(scopy::component::Context *ctx)
{
	if(m_initialized) {
		return;
	}
	m_initialized = true;

	setupBlocks(ctx);
	setupProcessors();
	setupPlots();
	// Both after setupPlots(): the fire window and the decode window are the plot window.
	setupTrigger();
	setupDecoders();
	// Last: it sits in a slot around the center widget setupPlots() installs.
	setupAnalysisPanel();

	// The tool menu's own run button and the instrument's stay in step. No loop: setRunning()
	// only changes state, so the echo back is a no-op. Guarded on both sides: the entry is
	// owned by the tool menu and can go before this controller does.
	if(m_tme) {
		connect(m_tme, &ToolMenuEntry::runToggled, m_ui, [this](bool on) {
			if(on) {
				m_ui->run();
			} else {
				m_ui->stop();
			}
		});
	}
	connect(m_ui, &AcqInstrument::started, this, [this]() {
		if(m_tme) {
			m_tme->setRunning(true);
		}
	});
	connect(m_ui, &AcqInstrument::stopped, this, [this]() {
		if(m_tme) {
			m_tme->setRunning(false);
		}
	});
}

void AcqInstrumentController::setupBlocks(scopy::component::Context *ctx)
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	MenuSectionCollapseWidget *sources = m_ui->shell()->addChannelGroup("Sources");

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

	// The section lands between Sources and Plots on the rail; the trigger and the decoders
	// add their rows to it later.
	MenuSectionCollapseWidget *procs = m_ui->shell()->addChannelGroup(kProcessorsGroup);

	// No inputs, and no search for any: which streams to transform — and with them real or
	// complex I/Q — is picked in the block's own settings panel. So this works with sources
	// that name their channels anything at all, and the block is here to be configured rather
	// than absent because nothing matched a naming convention. nfft is tied to the engine's
	// buffer size.
	m_fftProc = new scopy::acq::GenalyzerFFTProcessor(
		scopy::acq::DataKey::withStage(kFFTSourceId, kFFTChannelId, "fft"),
		scopy::acq::DataKey::withStage(kFFTSourceId, kFFTChannelId, "freq"),
		static_cast<int>(engine->bufferSize()), kFallbackSampleRate, GnWindowHann, engine);
	// Lets the block claim chunk history when averaging is turned on: navg frames are navg
	// past chunks.
	m_fftProc->setAveragingStore(m_ui->store());
	engine->addProcessor(m_fftProc);

	addBlockRow(procs, m_fftProc, QStringLiteral("FFT"), QStringLiteral("GENALYZER FFT"), QStringLiteral("fft"));
}

MenuControlButton *AcqInstrumentController::addBlockRow(MenuSectionCollapseWidget *group, QWidget *body,
							const QString &label, const QString &pageTitle,
							const QString &menuId)
{
	if(!group || !body) {
		return nullptr;
	}
	InstrumentTemplate *it = m_ui->shell();

	// No colour: a coloured swatch on the rail means "this is the curve you see in that
	// colour", and only plot channels have one.
	MenuControlButton *row = it->addChannelRow(group, label, QColor(), menuId);
	it->addMenuPage(menuId, widgetPage(it, body, pageTitle));
	return row;
}

MenuControlButton *AcqInstrumentController::addBlockRow(MenuSectionCollapseWidget *group, scopy::acq::Block *block,
							const QString &label, const QString &pageTitle,
							const QString &menuId)
{
	if(!block) {
		return nullptr;
	}
	MenuControlButton *row = addBlockRow(group, block->settingsWidget(), label, pageTitle, menuId);
	if(!row) {
		return nullptr;
	}

	// Both directions: the same state also has a checkbox on the settings page. No loop —
	// Block::setEnabled drops a write that does not change it.
	QCheckBox *chk = row->checkBox();
	chk->setChecked(block->isEnabled());
	connect(chk, &QAbstractButton::toggled, block, &scopy::acq::Block::setEnabled);
	connect(block, &scopy::acq::Block::enabledChanged, chk, &QCheckBox::setChecked);

	return row;
}

void AcqInstrumentController::setupPlots()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate *it = m_ui->shell();

	m_plots = new AcqPlotManager(m_ui->store(), engine, it, m_ui);
	it->setCenterWidget(m_plots);

	// Direct, not queued: AcqInstrument already mirrors the engine's worker-thread
	// signals onto the GUI thread, and these are its GUI-thread re-emissions.
	//
	// Made once and never unmade: a triggered run draws the cycle that fired, not every
	// cycle the worker completes, so the drop is decided at delivery.
	connect(m_ui, &AcqInstrument::cycleComplete, m_plots, [this]() {
		if(!m_trigProc || !m_trigProc->isEnabled()) {
			m_plots->onCycleComplete();
		}
	});
	connect(m_ui, &AcqInstrument::started, m_plots, &AcqPlotManager::onStarted);
	connect(m_ui, &AcqInstrument::stopped, m_plots, &AcqPlotManager::onStopped);

	// The file source publishes outside a cycle, and the frame timer only runs while
	// acquiring — this is what makes a file draw with the engine stopped. Found by type, so
	// the block keeps naming its own id.
	if(auto *fileSrc = sourceOfType<FileSourceBlock>(engine)) {
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
	connect(m_plots, &AcqPlotManager::maxWindowSizeChanged, engine,
		[engine](int n) { engine->setIndexRampLength(static_cast<std::size_t>(n)); });
	// Once now: a channel reads X on its very first pull.
	engine->setIndexRampLength(static_cast<std::size_t>(m_plots->plotSize()));

	// The timeline for channels whose producer declared no rate. A stream carrying its own
	// rate ignores this.
	m_plots->setFallbackSampleRate(m_fftProc->sampleRate());
}

void AcqInstrumentController::setupTrigger()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate *it = m_ui->shell();

	m_trigProc = new scopy::acq::TriggerProcessor(QStringLiteral("trigger"), engine);
	m_trigProc->setEnabled(false);
	m_trigProc->setWindowSize(m_plots->plotSize());
	m_trigProc->setTriggerPosition(0.5);
	engine->addProcessor(m_trigProc);
	// Its snapshot spans the whole store, so it has to run after this cycle's derived data —
	// the FFT, the decoder annotations — is written.
	engine->setRunLast(m_trigProc);

	m_trigMarker = new scopy::acq::TriggerMarker(m_trigProc, this);

	m_trigBinder = new scopy::acq::TriggerBinder(m_trigProc, engine, this);
	// The fire's own window set is forwarded rather than dropped: re-reading the store here
	// would draw the newest samples, a different window from the one the fire index names.
	connect(m_trigBinder, &scopy::acq::TriggerBinder::replotFired, m_plots,
		[this](quint32, const QMap<QString, scopy::acq::SampleVariant> &snap) {
			m_plots->onTriggerFired(snap);
		});

	// Built here and handed to the block, so the Pipeline tab shows this one widget rather
	// than a second, independent copy of the same condition list.
	QWidget *body = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(body);
	lay->setContentsMargins(0, 0, 0, 0);
	// The base widget explicitly: TriggerProcessor's own createSettingsWidget() would build
	// a second TriggerProcessorWidget right above the one added below.
	lay->addWidget(m_trigProc->ProcessorBlock::createSettingsWidget(body));

	// Which X axis the marker rides.
	MenuSectionCollapseWidget *axisSection = it->createMenuSection(tr("TRIGGER AXIS"), SO_VIEW, body);
	m_trigAxisCombo = new MenuCombo(tr("Axis source"), axisSection);
	// Every key, unfiltered: a picker that hid a key until some channel happened to draw it
	// would change its own contents behind the reader. A pick that resolves to no axis is
	// handled instead, by hiding the handle until it does resolve.
	m_plots->populateKeyCombo(m_trigAxisCombo, /*withSampleIndex=*/true);
	connect(m_trigAxisCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int) {
		if(m_plots) {
			m_plots->setTriggerAxisKey(AcqPlotManager::keyFromCombo(m_trigAxisCombo.data()));
		}
	});
	axisSection->add(m_trigAxisCombo);
	lay->addWidget(axisSection);

	m_trigWidget = new scopy::acq::TriggerProcessorWidget(m_trigProc, body);
	m_trigWidget->setMaxTargetSample(std::max(0, m_plots->plotSize() - 1));
	lay->addWidget(m_trigWidget);
	m_trigProc->setSettingsWidget(body);

	addBlockRow(it->addChannelGroup(kProcessorsGroup), m_trigProc, tr("Trigger"), QStringLiteral("TRIGGER"),
		    QStringLiteral("trigger"));

	// Which axis the bar rides is the manager's answer: it owns the plots, their channels and
	// the per-source axes, so it re-resolves on its own as those come and go. The window it
	// settles on comes back here, because the processor and its spinbox are ours.
	connect(m_plots, &AcqPlotManager::triggerWindowChanged, this, [this](int n) {
		m_trigProc->setWindowSize(n);
		if(m_trigWidget) {
			m_trigWidget->setMaxTargetSample(std::max(0, n - 1));
		}
	});
	// The key first, then the marker: the second call is the one that resolves, and by then
	// there is a key to resolve and a connection to report the window on.
	m_plots->setTriggerAxisKey(AcqPlotManager::keyFromCombo(m_trigAxisCombo.data()));
	m_plots->setTriggerMarker(m_trigMarker);

	// Follows the block, not a button of our own: the reader flips the trigger from its own
	// settings widget. The marker's visibility follows the same signal on its own.
	connect(m_trigProc, &scopy::acq::ProcessorBlock::enabledChanged, this, [this](bool en) {
		if(!m_trigBinder) {
			return;
		}
		if(en) {
			m_trigBinder->bindReplotOnFire();
		} else {
			m_trigBinder->unbindReplotOnFire();
		}
		// With a trigger, "one acquisition" is one fire, not one cycle.
		m_ui->setSingleWaitsForStop(en);
	});

	// Armed per press, not once: the binder's one-shot disconnects itself on the fire it
	// stops on.
	connect(m_ui, &AcqInstrument::singleRequested, this, [this]() {
		if(m_trigBinder && m_trigProc && m_trigProc->isEnabled()) {
			m_trigBinder->armSingleShot();
		}
	});
	// On any stop, so an aborted Single does not leave an arm behind to stop the *next* run
	// on its first fire.
	connect(m_ui, &AcqInstrument::stopped, this, [this]() {
		if(m_trigBinder) {
			m_trigBinder->disarmSingleShot();
		}
	});

	// The widget's status label ("idle" / "waiting" / "triggered").
	connect(m_ui, &AcqInstrument::started, m_trigWidget, [this]() { m_trigWidget->setAcquisitionRunning(true); });
	connect(m_ui, &AcqInstrument::stopped, m_trigWidget, [this]() { m_trigWidget->setAcquisitionRunning(false); });

	// Both key pickers off the one list. The axis combo is repopulated rather than retargeted:
	// populateKeyCombo keeps the current selection, and a new key is not a new axis.
	connect(m_plots, &AcqPlotManager::keysAvailable, this, [this](const QList<scopy::acq::DataKey> &keys) {
		if(m_trigWidget) {
			QStringList names;
			names.reserve(keys.size());
			for(const scopy::acq::DataKey &k : keys) {
				names << k.key;
			}
			m_trigWidget->setAvailableKeys(names);
		}
		if(m_trigAxisCombo && m_plots) {
			m_plots->populateKeyCombo(m_trigAxisCombo.data(), /*withSampleIndex=*/true);
		}
	});
}

void AcqInstrumentController::setupDecoders()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate *it = m_ui->shell();
	scopy::decoder::DecoderLogger *log = m_ui->decoderLogger();

	// Neither is enumerated here: listing decoders runs sigrok-cli as a subprocess, so it
	// waits until the reader opens a picker.
	auto catalog = std::make_unique<scopy::decoder::SigrokCliCatalog>();
	catalog->setLogger(log);
	auto factory = std::make_unique<scopy::decoder::SigrokCliBackendFactory>(catalog.get());
	factory->setLogger(log);

	m_decoderMgr = new scopy::decoder::DecoderManager(engine, m_ui->store(), factory.get(), this);
	m_decoderMgr->setLogger(log);
	m_decoderMgr->setDecoderWindowSize(m_plots->plotSize());
	// No setPlot/setOverlay: annotations are drawn by adding a ReprKind::Annotations channel
	// on the output key, through the same ADD CHANNEL flow every other stream uses.

	// HostScrolls: the right menu already wraps its stack in a scroll area, and a nested one
	// would cap the panel's reported height and cut off everything past it.
	m_decoderPanel = new scopy::decoder::DecoderPanel(m_decoderMgr, m_ui->store(), catalog.get(), m_ui,
							  scopy::decoder::DecoderPanel::HostScrolls);
	m_decoderPanel->setLogger(log);

	// The widget overload: the decoders are a stack of blocks the manager adds on demand, so
	// there is no one Block whose settings widget the panel is, and no one block to enable.
	addBlockRow(it->addChannelGroup(kProcessorsGroup), m_decoderPanel, tr("Decoders"), tr("DECODERS"),
		    QStringLiteral("decoders"));

	// Members last: the panel and the manager borrow both, and the factory borrows the
	// catalog, so all three must outlive them.
	m_decoderCatalog = std::move(catalog);
	m_decoderFactory = std::move(factory);

	// A new editor comes up with empty channel combos unless it is given the key set: the
	// store may not have changed since the last refresh.
	connect(m_decoderMgr, &scopy::decoder::DecoderManager::decoderAdded, m_decoderPanel,
		[this](const QString &) { m_decoderPanel->refreshKeys(m_ui->store()->keys()); });
	connect(m_plots, &AcqPlotManager::keysAvailable, m_decoderPanel,
		[this](const QList<scopy::acq::DataKey> &keys) { m_decoderPanel->refreshKeys(keys); });

	// The Pipeline tab lists the manager's decoders: blocksChanged only reports one once the
	// engine loop has applied it.
	m_ui->pipeline()->setDecoderManager(m_decoderMgr);

	// The whole link between the decoders and the plots, in both directions. Each side states
	// what it knows — a key appeared, a key is gone — and neither names the other's concepts:
	// the manager knows nothing about decoders, and the decoders know nothing about plots.
	connect(m_decoderMgr, &scopy::decoder::DecoderManager::outKeyAdded, m_plots,
		[this](const scopy::acq::DataKey &key) {
			// X left empty: annotation offsets are relative to the window the decode ran on, so
			// they index the plot's own sample ramp.
			m_plots->drawOnFirstPlot(scopy::acq::ReprKind::Annotations, key);
		});
	connect(m_decoderMgr, &scopy::decoder::DecoderManager::stagesRemoved, m_plots,
		[this](const QString &, const QList<scopy::acq::DataKey> &keys) { m_plots->removeChannelsFor(keys); });

	// The decode window is the plot window.
	connect(m_plots, &AcqPlotManager::plotSizeChanged, m_decoderMgr,
		&scopy::decoder::DecoderManager::setDecoderWindowSize);
}

void AcqInstrumentController::setupAnalysisPanel()
{
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
			const int n = snap.keys.size();
			std::vector<QByteArray> keyBytes;
			keyBytes.reserve(n);
			std::vector<char *> keyPtrs;
			keyPtrs.reserve(n);
			for(const QString &k : snap.keys) {
				keyBytes.emplace_back(k.toUtf8());
				keyPtrs.push_back(keyBytes.back().data());
			}
			std::vector<double> values(snap.values.begin(), snap.values.end());
			m_genalyzerPanel->updateResults(channelName, kAnalysisPanelColor, static_cast<size_t>(n),
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
