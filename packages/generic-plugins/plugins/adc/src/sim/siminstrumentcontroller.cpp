#include "siminstrumentcontroller.h"

#include "DecoderOverlay.h"
#include "DecoderManager.h"
#include "DecoderPanel.h"

#include <core/decoder/SigrokCliBackendFactory.h>
#include <core/decoder/SigrokCliCatalog.h>

#include <libm2k/digital/m2kdigital.hpp>

#include <numeric>
#include <variant>
#include <vector>
#include <QCoreApplication>
#include <QDateTime>
#include <QLoggingCategory>
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

	// ---- UI ----
	m_ui = new SimInstrument(nullptr);

	// ---- Decoder plumbing (composition root) ----
	// The controller picks the concrete catalog + backend factory here.
	// DecoderManager and DecoderPanel are backend-agnostic — swap these
	// two lines to plug in a different decoder backend (e.g. libsigrok,
	// a bespoke CLI) and nothing else needs to change.
	m_decoderCatalog        = std::make_unique<scopy::decoder::SigrokCliCatalog>();
	m_decoderBackendFactory = std::make_unique<scopy::decoder::SigrokCliBackendFactory>();

	m_decoderOverlay = new scopy::adc::DecoderOverlay(m_ui->m_plot, m_store, this);
	m_decoderMgr = new DecoderManager(m_engine, m_store,
	                                  m_decoderBackendFactory.get(), this);
	m_decoderMgr->setPlot(m_ui->m_plot);
	m_decoderMgr->setOverlay(m_decoderOverlay);

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
		refreshPlotAxis();
		m_engine->run();
	});
	connect(m_ui, &SimInstrument::requestStop, m_engine, &scopy::acq::AcquisitionEngine::stop);
	connect(m_ui, &SimInstrument::requestSingle, this, [this]() {
		if(m_store) m_store->clear();
		refreshPlotAxis();
		const std::size_t n = scopy::acq::DataStore::requiredHistoryDepth(
			static_cast<std::size_t>(m_plotSize), m_engine->bufferSize());
		m_engine->single(static_cast<unsigned int>(n));
	});

	connect(m_ui, &SimInstrument::sampleSizeChanged, this, [this](int n) {
		m_engine->setBufferSize(static_cast<std::size_t>(n));
	});
	connect(m_ui, &SimInstrument::plotSizeChanged, this, [this](int n) {
		m_plotSize = std::max(1, n);
		refreshPlotAxis();
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

	// ---- Wire engine cycleComplete → plot update ----
	connect(m_engine, &scopy::acq::AcquisitionEngine::cycleComplete,
		this, &SimInstrumentController::onCycleComplete, Qt::QueuedConnection);

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

	// ---- Decoders panel (right stack) ----
	m_decoderPanel = new DecoderPanel(m_decoderMgr, m_store,
	                                  m_decoderCatalog.get(), m_ui);
	m_ui->registerDecoderPanel(m_decoderPanel);

	connect(m_engine, &scopy::acq::AcquisitionEngine::started, m_decoderPanel,
		[this]() { m_decoderPanel->setEngineRunning(true); });
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped, m_decoderPanel,
		[this]() { m_decoderPanel->setEngineRunning(false); });
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, m_decoderPanel,
		[this]() { m_decoderPanel->setEngineRunning(false); });

	// Refresh the newly added editor's channel combos with the current
	// DataStore key set — otherwise editors created before/between runs
	// (i.e. before onCycleComplete has fired for the new key set) come up
	// with empty channel dropdowns.
	connect(m_decoderMgr, &scopy::adc::DecoderManager::decoderAdded, m_decoderPanel,
		[this](const QString &) {
			if(m_decoderPanel && m_store)
				m_decoderPanel->refreshRawKeys(m_store->keys());
		});

	// Wire waterfall history spinbox → update widget + DataStore history depth.
	connect(m_ui, &SimInstrument::waterfallRowsChanged, this, [this](int rows) {
		m_currentWaterfallRows = rows;
		m_ui->m_waterfall->setNumRows(rows);
		if(!m_fftWaterfallKey.key.isEmpty())
			m_store->setHistorySize(m_fftWaterfallKey, static_cast<std::size_t>(rows));
	});

	refreshPlotAxis();
}

void SimInstrumentController::stop()
{
	if(!m_engine)
		return;
	if(m_displayTimer)
		m_displayTimer->stop();
	disconnect(m_engine, &scopy::acq::AcquisitionEngine::cycleComplete,
		   this, &SimInstrumentController::onCycleComplete);
	QCoreApplication::removePostedEvents(this);
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

	// Refresh combos if the key set changed
	const QList<scopy::acq::DataKey> currentKeys = m_store->keys();
	if(currentKeys != m_lastKeys) {
		m_lastKeys = currentKeys;
		m_ui->updateCurveKeyCombos(currentKeys);
		if(m_decoderPanel)
			m_decoderPanel->refreshRawKeys(currentKeys);
	}

	// Refresh the DataStore inspector panel every cycle
	m_ui->refreshDatastoreView(m_store);

	// Read axis selections from the auto-generated per-curve combos
	const QString xKeyStr  = m_ui->curveXKey(0);
	const QString yKeyStr  = m_ui->curveYKey(0);
	const QString x2KeyStr = m_ui->curveXKey(1);
	const QString y2KeyStr = m_ui->curveYKey(1);

	// Decoder overlays are shown only for the DataKeys currently selected
	// as a curve's Y source. This lets the user "view" a decoder key just
	// like a numeric one.
	if(m_decoderOverlay) {
		QList<scopy::acq::DataKey> selected;
		for(const QString &s : {yKeyStr, y2KeyStr})
			if(!s.isEmpty())
				selected.append(scopy::acq::DataKey(s));
		m_decoderOverlay->setVisibleKeys(selected);
	}

	const bool xIsIndex  = xKeyStr.isEmpty();
	const bool yIsIndex  = yKeyStr.isEmpty();
	const bool x2IsIndex = x2KeyStr.isEmpty();
	const bool y2IsIndex = y2KeyStr.isEmpty();

	QVector<float> xVec, yVec, y2Vec, x2Vec;

	if(!xIsIndex)
		xVec = m_store->readWindow(scopy::acq::DataKey(xKeyStr), m_plotSize);
	if(!yIsIndex)
		yVec = m_store->readWindow(scopy::acq::DataKey(yKeyStr), m_plotSize);
	if(!y2IsIndex)
		y2Vec = m_store->readWindow(scopy::acq::DataKey(y2KeyStr), m_plotSize);
	if(!x2IsIndex)
		x2Vec = m_store->readWindow(scopy::acq::DataKey(x2KeyStr), m_plotSize);

	if(m_decoderOverlay)
		m_decoderOverlay->setSampleCount(static_cast<quint64>(m_plotSize));

	// A curve is "driven" only if it has numeric samples this cycle. Keys
	// that produce no numeric data (e.g. decoder annotation keys) return
	// an empty QVector<float> from readWindow — for those we hide the
	// curve and drop it from the autoscalers so it doesn't skew Y range.
	const bool curve1Driven = (xIsIndex || !xVec.isEmpty()) &&
				  (yIsIndex || !yVec.isEmpty());
	const bool curve2Driven = (x2IsIndex || !x2Vec.isEmpty()) &&
				  (y2IsIndex || !y2Vec.isEmpty());
	setCurveDriven(m_curve,  curve1Driven);
	setCurveDriven(m_curve2, curve2Driven);

	if(!curve1Driven)
		return;

	// Determine primary sample count (assembled, right-anchored)
	int n;
	if(!xIsIndex && !yIsIndex)
		n = qMin(xVec.size(), yVec.size());
	else if(!xIsIndex)
		n = xVec.size();
	else if(!yIsIndex)
		n = yVec.size();
	else
		n = m_plotSize; // both sample-index — draw full plot span

	if(n <= 0)
		return;

	// Sample-Index X: point into the tail of the static [0..plotSize-1] buffer
	// so that the newest chunk lines up at the right edge of the plot.
	const int   idxOffset = qMax(0, m_indexBuf.size() - n);
	const float *xPtr = xIsIndex ? (m_indexBuf.data() + idxOffset) : xVec.data();
	const float *yPtr = yIsIndex ? (m_indexBuf.data() + idxOffset) : yVec.data();

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
			n2    = y2Vec.size();
			y2Ptr = y2Vec.data();
		}

		const float *x2Ptr;
		if(x2IsIndex) {
			const int off2 = qMax(0, m_indexBuf.size() - n2);
			x2Ptr = m_indexBuf.data() + off2;
			n2    = qMin(n2, m_indexBuf.size() - off2);
		} else {
			n2    = qMin(n2, x2Vec.size());
			x2Ptr = x2Vec.data();
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
