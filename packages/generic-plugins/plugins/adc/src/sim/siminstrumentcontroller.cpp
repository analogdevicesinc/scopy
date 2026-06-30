#include "siminstrumentcontroller.h"

#include "DecoderOverlay.h"
#include <core/decoder/SigrokCliBackend.h>

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

		// ---- Hardcoded UART decoder demo on DIO0 ----
		auto backend = std::make_unique<scopy::decoder::SigrokCliBackend>();
		m_uartDecoder = new scopy::acq::ExternalDecoderProcessor(
			"uart-decoder", std::move(backend), m_engine);

		scopy::decoder::DecoderConfig cfg;
		cfg.decoderId   = "uart";
		cfg.sampleRate  = 1.0e6;
		cfg.numChannels = 1;
		cfg.channels    = {{"rx", 0}};
		cfg.options     = {{"baudrate", "115200"}, {"parity", "none"}};

		m_uartDecoder->setConfig(cfg);
		m_uartDecoder->setOrderedRawKeys(
			{scopy::acq::DataKey::raw("m2k_logic", "DIO0")});
		m_uartDecoder->setOutputKey(
			scopy::acq::DataKey::withStage("m2k_logic", "uart-decoder", "decoded"));
		m_uartDecoder->setWatchedKeys(
			{scopy::acq::DataKey::raw("m2k_logic", "DIO0")});
		m_engine->addProcessor(m_uartDecoder);
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

	// ---- Decoder overlay (after UI is built) ----
	if(m_uartDecoder) {
		m_decoderOverlay = new scopy::adc::DecoderOverlay(m_ui->m_plot, m_store, this);
		m_decoderOverlay->registerDecoder(m_uartDecoder);
	}

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
	connect(m_ui, &SimInstrument::requestRun,  m_engine, &scopy::acq::AcquisitionEngine::run);
	connect(m_ui, &SimInstrument::requestStop, m_engine, &scopy::acq::AcquisitionEngine::stop);
	connect(m_ui, &SimInstrument::requestSingle, this, [this]() { m_engine->single(1); });

	connect(m_ui, &SimInstrument::sampleSizeChanged, this, [this](int n) {
		m_engine->setBufferSize(static_cast<std::size_t>(n));
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

	// Wire waterfall history spinbox → update widget + DataStore history depth.
	connect(m_ui, &SimInstrument::waterfallRowsChanged, this, [this](int rows) {
		m_currentWaterfallRows = rows;
		m_ui->m_waterfall->setNumRows(rows);
		if(!m_fftWaterfallKey.key.isEmpty())
			m_store->setHistorySize(m_fftWaterfallKey, static_cast<std::size_t>(rows));
	});
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
	}

	// Refresh the DataStore inspector panel every cycle
	m_ui->refreshDatastoreView(m_store);

	// Read axis selections from the auto-generated per-curve combos
	const QString xKeyStr  = m_ui->curveXKey(0);
	const QString yKeyStr  = m_ui->curveYKey(0);
	const QString x2KeyStr = m_ui->curveXKey(1);
	const QString y2KeyStr = m_ui->curveYKey(1);

	const bool xIsIndex  = xKeyStr.isEmpty();
	const bool yIsIndex  = yKeyStr.isEmpty();
	const bool x2IsIndex = x2KeyStr.isEmpty();
	const bool y2IsIndex = y2KeyStr.isEmpty();

	QVector<float> xVec, yVec, y2Vec, x2Vec;

	if(!xIsIndex) {
		const scopy::acq::SampleBuffer xBuf = m_store->read(scopy::acq::DataKey(xKeyStr));
		if(xBuf.empty()) return;
		xVec = toFloatVec(xBuf.sample(0));
		if(xVec.isEmpty()) return;
	}

	if(!yIsIndex) {
		const scopy::acq::SampleBuffer yBuf = m_store->read(scopy::acq::DataKey(yKeyStr));
		if(yBuf.empty()) return;
		yVec = toFloatVec(yBuf.sample(0));
		if(yVec.isEmpty()) return;
	}

	if(!y2IsIndex) {
		const scopy::acq::SampleBuffer y2Buf = m_store->read(scopy::acq::DataKey(y2KeyStr));
		if(!y2Buf.empty())
			y2Vec = toFloatVec(y2Buf.sample(0));
	}

	if(!x2IsIndex) {
		const scopy::acq::SampleBuffer x2Buf = m_store->read(scopy::acq::DataKey(x2KeyStr));
		if(!x2Buf.empty())
			x2Vec = toFloatVec(x2Buf.sample(0));
	}

	// Determine primary sample count
	int n = static_cast<int>(m_engine->bufferSize());
	if(!xIsIndex && !yIsIndex)
		n = qMin(xVec.size(), yVec.size());
	else if(!xIsIndex)
		n = xVec.size();
	else if(!yIsIndex)
		n = yVec.size();

	if(n <= 0)
		return;

	// Rebuild shared index buffer if size changed
	if(xIsIndex || yIsIndex || y2IsIndex || x2IsIndex) {
		if(m_indexBuf.size() != n) {
			m_indexBuf.resize(n);
			std::iota(m_indexBuf.begin(), m_indexBuf.end(), 0.0f);
		}
		if(xIsIndex)
			m_ui->m_plot->xAxis()->setInterval(0.0, static_cast<double>(n - 1));
	}

	const float *xPtr = xIsIndex ? m_indexBuf.data() : xVec.data();
	const float *yPtr = yIsIndex ? m_indexBuf.data() : yVec.data();

	m_curve->setSamples(xPtr, yPtr, static_cast<size_t>(n), true);

	// Curve 2
	[&] {
		const float *y2Ptr;
		int n2;
		if(y2IsIndex) {
			y2Ptr = m_indexBuf.data();
			n2    = n;
		} else if(!y2Vec.isEmpty()) {
			n2    = y2Vec.size();
			y2Ptr = y2Vec.data();
		} else {
			// Y2 key not yet in store — skip this cycle
			return;
		}

		const float *x2Ptr;
		if(x2IsIndex) {
			x2Ptr = m_indexBuf.data();
			n2    = qMin(n2, static_cast<int>(m_indexBuf.size()));
		} else if(!x2Vec.isEmpty()) {
			n2    = qMin(n2, x2Vec.size());
			x2Ptr = x2Vec.data();
		} else {
			x2Ptr = m_indexBuf.data();
			n2    = qMin(n2, static_cast<int>(m_indexBuf.size()));
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
