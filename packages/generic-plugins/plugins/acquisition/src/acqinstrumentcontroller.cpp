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
#include "logview.h"
#include "sourceregistry.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/block.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/genalyzerfftprocessor.h>
#include <core/acq_engine/pipelineinspector.h>
#include <core/acq_engine/sourceblock.h>
#include <core/acq_engine/triggermarker.h>
#include <core/acq_engine/triggerprocessor.h>
#include <core/acq_engine/triggerprocessorwidget.h>
#include <core/decoder/decoderlogger.h>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>
#include <core/decoder/sigrokclibackendfactory.h>
#include <core/decoder/sigrokclicatalog.h>
#include <gui/instrumenttemplate.h>
#include <gui/widgets/genalyzerpanel.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/toolbuttons.h>

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

// A rail row's right-menu page: `body` inside a section with an owner pill.
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

// The FFT's rate until its input declares one (see GenalyzerFFTProcessor::process()).
constexpr double kFFTSampleRate = 2.4e6;

// The FFT's output keys, fixed for its life: plot channels are created against them.
const QString kFFTSourceId = QStringLiteral("genalyzer");
const QString kFFTChannelId = QStringLiteral("spectrum");

// addChannelGroup() returns the existing group for a repeat title.
const QString kProcessorsGroup = QStringLiteral("Processors");

const QColor kAnalysisPanelColor(0x4a, 0xb8, 0xff);

} // namespace

AcqInstrumentController::AcqInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_tme(tme)
{
	// Parentless until the caller hands it to the ToolMenuEntry, before init().
	m_ui = new AcqInstrument(nullptr);
}

AcqInstrumentController::~AcqInstrumentController()
{
	stop();
	// The instrument first (it holds the engine and the decoder panel); the decoder members
	// then go in reverse declaration order.
	delete m_ui;
}

void AcqInstrumentController::init(scopy::component::Context *ctx)
{
	if(m_initialized) {
		return;
	}
	m_initialized = true;

	// Order matters: blocks before plots (the plots point at their keys), plots before the
	// trigger and decoders (both use the plot window), the analysis panel last (it sits
	// beside the center widget setupPlots() installs).
	setupBlocks(ctx);
	setupProcessors();
	setupPlots();
	setupTrigger();
	setupDecoders();
	setupAnalysisPanel();

	connect(m_ui->shell()->singleBtn(), &QAbstractButton::toggled, this, [this](bool checked) {
		if(checked) {
			single();
		}
	});

	// Keep the tool menu's run button and the instrument's in step. The entry belongs to
	// the tool menu and can go first.
	if(m_tme) {
		connect(m_tme, &ToolMenuEntry::runToggled, m_ui, [this](bool on) { on ? m_ui->run() : m_ui->stop(); });
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
		// Built to be asked: each source decides whether its device is in the context.
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
	MenuSectionCollapseWidget *procs = m_ui->shell()->addChannelGroup(kProcessorsGroup);

	// No inputs: the reader picks them in the block's own settings panel.
	m_fftProc = new scopy::acq::GenalyzerFFTProcessor(
		scopy::acq::DataKey::withStage(kFFTSourceId, kFFTChannelId, "fft"),
		scopy::acq::DataKey::withStage(kFFTSourceId, kFFTChannelId, "freq"),
		static_cast<int>(engine->bufferSize()), kFFTSampleRate, GnWindowHann, engine);
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

	// No colour: a swatch on the rail means "the curve in that colour", and only plot
	// channels have one.
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

	// Both ways; Block::setEnabled drops a write that changes nothing, so no loop.
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

	// AcqInstrument's signals are already on the GUI thread. With the trigger on, a plot
	// draws the cycle that fired, not every cycle.
	connect(m_ui, &AcqInstrument::cycleComplete, m_plots, [this]() {
		if(!m_trigProc || !m_trigProc->isEnabled()) {
			m_plots->onCycleComplete();
		}
	});
	connect(m_ui, &AcqInstrument::started, m_plots, &AcqPlotManager::onStarted);
	connect(m_ui, &AcqInstrument::stopped, m_plots, &AcqPlotManager::onStopped);

	// A file is published outside any cycle, so it draws with the engine stopped.
	for(scopy::acq::SourceBlock *src : engine->sources()) {
		if(auto *fileSrc = qobject_cast<FileSourceBlock *>(src)) {
			connect(fileSrc, &FileSourceBlock::published, m_plots, [this]() {
				m_plots->onCycleComplete();
				m_plots->replot();
			});
		}
	}

	// How much history is drawn, independent of how much arrives per cycle.
	gui::MenuSpinbox *plotSpin = new gui::MenuSpinbox("Plot window", m_plots->plotSize(), "samples", 16, 1 << 20,
							  true, false, false, it);
	plotSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	it->addEngineControl(plotSpin);
	connect(plotSpin, &gui::MenuSpinbox::valueChanged, m_plots,
		[this](double v) { m_plots->setPlotSize(static_cast<int>(v)); });
	connect(m_plots, &AcqPlotManager::plotSizeChanged, plotSpin,
		[plotSpin](int n) { plotSpin->setValueSilent(n); });

	// The sample-index ramp must be at least as long as the plot window.
	connect(m_plots, &AcqPlotManager::plotSizeChanged, engine,
		[engine](int n) { engine->setIndexRampLength(static_cast<std::size_t>(n)); });
	engine->setIndexRampLength(static_cast<std::size_t>(m_plots->plotSize()));
}

void AcqInstrumentController::setupTrigger()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate *it = m_ui->shell();

	m_trigProc = new scopy::acq::TriggerProcessor(QStringLiteral("trigger"), engine);
	m_trigProc->setEnabled(false);
	m_trigProc->setWindowSize(m_plots->plotSize());
	m_trigProc->setTriggerPosition(0.5);
	// Its snapshot spans the whole store, so it runs after the FFT and the decoders.
	m_trigProc->setRunsLast(true);
	engine->addProcessor(m_trigProc);

	m_trigMarker = new scopy::acq::TriggerMarker(m_trigProc, this);

	// Draw the fire's own windows: re-reading the store would draw the newest samples
	// instead of the window the fire index names. Fires arrive on this thread, coalesced.
	connect(m_trigProc, &scopy::acq::TriggerProcessor::fired, m_plots,
		[this](quint32, const QMap<QString, scopy::acq::SampleVariant> &snap) {
			if(m_trigProc->isEnabled()) {
				m_plots->onTriggerFired(snap);
			}
		});

	// The settings page, built here and handed to the block so the Pipeline tab shows this
	// same widget. The base checkbox explicitly: TriggerProcessor's own
	// createSettingsWidget() would add a second TriggerProcessorWidget.
	QWidget *body = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(body);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(m_trigProc->ProcessorBlock::createSettingsWidget(body));

	// Which X axis the marker rides. Every key is offered; a pick that resolves to no axis
	// hides the marker until it does.
	MenuSectionCollapseWidget *axisSection = it->createMenuSection(tr("TRIGGER AXIS"), SO_VIEW, body);
	m_trigAxisCombo = new MenuCombo(tr("Axis source"), axisSection);
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

	// The manager resolves which axis the marker rides and reports the window it spans;
	// the processor and its spinbox follow that one number.
	connect(m_plots, &AcqPlotManager::triggerWindowChanged, this, [this](int n) {
		m_trigProc->setWindowSize(n);
		if(m_trigWidget) {
			m_trigWidget->setMaxTargetSample(std::max(0, n - 1));
		}
	});
	// Key first, then the marker: the second call is the one that resolves.
	m_plots->setTriggerAxisKey(AcqPlotManager::keyFromCombo(m_trigAxisCombo.data()));
	m_plots->setTriggerMarker(m_trigMarker);

	// Disarmed on any stop, so an aborted Single cannot stop the next run.
	connect(m_ui, &AcqInstrument::stopped, this, [this]() { disconnect(m_singleShot); });

	connect(m_ui, &AcqInstrument::started, m_trigWidget, [this]() { m_trigWidget->setAcquisitionRunning(true); });
	connect(m_ui, &AcqInstrument::stopped, m_trigWidget, [this]() { m_trigWidget->setAcquisitionRunning(false); });

	// Both key pickers follow the store's key list.
	connect(m_plots, &AcqPlotManager::keysAvailable, this, [this](const QList<scopy::acq::DataKey> &keys) {
		if(m_trigWidget) {
			QStringList names;
			names.reserve(keys.size());
			for(const scopy::acq::DataKey &k : keys) {
				names << k.toString();
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
	// Its own tab, not forwarded to the engine log: per-annotation chatter would bury it.
	m_decoderLogger = std::make_unique<scopy::decoder::DecoderLogger>();
	scopy::decoder::DecoderLogger *log = m_decoderLogger.get();
	LogView *logView = new LogView("No decoder messages.", it);
	it->addDebugTab(logView, "Decoder logs");
	connect(log, &scopy::decoder::DecoderLogger::messageLogged, logView, &LogView::append, Qt::QueuedConnection);
	connect(logView, &LogView::minSeverityChanged, log,
		[log](int sev) { log->setMinLevel(static_cast<scopy::decoder::LogLevel>(sev)); });
	logView->setMinSeverity(static_cast<int>(log->minLevel()));

	auto catalog = std::make_unique<scopy::decoder::SigrokCliCatalog>();
	catalog->setLogger(log);
	auto factory = std::make_unique<scopy::decoder::SigrokCliBackendFactory>(catalog.get());
	factory->setLogger(log);

	m_decoderMgr = std::make_unique<scopy::decoder::DecoderManager>(engine, m_ui->store(), factory.get());
	m_decoderMgr->setLogger(log);
	m_decoderMgr->setDecoderWindowSize(m_plots->plotSize());

	// HostScrolls: the right menu already scrolls, and a nested scroll area would cut the
	// panel off.
	m_decoderPanel = new scopy::decoder::DecoderPanel(m_decoderMgr.get(), m_ui->store(), catalog.get(), m_ui,
							  scopy::decoder::DecoderPanel::HostScrolls);
	m_decoderPanel->setLogger(log);

	// A page with no one block behind it: the manager creates decoder blocks on demand.
	addBlockRow(it->addChannelGroup(kProcessorsGroup), m_decoderPanel, tr("Decoders"), tr("DECODERS"),
		    QStringLiteral("decoders"));

	m_decoderCatalog = std::move(catalog);
	m_decoderFactory = std::move(factory);

	connect(m_decoderMgr.get(), &scopy::decoder::DecoderManager::decoderAdded, m_decoderPanel,
		[this](const QString &) { m_decoderPanel->refreshKeys(m_ui->store()->keys()); });
	connect(m_plots, &AcqPlotManager::keysAvailable, m_decoderPanel,
		[this](const QList<scopy::acq::DataKey> &keys) { m_decoderPanel->refreshKeys(keys); });

	m_ui->pipeline()->setDecoderManager(m_decoderMgr.get());

	// A decoder's output is drawn as an annotation channel; its X is the plot's own sample
	// ramp, since annotation offsets are relative to the decoded window.
	connect(m_decoderMgr.get(), &scopy::decoder::DecoderManager::outKeyAdded, m_plots,
		[this](const scopy::acq::DataKey &key) {
			m_plots->drawOnFirstPlot(scopy::acq::ReprKind::Annotations, key);
		});
	connect(m_decoderMgr.get(), &scopy::decoder::DecoderManager::stagesRemoved, m_plots,
		[this](const QString &, const QList<scopy::acq::DataKey> &keys) { m_plots->removeChannelsFor(keys); });

	// The decode window is the plot window.
	connect(m_plots, &AcqPlotManager::plotSizeChanged, m_decoderMgr.get(),
		&scopy::decoder::DecoderManager::setDecoderWindowSize);
}

void AcqInstrumentController::setupAnalysisPanel()
{
	InstrumentTemplate *it = m_ui->shell();

	// Beside the plot rather than in the right menu: it annotates the spectrum.
	m_genalyzerPanel = new GenalyzerPanel(it);
	it->addToSlot(PS_RIGHT, m_genalyzerPanel);
	m_genalyzerPanel->setVisible(m_fftProc->config().enabled);

	const QString channelName = m_fftProc->outputKey().toString();

	connect(
		m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisReady, this,
		[this, channelName](const scopy::acq::GenalyzerResultsSnapshot &snap) {
			if(!m_genalyzerPanel) {
				return;
			}
			// GenalyzerPanel takes genalyzer's raw char** / double* arrays.
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

	// Cleared, not just hidden, so the next enable does not flash stale values.
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

void AcqInstrumentController::single()
{
	if(!m_trigProc || !m_trigProc->isEnabled()) {
		m_ui->single();
		return;
	}
	// One cycle is wrong for a trigger: it may need many cycles to see its edge, and a
	// latched fire completes on a later cycle. So run, and stop on the first fire — after
	// the replot connection above has drawn it, since that one was made first.
	disconnect(m_singleShot);
	m_singleShot = connect(m_trigProc, &scopy::acq::TriggerProcessor::fired, this, [this]() {
		disconnect(m_singleShot);
		m_ui->stop();
	});
	m_ui->run();
}

void AcqInstrumentController::stop()
{
	if(m_ui) {
		m_ui->stop();
	}
}

AcqInstrument *AcqInstrumentController::ui() const { return m_ui; }

AcqPlotManager *AcqInstrumentController::plots() const { return m_plots; }

scopy::decoder::DecoderPanel *AcqInstrumentController::decoders() const { return m_decoderPanel; }

#include "moc_acqinstrumentcontroller.cpp"
