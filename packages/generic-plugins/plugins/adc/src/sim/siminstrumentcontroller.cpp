#include "siminstrumentcontroller.h"

#include "DecoderOverlay.h"
#include "DecoderManager.h"
#include "DigitalTrackManager.h"
#include "DecoderPanel.h"

#include <core/decoder/DecoderLogger.h>
#include <core/decoder/SigrokCliBackendFactory.h>
#include <core/decoder/SigrokCliCatalog.h>

#include <libm2k/digital/m2kdigital.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>
#include <variant>
#include <vector>
#include <QCoreApplication>
#include <QDateTime>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QPen>
#include <Qt>
#include <plotlegend.h>

#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/genalyzerpanel.h>
#include <gui/widgets/plotinfowidgets.h>
#include <gui/widgets/plotinfo.h>

#include <core/acq_engine/GenalyzerConfig.h>
#include <core/acq_engine/GenalyzerSettings.h>

Q_LOGGING_CATEGORY(CAT_SIM_CTRL, "SimInstrumentController")

namespace scopy {
namespace adc {

SimInstrumentController::SimInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_tme(tme)
{}

SimInstrumentController::~SimInstrumentController()
{
	stop();
}

void SimInstrumentController::init(iio_context *ctx, libm2k::digital::M2kDigital *digital)
{
	// ---- Engine + store ----
	m_store  = new scopy::acq::DataStore(this);
	m_engine = new scopy::acq::AcquisitionEngine(m_store, this);
	m_engine->setBufferSize(1024);
	m_engine->setMaxFPS(30);
	m_plotSize = 1024;

	// ---- Sources ----
	m_src = new scopy::adc::sim::SimulatedSource("sim-adc", m_engine);
	m_src->enableChannel("voltage0", true);
	m_src->enableChannel("voltage1", true);
	m_engine->addSource(m_src);

	if(ctx) {
		m_plutoSrc = new scopy::adc::sim::PlutoIIOSource(ctx, "pluto", "cf-ad9361-lpc", m_engine);
		m_plutoSrc->enableChannel("voltage0", true);
		m_plutoSrc->enableChannel("voltage1", true);
		m_engine->addSource(m_plutoSrc);

		m_fftProc = new scopy::acq::GenalyzerFFTProcessor(
			scopy::acq::DataKey::raw("pluto", "voltage0"),
			scopy::acq::DataKey::raw("pluto", "voltage1"),
			scopy::acq::DataKey::withStage("pluto", "iq", "fft"),
			scopy::acq::DataKey::withStage("pluto", "iq", "freq"),
			static_cast<int>(m_engine->bufferSize()),
			/*sampleRate=*/2.4e6,
			GnWindowHann,
			m_engine);
		m_engine->addProcessor(m_fftProc);
	}

	if(digital) {
		m_logicSrc = new scopy::adc::sim::M2kLogicSource(digital, "m2k_logic", m_engine);
		// Enable all 16 digital channels by default
		for(int ch = 0; ch < scopy::adc::sim::M2kLogicSource::NR_CHANNELS; ++ch)
			m_logicSrc->enableChannel(QString("DIO%1").arg(ch), true);
		m_engine->addSource(m_logicSrc);
	}

	// ---- Processor: scale + offset ----
	m_scaleProc = new scopy::acq::ScaleOffsetProcessor("scale-offset", this);
	m_scaleProc->addChannel(scopy::acq::DataKey::raw("sim-adc", "voltage0"),
				scopy::acq::DataKey::withStage("sim-adc", "voltage0", "scaled"),
				"voltage0");
	m_scaleProc->addChannel(scopy::acq::DataKey::raw("sim-adc", "voltage1"),
				scopy::acq::DataKey::withStage("sim-adc", "voltage1", "scaled"),
				"voltage1");
	m_engine->addProcessor(m_scaleProc);

	// ---- Math source + processor ----
	m_mathSrc = new scopy::acq::MathSource("math-src", m_engine);
	m_engine->addSource(m_mathSrc);

	m_mathProc = new scopy::acq::MathProcessor("math", m_engine);
	m_mathProc->setOutputKey(scopy::acq::DataKey::withStage("math-src", "out", "proc"));
	m_mathProc->setWatchedKeys({m_mathSrc->outputKey()});
	m_engine->addProcessor(m_mathProc);

	// ---- Trigger processor ----
	// Disabled by default so behaviour is byte-identical to today; the user
	// enables it from the "Trigger" settings group. When enabled the
	// controller switches its replot path from cycleComplete to fired.
	// Starts with no conditions — the user adds them from the widget.
	m_trigProc = new scopy::acq::TriggerProcessor("trigger", m_engine);
	m_trigProc->setEnabled(false);
	m_engine->addProcessor(m_trigProc);

	// Standardized trigger ↔ engine wiring. All fire deliveries reach the
	// GUI through the binder's re-emitted signals so we never touch raw
	// QMetaObject::Connection pointers in call-site lambdas.
	m_trigBinder = new scopy::acq::TriggerBinder(m_trigProc, m_engine, this);
	connect(m_trigBinder, &scopy::acq::TriggerBinder::singleShotFired,
		this, &SimInstrumentController::onTriggerFired);
	connect(m_trigBinder, &scopy::acq::TriggerBinder::replotFired,
		this, &SimInstrumentController::onTriggerFired);

	// ---- UI ----
	m_ui = new SimInstrument(nullptr);

	// ---- Decoder plumbing (composition root) ----
	m_decoderLogger = new scopy::decoder::DecoderLogger(this);
	m_decoderLogger->setEngine(m_engine);
	// Decoder logs go to their own right-stack panel; don't interleave them
	// with the generic acq-error "Logs" panel via the engine::error path.
	m_decoderLogger->setForwardToEngine(false);
	connect(m_decoderLogger, &scopy::decoder::DecoderLogger::messageLogged,
		m_ui, &SimInstrument::appendDecoderLog, Qt::QueuedConnection);

	auto catalog            = std::make_unique<scopy::decoder::SigrokCliCatalog>();
	auto *catalogPtr        = catalog.get();
	catalogPtr->setLogger(m_decoderLogger);
	m_decoderCatalog        = std::move(catalog);
	auto backendFactory     = std::make_unique<scopy::decoder::SigrokCliBackendFactory>(
		catalogPtr);
	backendFactory->setLogger(m_decoderLogger);
	m_decoderBackendFactory = std::move(backendFactory);

	// Mixed-signal digital area: dedicated y-axis + per-item drag handles,
	// hosting both raw 0/1 curves and decoder annotation bands.
	m_digitalMgr = new scopy::adc::DigitalTrackManager(m_store, this);
	m_digitalMgr->setPlot(m_ui->m_plot);

	// Auto-register every M2K DIOx channel as a raw digital track and
	// keep its visibility in sync with the source-block's enable state.
	// The manager doesn't know about the source block itself — we just
	// forward (sourceId, channelName, bool) via a plain slot.
	if(m_logicSrc) {
		const QString sourceId = m_logicSrc->id();
		for(int ch = 0; ch < scopy::adc::sim::M2kLogicSource::NR_CHANNELS; ++ch) {
			const QString name = QStringLiteral("DIO%1").arg(ch);
			m_digitalMgr->addRawChannel(
				scopy::acq::DataKey::raw(sourceId, name), name);
			// Apply initial enable state so already-disabled
			// channels start hidden.
			m_digitalMgr->setChannelVisible(sourceId, name,
				m_logicSrc->isChannelEnabled(name));
		}
		connect(m_logicSrc, &scopy::acq::SourceBlock::channelEnabledChanged,
			m_digitalMgr,
			[this, sourceId](const QString &ch, bool en) {
				m_digitalMgr->setChannelVisible(sourceId, ch, en);
			});
	}

	m_decoderOverlay = new scopy::adc::DecoderOverlay(m_ui->m_plot, m_store, this);
	m_decoderOverlay->setAnnotationYAxis(m_digitalMgr->yAxis());
	m_decoderMgr = new DecoderManager(m_engine, m_store,
	                                  m_decoderBackendFactory.get(), this);
	m_decoderMgr->setLogger(m_decoderLogger);
	m_decoderMgr->setPlot(m_ui->m_plot);
	m_decoderMgr->setOverlay(m_decoderOverlay);
	m_decoderMgr->setDigitalTrackManager(m_digitalMgr);
	m_decoderMgr->setDecoderWindowSize(m_plotSize);

	// ---- Genalyzer analysis panel ----
	m_genalyzerPanel = new scopy::GenalyzerPanel(m_ui);
	m_ui->m_tool->addWidgetToCentralContainerHelper(m_genalyzerPanel);

	// AcquisitionEngine runs the processor on its own QThread; cross-thread
	// delivery of the analysis snapshot is handled via Qt::QueuedConnection.
	// GenalyzerResultsSnapshot is Q_DECLARE_METATYPE-d and registered in the
	// processor's translation unit so queued dispatch is safe.
	connect(m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisReady,
		this, [this](const scopy::acq::GenalyzerResultsSnapshot &snap) {
			if(!m_genalyzerPanel)
				return;
			// Build temporary char**/double* views for GenalyzerPanel::updateResults,
			// which (by design) expects raw genalyzer-style arrays.
			const int n = snap.keys.size();
			std::vector<QByteArray> keyBytes;
			keyBytes.reserve(n);
			std::vector<char *>     keyPtrs;
			keyPtrs.reserve(n);
			for(const QString &k : snap.keys) {
				keyBytes.emplace_back(k.toUtf8());
				keyPtrs.push_back(keyBytes.back().data());
			}
			std::vector<double> values(snap.values.begin(), snap.values.end());
			m_genalyzerPanel->updateResults(
				QStringLiteral("sim-adc/voltage0"),
				QColor(0xff, 0x7e, 0x40),
				static_cast<size_t>(n),
				keyPtrs.empty() ? nullptr : keyPtrs.data(),
				values.empty()  ? nullptr : values.data());
		}, Qt::QueuedConnection);

	// Surface failures in the log view so the user can see why analysis isn't
	// producing numbers.
	connect(m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisFailed,
		this, [this](const QString &reason) {
			if(m_ui)
				m_ui->appendLog(
					static_cast<int>(scopy::acq::AcquisitionError::Severity::Warning),
					QStringLiteral("genalyzer"),
					reason);
		}, Qt::QueuedConnection);

	// ---- Plot channels ----
	// Channel 1 — cyan
	m_curve = new PlotChannel("Y1", QPen(QColor(0x4a, 0xb8, 0xff)),
				  m_ui->m_plot->xAxis(), m_ui->m_plot->yAxis(), m_ui);
	m_ui->m_plot->addPlotChannel(m_curve);
	m_curve->enable();

	// Channel 2 — orange
	m_curve2 = new PlotChannel("Y2", QPen(QColor(0xff, 0x7e, 0x40)),
				   m_ui->m_plot->xAxis(), m_ui->m_plot->yAxis(), m_ui);
	m_ui->m_plot->addPlotChannel(m_curve2);
	m_curve2->enable();

	// ---- Autoscalers ----
	m_autoscalerY = new scopy::gui::PlotAutoscaler(this);
	m_autoscalerY->addChannels(m_curve);
	m_autoscalerY->addChannels(m_curve2);
	connect(m_autoscalerY, &scopy::gui::PlotAutoscaler::newMin,
		m_ui->m_plot->yAxis(), &PlotAxis::setMin);
	connect(m_autoscalerY, &scopy::gui::PlotAutoscaler::newMax,
		m_ui->m_plot->yAxis(), &PlotAxis::setMax);
	connect(m_engine, &scopy::acq::AcquisitionEngine::started,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::start);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::stop);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::stop);

	m_autoscalerX = new scopy::gui::PlotAutoscaler(this);
	m_autoscalerX->setXAxisMode(true);
	m_autoscalerX->addChannels(m_curve);
	m_autoscalerX->addChannels(m_curve2);
	connect(m_autoscalerX, &scopy::gui::PlotAutoscaler::newMin,
		m_ui->m_plot->xAxis(), &PlotAxis::setMin);
	connect(m_autoscalerX, &scopy::gui::PlotAutoscaler::newMax,
		m_ui->m_plot->xAxis(), &PlotAxis::setMax);
	connect(m_engine, &scopy::acq::AcquisitionEngine::started,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::start);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::stop);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::stop);

	// ---- Wire UI buttons → engine ----
	connect(m_ui, &SimInstrument::requestRun, this, [this]() {
		if(m_store) m_store->clear();
		resetLiveBuffers();
		refreshPlotAxis();
		m_engine->run();
	});
	connect(m_ui, &SimInstrument::requestStop, m_engine, &scopy::acq::AcquisitionEngine::stop);
	connect(m_ui, &SimInstrument::requestSingle, this, [this]() {
		if(m_store) m_store->clear();
		resetLiveBuffers();
		refreshPlotAxis();
		if(m_triggerReplotEnabled) {
			// Trigger-armed single: arm the binder's one-shot. On fire
			// it re-emits singleShotFired (→ onTriggerFired plots the
			// snapshot) and calls engine->stop(). No manual connection
			// bookkeeping in the call site.
			{
				QSignalBlocker b(m_ui->m_singleBtn);
				m_ui->m_singleBtn->setChecked(false);
			}
			if(m_trigBinder)
				m_trigBinder->armSingleShot();
			m_engine->run();
		} else {
			const std::size_t n = scopy::acq::DataStore::requiredHistoryDepth(
				static_cast<std::size_t>(m_plotSize), m_engine->bufferSize());
			m_engine->single(static_cast<unsigned int>(n));
		}
	});

	connect(m_ui, &SimInstrument::sampleSizeChanged, this, [this](int n) {
		m_engine->setBufferSize(static_cast<std::size_t>(n));
		// targetSample is chunk-local, so its upper bound follows
		// the engine's buffer size, not the plot window size.
		if(m_trigWidget)
			m_trigWidget->setMaxTargetSample(std::max(0, n - 1));
	});
	connect(m_ui, &SimInstrument::plotSizeChanged, this, [this](int n) {
		m_plotSize = std::max(1, n);
		refreshPlotAxis();
		if(m_decoderMgr)
			m_decoderMgr->setDecoderWindowSize(m_plotSize);
	});
	connect(m_ui, &SimInstrument::maxFpsChanged, this, [this](int fps) {
		m_engine->setMaxFPS(static_cast<unsigned int>(fps));
	});
	connect(m_ui, &SimInstrument::acqModeChanged, this, [this](int idx) {
		m_engine->setMode(idx == 0
			? scopy::acq::AcquisitionEngine::Mode::Continuous
			: scopy::acq::AcquisitionEngine::Mode::Triggered);
	});

	// ---- Wire engine signals → UI state ----
	connect(m_engine, &scopy::acq::AcquisitionEngine::started,      m_ui, &SimInstrument::onStarted);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,      m_ui, &SimInstrument::onStopped);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, m_ui, &SimInstrument::onForceStopped);

	// Trigger label follows acquisition run state. "triggered" is only
	// ever shown post-stop when a fire arrived during the run; during
	// the run the label stays at "waiting" (enabled) / "idle" (disabled).
	auto reportAcqRunning = [this](bool running) {
		if(m_trigWidget)
			m_trigWidget->setAcquisitionRunning(running);
	};
	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this,
		[reportAcqRunning]() { reportAcqRunning(true); });
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped, this,
		[reportAcqRunning]() { reportAcqRunning(false); });
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, this,
		[reportAcqRunning]() { reportAcqRunning(false); });

	// ---- Wire engine cycleComplete → plot update ----
	// The exact source of "replot now" is swappable at runtime — when the
	// trigger is enabled we switch to TriggerProcessor::fired instead. See
	// setTriggerReplotEnabled().
	m_cycleConn = connect(m_engine, &scopy::acq::AcquisitionEngine::cycleComplete,
		this, &SimInstrumentController::onCycleComplete,
		Qt::QueuedConnection);

	// Follow the processor's enable state so the user can flip trigger mode
	// on/off directly from the trigger's own settings widget.
	connect(m_trigProc, &scopy::acq::ProcessorBlock::enabledChanged, this,
		[this](bool en) { setTriggerReplotEnabled(en); });

	// ---- Log errors ----
	connect(m_engine, &scopy::acq::AcquisitionEngine::error, this,
		[this](int severity, const QString &id, const QString &message) {
			const auto sev = static_cast<scopy::acq::AcquisitionError::Severity>(severity);
			const char *sevStr =
				sev == scopy::acq::AcquisitionError::Severity::Critical ? "CRITICAL" :
				sev == scopy::acq::AcquisitionError::Severity::Warning  ? "WARNING"  :
				                                                   "INFO";
			qWarning(CAT_SIM_CTRL) << "[" << sevStr << "]" << id << ":" << message;
			if(m_ui)
				m_ui->appendLog(severity, id, message);
		});

	// ---- Axis labels ----
	m_ui->m_plot->setShowXAxisLabels(true);
	m_ui->m_plot->setShowYAxisLabels(true);
	m_ui->m_plot->showAxisLabels();
	m_ui->m_plot->plot()->insertLegend(m_ui->m_plot->plot()->legend());

	// ---- FPS label (top-left of plot canvas) ----
	m_fpsLabel = m_ui->m_plot->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	m_fpsLabel->setText("-- FPS");
	m_cycleCount = 0;
	m_fpsTimer.start();
	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this, [this]() {
		m_cycleCount = 0;
		m_fpsTimer.restart();
		m_fpsLabel->setText("-- FPS");
	});

	// ---- Display timer: replot at up to 60 Hz ----
	m_displayTimer = new QTimer(this);
	m_displayTimer->setInterval(16);
	connect(m_displayTimer, &QTimer::timeout, this, [this]() {
		if(!m_ui || !m_dataDirty)
			return;
		m_dataDirty = false;
		m_ui->m_plot->replot();
		m_ui->m_waterfall->replot();
	});
	connect(m_engine, &scopy::acq::AcquisitionEngine::started,      m_displayTimer, QOverload<>::of(&QTimer::start));
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,      m_displayTimer, &QTimer::stop);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, m_displayTimer, &QTimer::stop);
	auto flushDirty = [this]() {
		if(m_dataDirty) {
			m_dataDirty = false;
			m_ui->m_plot->replot();
			m_ui->m_waterfall->replot();
		}
	};
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,      this, flushDirty);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, this, flushDirty);

	// ---- Cursors ----
	m_cursorCtrl = new scopy::CursorController(m_ui->m_plot, this);
	auto *cursorSettings = new scopy::CursorSettings(m_ui);
	m_cursorCtrl->connectSignals(cursorSettings);
	m_ui->m_tool->rightStack()->add("cursor-config", cursorSettings);
	connect(m_ui->m_cursorBtn, &QPushButton::toggled, m_cursorCtrl, &scopy::CursorController::setVisible);

	// ---- Waterfall configuration ----
	m_ui->m_waterfall->setNumRows(WATERFALL_ROWS);
	m_ui->m_waterfall->setIntensityRange(-120.0, 0.0);
	m_ui->m_waterfall->setWaterfallEnabled(true);

	// TODO: temporary — reset autoscale accumulators on each engine start so stale
	// extremes from a previous run don't pin the color range (see onCycleComplete).
	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this, [this]() {
		m_wfAutoMin =  DBL_MAX;
		m_wfAutoMax = -DBL_MAX;
	});

	// ---- Build the auto-generated settings panel ----
	SimInstrument::CurveDescriptor curve1;
	curve1.name  = "Curve 1";
	curve1.color = QColor(0x4a, 0xb8, 0xff);
	curve1.processors << m_scaleProc;

	SimInstrument::CurveDescriptor curve2;
	curve2.name  = "Curve 2";
	curve2.color = QColor(0xff, 0x7e, 0x40);
	if(m_fftProc)
		curve2.processors << m_fftProc;
	curve2.processors << m_mathProc;

	SimInstrument::CurveDescriptor wfDesc;
	wfDesc.name  = "Waterfall";
	wfDesc.color = QColor(0x00, 0xc8, 0xff); // cyan-ish, just for the swatch

	m_ui->buildControlPanel(m_engine, {curve1, curve2, wfDesc});

	// Trigger settings live in a global (non-curve) group at the bottom of
	// the Settings panel. Build the widget manually so we can wire it to
	// the DataStore's keysChanged signal for live key-combo refresh.
	if(m_trigProc) {
		auto *body = new QWidget(m_ui);
		auto *lay  = new QVBoxLayout(body);
		lay->setContentsMargins(0, 0, 0, 0);
		lay->setSpacing(4);
		// Base ProcessorBlock widget (enable checkbox etc.) — call the
		// base virtual explicitly to avoid TriggerProcessor's override
		// creating a second TriggerProcessorWidget internally.
		lay->addWidget(m_trigProc->ProcessorBlock::createSettingsWidget(body));
		m_trigWidget = new scopy::acq::TriggerProcessorWidget(m_trigProc, body);
		lay->addWidget(m_trigWidget);
		m_ui->addGlobalWidgetGroup(m_trigProc->name(), body);
		m_trigWidget->setMaxTargetSample(
			std::max(0, static_cast<int>(m_engine->bufferSize()) - 1));
	}

	// ---- Trigger sample-position handle on plot X axis ----
	// Visible only when the trigger processor is in sample-specific mode.
	// Draggable in that mode → writes back to processor via setTargetSample.
	// Otherwise repositioned by fired(atSample) as a read-only indicator.
	if(m_trigProc && m_ui && m_ui->m_plot && m_ui->m_plot->xAxis()) {
		m_triggerHandle = new scopy::PlotAxisHandle(m_ui->m_plot, m_ui->m_plot->xAxis());
		m_triggerHandle->handle()->setBarVisibility(scopy::BarVisibility::ALWAYS);
		// Sample-specific mode: handle sits below the plot canvas so it
		// doesn't collide with the top toolbar / channel labels.
		m_triggerHandle->handle()->setHandlePos(scopy::HandlePos::SOUTH_OR_EAST);
		m_ui->m_plot->addPlotAxisHandle(m_triggerHandle);
		if(auto *ah = m_triggerHandle->handle()) {
			if(auto *canvas = m_ui->m_plot->plot()->canvas()) {
				ah->setParent(canvas);
				ah->resize(canvas->size());
			}
		}
		// Handle is visible whenever the trigger processor is enabled,
		// regardless of mode. Hidden otherwise.
		const bool initVis = m_trigProc->isEnabled();
		m_triggerHandle->setVisible(initVis);
		if(auto *ah = m_triggerHandle->handle())
			ah->setVisible(initVis);

		// Drag semantics:
		//   • sample-specific: handle is a sample-index picker.
		//   • scan mode: user just picked a new canvas fraction — the
		//     handle stays where the user put it, and the axis shifts
		//     so the fired sample lands under it. If no fire has been
		//     recorded yet we only remember the fraction; the next
		//     fire will realign.
		connect(m_triggerHandle, &scopy::PlotAxisHandle::scalePosChanged, this,
			[this](double pos) {
				if(!m_trigProc) return;
				if(m_trigProc->sampleSpecific()) {
					// Convert axis position to a plot-window index,
					// then to a chunk-local index (targetSample lives
					// in chunk coordinates, matching the fired()
					// payload).
					const int chunkSize = m_engine
						? static_cast<int>(m_engine->bufferSize())
						: m_plotSize;
					const int rightEdge = m_lastPlotX.isEmpty()
						? m_plotSize
						: m_lastPlotX.size();
					int plotIdx;
					if(m_lastPlotX.isEmpty()) {
						plotIdx = static_cast<int>(std::round(pos));
					} else {
						// Nearest-neighbor along X array.
						const float t = static_cast<float>(pos);
						int best = 0;
						float bestDist = std::abs(m_lastPlotX[0] - t);
						for(int i = 1; i < m_lastPlotX.size(); ++i) {
							const float d = std::abs(m_lastPlotX[i] - t);
							if(d < bestDist) { bestDist = d; best = i; }
						}
						plotIdx = best;
					}
					plotIdx = std::clamp(plotIdx, 0, std::max(0, rightEdge - 1));
					const int chunkIdx = std::clamp(
						plotIdx - (rightEdge - chunkSize),
						0, std::max(0, chunkSize - 1));
					m_trigProc->setTargetSample(static_cast<quint32>(chunkIdx));
				} else {
					// Convert axis-space drop position back to a
					// canvas fraction using the current interval.
					if(!m_ui || !m_ui->m_plot || !m_ui->m_plot->xAxis())
						return;
					auto *ax = m_ui->m_plot->xAxis();
					const double W = ax->max() - ax->min();
					if(W <= 0.0) return;
					m_handleFraction = std::clamp((pos - ax->min()) / W, 0.0, 1.0);
					align();
					if(m_ui && m_ui->m_plot) m_ui->m_plot->replot();
				}
			});

		// Fire:
		//   • sample-specific: snap handle to fired sample (indicator).
		//   • scan mode: cache fired sample and align axis so it lands
		//     at the handle's current fraction.
		connect(m_trigProc, &scopy::acq::TriggerProcessor::fired, this,
			[this](quint32 atSample) {
				if(!m_triggerHandle) return;
				if(m_trigProc && m_trigProc->sampleSpecific()) {
					m_triggerHandle->setPositionSilent(axisPosForSample(atSample));
				} else {
					m_lastFiredSample = atSample;
					m_haveLastFired   = true;
					align();
				}
			}, Qt::QueuedConnection);

		// Mode toggle: update visibility. In sample-specific mode the
		// handle snaps to the current target (picker/indicator). In
		// scan mode the handle keeps its fraction; alignment happens
		// on the next fire.
		connect(m_trigProc, &scopy::acq::TriggerProcessor::sampleSpecificChanged, this,
			[this](bool on) {
				if(!m_triggerHandle) return;
				const bool vis = m_trigProc->isEnabled();
				m_triggerHandle->setVisible(vis);
				if(auto *ah = m_triggerHandle->handle())
					ah->setVisible(vis);
				if(on) {
					m_triggerHandle->setPositionSilent(
						axisPosForSample(m_trigProc->targetSample()));
				} else {
					// Entering scan mode: drop stale fire anchor.
					// Fraction is preserved — user's chosen handle
					// position carries over.
					m_haveLastFired = false;
				}
				if(m_ui && m_ui->m_plot) m_ui->m_plot->replot();
			});

		// Programmatic target change (spinbox / single-shot latch).
		connect(m_trigProc, &scopy::acq::TriggerProcessor::targetSampleChanged, this,
			[this](quint32 s) {
				if(!m_triggerHandle) return;
				m_triggerHandle->setPositionSilent(axisPosForSample(s));
			});

		// External axis-scale changes (autoscaler tick, refreshPlotAxis,
		// user pan/zoom) would otherwise slide the fired sample out from
		// under the handle. Re-align on every scale update; the m_aligning
		// guard breaks recursion when the source is our own setInterval().
		connect(m_ui->m_plot->xAxis(), &PlotAxis::axisScaleUpdated, this,
			&SimInstrumentController::onAxisScaleUpdated);
	}

	// ---- Decoders panel (right stack) ----
	m_decoderPanel = new DecoderPanel(m_decoderMgr, m_store,
	                                  m_decoderCatalog.get(), m_ui);
	m_decoderPanel->setLogger(m_decoderLogger);
	m_ui->registerDecoderPanel(m_decoderPanel);

	// Refresh the newly added editor's channel combos with the current
	// DataStore key set — otherwise editors created before/between runs
	// (i.e. before onCycleComplete has fired for the new key set) come up
	// with empty channel dropdowns.
	connect(m_decoderMgr, &scopy::adc::DecoderManager::decoderAdded, m_decoderPanel,
		[this](const QString &) {
			if(m_decoderPanel && m_store)
				m_decoderPanel->refreshKeys(m_store->keys());
		});

	// Wire waterfall history spinbox → update widget + DataStore history depth.
	connect(m_ui, &SimInstrument::waterfallRowsChanged, this, [this](int rows) {
		m_currentWaterfallRows = rows;
		m_ui->m_waterfall->setNumRows(rows);
		if(!m_fftWaterfallKey.key.isEmpty())
			m_store->setHistorySize(m_fftWaterfallKey, static_cast<std::size_t>(rows));
	});

	// ---- Central DataStore key-set → GUI wiring ----
	// Every consumer that needs the current key list subscribes here rather
	// than polling from onCycleComplete(). keysChanged is emitted from the
	// engine worker thread on new-key insertion (see DataStore::write) and
	// on removal/reset, so cross-thread delivery uses Qt::QueuedConnection.
	connect(m_store, &scopy::acq::DataStore::keysChanged, this,
		[this](const QList<scopy::acq::DataKey> &keys) {
			if(m_ui)
				m_ui->updateCurveKeyCombos(keys);
			if(m_decoderPanel)
				m_decoderPanel->refreshKeys(keys);
			if(m_trigWidget) {
				QStringList sl;
				sl.reserve(keys.size());
				for(const auto &k : keys)
					sl << k.key;
				m_trigWidget->setAvailableKeys(sl);
			}
		}, Qt::QueuedConnection);

	// Prime the GUI with whatever keys already exist (usually none at init,
	// but harmless if the store was pre-populated).
	{
		const QList<scopy::acq::DataKey> keys = m_store->keys();
		m_ui->updateCurveKeyCombos(keys);
		if(m_decoderPanel)
			m_decoderPanel->refreshKeys(keys);
		if(m_trigWidget) {
			QStringList sl;
			sl.reserve(keys.size());
			for(const auto &k : keys)
				sl << k.key;
			m_trigWidget->setAvailableKeys(sl);
		}
	}

	refreshPlotAxis();
}

void SimInstrumentController::stop()
{
	if(!m_engine)
		return;
	// Restore the engine's saved maxFPS and reconnect cycleComplete if we
	// were in trigger-replot mode. This keeps the engine in a well-known
	// state across start/stop cycles.
	if(m_triggerReplotEnabled)
		setTriggerReplotEnabled(false);
	if(m_trigBinder)
		m_trigBinder->disarmSingleShot();
	if(m_displayTimer)
		m_displayTimer->stop();
	if(m_cycleConn) {
		disconnect(m_cycleConn);
		m_cycleConn = {};
	}
	m_engine->stop();
	m_dataDirty = false;
}

SimInstrument *SimInstrumentController::ui() const
{
	return m_ui;
}

void SimInstrumentController::refreshPlotAxis()
{
	if(!m_ui)
		return;

	if(m_indexBuf.size() != m_plotSize) {
		m_indexBuf.resize(m_plotSize);
		std::iota(m_indexBuf.begin(), m_indexBuf.end(), 0.0f);
	}

	m_ui->m_plot->xAxis()->setInterval(0.0, static_cast<double>(m_plotSize - 1));
}

bool SimInstrumentController::scanActive() const
{
	return m_trigProc && m_trigProc->isEnabled() && !m_trigProc->sampleSpecific();
}

double SimInstrumentController::axisPosForSample(quint32 s) const
{
	// `s` is a chunk-local index emitted by TriggerProcessor::fired
	// (0..bufferSize-1). The plot window aggregates one or more chunks
	// with the newest on the right, so the newest chunk occupies the
	// rightmost `bufferSize` positions of m_lastPlotX (or of the
	// [0..m_plotSize-1] index axis when no X-key is selected).
	const int chunkSize = m_engine
		? static_cast<int>(m_engine->bufferSize())
		: m_plotSize;
	// Right-edge of the plot window measured in whichever units we live in:
	//   • X-key selected:  m_lastPlotX.size()
	//   • sample-index:    m_plotSize
	const int rightEdge = m_lastPlotX.isEmpty()
		? m_plotSize
		: m_lastPlotX.size();
	const int chunkIdx  = std::clamp(static_cast<int>(s), 0, std::max(0, chunkSize - 1));
	const int plotIdx   = rightEdge - chunkSize + chunkIdx;
	if(plotIdx < 0)
		return m_lastPlotX.isEmpty() ? 0.0 : static_cast<double>(m_lastPlotX.first());
	const int clamped = std::clamp(plotIdx, 0, std::max(0, rightEdge - 1));
	if(m_lastPlotX.isEmpty())
		return static_cast<double>(clamped);
	return static_cast<double>(m_lastPlotX[clamped]);
}

void SimInstrumentController::align()
{
	if(m_aligning) return;
	if(!scanActive() || !m_haveLastFired || !m_triggerHandle)
		return;
	if(!m_ui || !m_ui->m_plot || !m_ui->m_plot->xAxis())
		return;
	auto *ax = m_ui->m_plot->xAxis();
	const double W = ax->max() - ax->min();
	if(W <= 0.0)
		return;
	const double firedX = axisPosForSample(m_lastFiredSample);
	const double frac   = std::clamp(m_handleFraction, 0.0, 1.0);
	const double newMin = firedX - frac * W;

	m_aligning = true;
	ax->setInterval(newMin, newMin + W);
	// Glue the handle onto the fired sample's axis value. axisScaleUpdated
	// from setInterval() will fire onAxisScaleUpdated(), but m_aligning
	// short-circuits it.
	m_triggerHandle->setPositionSilent(firedX);
	m_aligning = false;
}

void SimInstrumentController::onAxisScaleUpdated()
{
	if(m_aligning) return;
	if(!scanActive() || !m_haveLastFired) return;
	// External change to the interval (autoscaler / refreshPlotAxis /
	// pan / zoom). Re-assert the invariant: fired sample under handle
	// at m_handleFraction.
	align();
}

void SimInstrumentController::resetLiveBuffers()
{
	// Drop all cached read windows so no prior-run buffer is reachable
	// through m_live[X|Y|X2|Y2] on the next cycle.
	m_liveX  = QVector<float>{};
	m_liveY  = QVector<float>{};
	m_liveX2 = QVector<float>{};
	m_liveY2 = QVector<float>{};
	m_scratchX.clear();
	m_scratchY.clear();
	m_scratchX2.clear();
	m_scratchY2.clear();
	m_lastPlotX.clear();

	// Also drop any pending fire snapshot: it's no longer relevant to
	// the fresh run.
	m_firedSnapshot.clear();

	// Detach Qwt curves from whatever pointers they were holding. With
	// copy=true this is defensive rather than strictly needed, but it
	// also makes the plot visually blank until fresh data arrives —
	// preferable to briefly showing the previous run's tail. We pass a
	// single dummy sample rather than size=0 to sidestep any Qwt path
	// that dereferences the data pointer even for empty ranges.
	static const float kZero = 0.0f;
	if(m_curve)
		m_curve->setSamples(&kZero, &kZero, 0, true);
	if(m_curve2)
		m_curve2->setSamples(&kZero, &kZero, 0, true);
}

void SimInstrumentController::setTriggerReplotEnabled(bool en)
{
	if(en == m_triggerReplotEnabled)
		return;
	m_triggerReplotEnabled = en;

	if(en) {
		// Force engine to free-run: setMaxFPS(0) removes the per-cycle
		// sleep on the Triggered branch of the loop; Continuous never
		// sleeps. We also force Mode::Continuous — with the trigger
		// gating replot, per-cycle emission is what we want.
		m_savedMaxFPS = m_engine->maxFPS();
		m_savedMode   = m_engine->mode();
		m_engine->setMaxFPS(0);
		m_engine->setMode(scopy::acq::AcquisitionEngine::Mode::Continuous);
		if(m_ui && m_ui->m_modeCombo) {
			QSignalBlocker b(m_ui->m_modeCombo);
			m_ui->m_modeCombo->setCurrentIndex(0); // Continuous
		}

		// Disconnect cycleComplete — only trigger fires should mark data
		// dirty. Otherwise every free-running cycle would replot.
		if(m_cycleConn) {
			disconnect(m_cycleConn);
			m_cycleConn = {};
		}

		// Keep m_displayTimer running: it enforces the replot rate cap.
		// Trigger fires only mark m_dataDirty; the timer coalesces them
		// to at most 1 replot every 1000/savedMaxFPS ms.
		if(m_displayTimer) {
			const int intervalMs = (m_savedMaxFPS > 0)
				? std::max(1, 1000 / static_cast<int>(m_savedMaxFPS))
				: 16;
			m_displayTimer->setInterval(intervalMs);
		}

		if(m_trigBinder)
			m_trigBinder->bindReplotOnFire();

		// Entering scan mode: drop stale anchor. The handle keeps its
		// current position — the axis will move to align with it on
		// the next fire.
		if(scanActive())
			m_haveLastFired = false;
		if(m_triggerHandle) {
			m_triggerHandle->setVisible(true);
			if(auto *ah = m_triggerHandle->handle())
				ah->setVisible(true);
			if(m_ui && m_ui->m_plot) m_ui->m_plot->replot();
		}
	} else {
		if(m_trigBinder)
			m_trigBinder->unbindReplotOnFire();
		if(m_triggerHandle) {
			m_haveLastFired = false;
			m_triggerHandle->setVisible(false);
			if(auto *ah = m_triggerHandle->handle())
				ah->setVisible(false);
			if(m_ui && m_ui->m_plot) m_ui->m_plot->replot();
		}
		m_engine->setMaxFPS(m_savedMaxFPS);
		m_engine->setMode(m_savedMode);
		if(m_ui && m_ui->m_modeCombo) {
			QSignalBlocker b(m_ui->m_modeCombo);
			m_ui->m_modeCombo->setCurrentIndex(
				m_savedMode == scopy::acq::AcquisitionEngine::Mode::Continuous ? 0 : 1);
		}
		if(!m_cycleConn)
			m_cycleConn = connect(m_engine,
				&scopy::acq::AcquisitionEngine::cycleComplete,
				this, &SimInstrumentController::onCycleComplete,
				Qt::QueuedConnection);
		if(m_displayTimer)
			m_displayTimer->setInterval(16);
	}
}

void SimInstrumentController::onTriggerFired(quint32 /*sampleIndex*/,
	QMap<QString, scopy::acq::SampleVariant> snapshot)
{
	if(!m_ui || !m_engine)
		return;
	// Route reads through the fire-time snapshot delivered inside the
	// fired() signal so we plot the exact cycle that fired, not
	// whatever the free-running worker has since replaced it with.
	m_firedSnapshot = std::move(snapshot);
	onCycleComplete();
	m_firedSnapshot.clear();
}

void SimInstrumentController::setCurveDriven(PlotChannel *ch, bool driven)
{
	if(!ch || ch->isEnabled() == driven)
		return;
	if(driven) {
		ch->enable();
		m_autoscalerX->addChannels(ch);
		m_autoscalerY->addChannels(ch);
	} else {
		ch->disable();
		m_autoscalerX->removeChannels(ch);
		m_autoscalerY->removeChannels(ch);
	}
}

// Convert any SampleVariant type to QVector<float> so that non-float sources
// (e.g. M2kLogicSource which stores QVector<quint8>) can be plotted on the
// same curves as float sources.
static QVector<float> toFloatVec(const scopy::acq::SampleVariant &v)
{
	return std::visit([](const auto &vec) -> QVector<float> {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<VecT, QVector<scopy::acq::Annotation>>) {
			// Annotations are not numerical; return empty so plotting
			// paths silently skip decoded keys.
			return QVector<float>{};
		} else {
			QVector<float> out;
			out.reserve(static_cast<int>(vec.size()));
			for(const auto &s : vec)
				out.append(static_cast<float>(s));
			return out;
		}
	}, v);
}

void SimInstrumentController::onCycleComplete()
{
	if(!m_ui)
		return;

	// FPS update (every 500 ms)
	++m_cycleCount;
	const qint64 elapsed = m_fpsTimer.elapsed();
	if(elapsed >= 500) {
		const double fps = m_cycleCount * 1000.0 / elapsed;
		m_fpsLabel->setText(QString::number(fps, 'f', 1) + " FPS");
		m_cycleCount = 0;
		m_fpsTimer.restart();
	}

	// Key-set refresh is now driven by DataStore::keysChanged (see init()).
	// No per-cycle polling here.

	// Refresh the DataStore inspector panel every cycle
	m_ui->refreshDatastoreView(m_store);

	// Read axis selections from the auto-generated per-curve combos
	const QString xKeyStr  = m_ui->curveXKey(0);
	const QString yKeyStr  = m_ui->curveYKey(0);
	const QString x2KeyStr = m_ui->curveXKey(1);
	const QString y2KeyStr = m_ui->curveYKey(1);

	// Decoder annotations are drawn for every registered decoder,
	// independent of the curve Y-combo selection. The curve combos only
	// control the numeric X/Y traces below.
	const bool xIsIndex  = xKeyStr.isEmpty();
	const bool yIsIndex  = yKeyStr.isEmpty();
	const bool x2IsIndex = x2KeyStr.isEmpty();
	const bool y2IsIndex = y2KeyStr.isEmpty();

	// When a trigger fire is being serviced, prefer the fire-time snapshot
	// delivered inside the fired() signal over the live store. The store
	// may already contain newer chunks that displaced the fire-cycle's
	// data from history (armed-continuous is free-running).
	auto readWindow = [this](const QString &keyStr) -> scopy::acq::SampleVariant {
		if(!m_firedSnapshot.isEmpty()) {
			auto it = m_firedSnapshot.constFind(keyStr);
			if(it != m_firedSnapshot.constEnd())
				return it.value();
			// Fall through to live read for keys not present in the
			// snapshot (e.g. GUI added a new curve after the fire).
		}
		return m_store->readWindowNative(scopy::acq::DataKey(keyStr), m_plotSize);
	};

	if(!xIsIndex)  m_liveX  = readWindow(xKeyStr);
	else           m_liveX  = QVector<float>{};
	if(!yIsIndex)  m_liveY  = readWindow(yKeyStr);
	else           m_liveY  = QVector<float>{};
	if(!x2IsIndex) m_liveX2 = readWindow(x2KeyStr);
	else           m_liveX2 = QVector<float>{};
	if(!y2IsIndex) m_liveY2 = readWindow(y2KeyStr);
	else           m_liveY2 = QVector<float>{};

	auto toFloatView = [](const scopy::acq::SampleVariant &v,
			      QVector<float> &scratch)
		-> std::pair<const float *, int> {
		return std::visit(
			[&](const auto &vec) -> std::pair<const float *, int> {
				using VecT = std::decay_t<decltype(vec)>;
				if constexpr(std::is_same_v<VecT, QVector<scopy::acq::Annotation>>) {
					return {nullptr, 0};
				} else if constexpr(std::is_same_v<VecT, QVector<float>>) {
					return {vec.constData(), vec.size()};
				} else {
					scratch.resize(vec.size());
					const auto *src = vec.constData();
					float *dst = scratch.data();
					for(int i = 0; i < vec.size(); ++i)
						dst[i] = static_cast<float>(src[i]);
					return {scratch.constData(), scratch.size()};
				}
			},
			v);
	};

	const auto xView  = xIsIndex  ? std::pair<const float *, int>{nullptr, 0}
				      : toFloatView(m_liveX,  m_scratchX);
	const auto yView  = yIsIndex  ? std::pair<const float *, int>{nullptr, 0}
				      : toFloatView(m_liveY,  m_scratchY);
	const auto x2View = x2IsIndex ? std::pair<const float *, int>{nullptr, 0}
				      : toFloatView(m_liveX2, m_scratchX2);
	const auto y2View = y2IsIndex ? std::pair<const float *, int>{nullptr, 0}
				      : toFloatView(m_liveY2, m_scratchY2);

	// Snapshot the current X-axis array so the trigger handle can map
	// its scale-space position (axis units) to a chunk sample index and
	// vice versa. Empty when the user selected "Sample Index" — mapping
	// then falls back to identity.
	if(xIsIndex) {
		m_lastPlotX.clear();
	} else if(xView.first && xView.second > 0) {
		m_lastPlotX.resize(xView.second);
		std::memcpy(m_lastPlotX.data(), xView.first, xView.second * sizeof(float));
	}

	// Map annotation sample indices [0..m_plotSize) proportionally across
	// whatever x-scale the plot currently uses, so decoder annotations
	// stay aligned with the waveform curves regardless of the selected
	// x-key (index, time, frequency, …).
	if(m_decoderOverlay)
		m_decoderOverlay->setSampleCount(static_cast<quint64>(m_plotSize));

	// Refresh raw digital tracks (0/1 waveforms) from the DataStore.
	// Decoder annotation bands are pushed separately by DecoderOverlay
	// on cycleProduced.
	if(m_digitalMgr)
		m_digitalMgr->updateRawCurves(m_plotSize);

	const bool curve1Driven = (xIsIndex || xView.second  > 0) &&
				  (yIsIndex || yView.second  > 0);
	const bool curve2Driven = (x2IsIndex || x2View.second > 0) &&
				  (y2IsIndex || y2View.second > 0);
	setCurveDriven(m_curve,  curve1Driven);
	setCurveDriven(m_curve2, curve2Driven);

	if(!curve1Driven)
		return;

	int n;
	if(!xIsIndex && !yIsIndex)
		n = qMin(xView.second, yView.second);
	else if(!xIsIndex)
		n = xView.second;
	else if(!yIsIndex)
		n = yView.second;
	else
		n = m_plotSize; // both sample-index — draw full plot span

	if(n <= 0)
		return;

	const int   idxOffset = qMax(0, m_indexBuf.size() - n);
	const float *xPtr = xIsIndex ? (m_indexBuf.data() + idxOffset) : xView.first;
	const float *yPtr = yIsIndex ? (m_indexBuf.data() + idxOffset) : yView.first;

	// copy=true: Qwt keeps its own buffer. Prevents stale-pointer aliasing
	// on m_liveX/Y across Stop/Run boundaries and out-of-band replots
	// (decoder overlay, digital tracks, trigger handle drags) that could
	// otherwise paint the tail of a prior run.
	m_curve->setSamples(xPtr, yPtr, static_cast<size_t>(n), true);

	// Curve 2
	[&] {
		if(!curve2Driven)
			return;

		const float *y2Ptr;
		int n2;
		if(y2IsIndex) {
			y2Ptr = m_indexBuf.data() + idxOffset;
			n2    = n;
		} else {
			n2    = y2View.second;
			y2Ptr = y2View.first;
		}

		const float *x2Ptr;
		if(x2IsIndex) {
			const int off2 = qMax(0, m_indexBuf.size() - n2);
			x2Ptr = m_indexBuf.data() + off2;
			n2    = qMin(n2, m_indexBuf.size() - off2);
		} else {
			n2    = qMin(n2, x2View.second);
			x2Ptr = x2View.first;
		}
		m_curve2->setSamples(x2Ptr, y2Ptr, static_cast<size_t>(n2), true);
	}();

	// Feed waterfall from the key selected in the Waterfall Y combo (index 2).
	const QString wfYKeyStr = m_ui->curveYKey(2);
	if(!wfYKeyStr.isEmpty()) {
		const scopy::acq::DataKey wfYKey(wfYKeyStr);

		// Key changed: migrate history-size budget to the new key.
		if(wfYKey != m_fftWaterfallKey) {
			if(!m_fftWaterfallKey.key.isEmpty())
				m_store->setHistorySize(m_fftWaterfallKey, 1);
			m_fftWaterfallKey = wfYKey;
			m_store->setHistorySize(m_fftWaterfallKey,
						static_cast<std::size_t>(m_currentWaterfallRows));
		}

		// Update X frequency axis from the selected X key (if any).
		const QString wfXKeyStr = m_ui->curveXKey(2);
		if(!wfXKeyStr.isEmpty()) {
			const scopy::acq::SampleBuffer xBuf = m_store->read(scopy::acq::DataKey(wfXKeyStr));
			if(!xBuf.empty()) {
				const QVector<float> freq = toFloatVec(xBuf.sample(0));
				if(freq.size() >= 2)
					m_ui->m_waterfall->setFrequencyRange(freq.first(), freq.last());
			}
		}

		// Build and push history snapshot.
		const scopy::acq::SampleBuffer yBuf = m_store->read(m_fftWaterfallKey);
		if(!yBuf.empty()) {
			std::vector<QVector<float>> snap;
			snap.reserve(yBuf.depth());
			for(std::size_t i = 0; i < yBuf.depth(); ++i) {
				snap.push_back(toFloatVec(yBuf.sample(i)));
			}

			// TODO: temporary waterfall intensity autoscaling — should be reworked
			// (e.g. via PlotAutoscaler or a dedicated WaterfallAutoscaler) and removed.
			if(!snap.empty()) {
				const QVector<float> &newest = snap[0];
				for(float v : newest) {
					if(v < m_wfAutoMin) m_wfAutoMin = v;
					if(v > m_wfAutoMax) m_wfAutoMax = v;
				}
				if(m_wfAutoMin < m_wfAutoMax)
					m_ui->m_waterfall->setIntensityRange(m_wfAutoMin, m_wfAutoMax);
			}

			m_ui->m_waterfall->setHistorySnapshot(std::move(snap));
		}
	}

	m_dataDirty = true;
}

} // namespace adc
} // namespace scopy
