#include "siminstrumentcontroller.h"

#include <numeric>
#include <QCoreApplication>
#include <QDateTime>
#include <QLoggingCategory>
#include <QPen>
#include <Qt>
#include <plotlegend.h>

#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/plotinfowidgets.h>
#include <gui/widgets/plotinfo.h>

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

void SimInstrumentController::init(iio_context *ctx)
{
	// ---- Engine + store ----
	m_store  = new sim::DataStore(this);
	m_engine = new sim::AcquisitionEngine(m_store, this);
	m_engine->setBufferSize(1024);
	m_engine->setMaxFPS(30);

	// ---- Sources ----
	m_src = new sim::SimulatedSource("sim-adc", m_engine);
	m_src->enableChannel("voltage0", true);
	m_src->enableChannel("voltage1", true);
	m_engine->addSource(m_src);

	if(ctx) {
		m_plutoSrc = new sim::PlutoIIOSource(ctx, "pluto", "cf-ad9361-lpc", m_engine);
		m_plutoSrc->enableChannel("voltage0", true);
		m_plutoSrc->enableChannel("voltage1", true);
		m_engine->addSource(m_plutoSrc);

		m_fftProc = new sim::GenalyzerFFTProcessor(
			sim::DataKey::raw("pluto", "voltage0"),
			sim::DataKey::raw("pluto", "voltage1"),
			sim::DataKey::withStage("pluto", "iq", "fft"),
			sim::DataKey::withStage("pluto", "iq", "freq"),
			static_cast<int>(m_engine->bufferSize()),
			/*sampleRate=*/2.4e6,
			GnWindowHann,
			m_engine);
		m_engine->addProcessor(m_fftProc);
	}

	// ---- Processor: scale + offset ----
	m_scaleProc = new sim::ScaleOffsetProcessor("scale-offset", this);
	m_scaleProc->addChannel(sim::DataKey::raw("sim-adc", "voltage0"),
				sim::DataKey::withStage("sim-adc", "voltage0", "scaled"),
				"voltage0");
	m_scaleProc->addChannel(sim::DataKey::raw("sim-adc", "voltage1"),
				sim::DataKey::withStage("sim-adc", "voltage1", "scaled"),
				"voltage1");
	m_engine->addProcessor(m_scaleProc);

	// ---- Math source + processor ----
	m_mathSrc = new sim::MathSource("math-src", m_engine);
	m_engine->addSource(m_mathSrc);

	m_mathProc = new sim::MathProcessor("math", m_engine);
	m_mathProc->configure(m_mathSrc->outputKey(),
			      sim::DataKey::withStage("math-src", "out", "proc"));
	m_engine->addProcessor(m_mathProc);

	// ---- UI ----
	m_ui = new SimInstrument(nullptr);

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
	connect(m_engine, &sim::AcquisitionEngine::started,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::start);
	connect(m_engine, &sim::AcquisitionEngine::stopped,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::stop);
	connect(m_engine, &sim::AcquisitionEngine::forceStopped,
		m_autoscalerY, &scopy::gui::PlotAutoscaler::stop);

	m_autoscalerX = new scopy::gui::PlotAutoscaler(this);
	m_autoscalerX->setXAxisMode(true);
	m_autoscalerX->addChannels(m_curve);
	m_autoscalerX->addChannels(m_curve2);
	connect(m_autoscalerX, &scopy::gui::PlotAutoscaler::newMin,
		m_ui->m_plot->xAxis(), &PlotAxis::setMin);
	connect(m_autoscalerX, &scopy::gui::PlotAutoscaler::newMax,
		m_ui->m_plot->xAxis(), &PlotAxis::setMax);
	connect(m_engine, &sim::AcquisitionEngine::started,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::start);
	connect(m_engine, &sim::AcquisitionEngine::stopped,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::stop);
	connect(m_engine, &sim::AcquisitionEngine::forceStopped,
		m_autoscalerX, &scopy::gui::PlotAutoscaler::stop);

	// ---- Wire UI buttons → engine ----
	connect(m_ui, &SimInstrument::requestRun,  m_engine, &sim::AcquisitionEngine::run);
	connect(m_ui, &SimInstrument::requestStop, m_engine, &sim::AcquisitionEngine::stop);
	connect(m_ui, &SimInstrument::requestSingle, this, [this]() { m_engine->single(1); });

	connect(m_ui, &SimInstrument::sampleSizeChanged, this, [this](int n) {
		m_engine->setBufferSize(static_cast<std::size_t>(n));
	});
	connect(m_ui, &SimInstrument::maxFpsChanged, this, [this](int fps) {
		m_engine->setMaxFPS(static_cast<unsigned int>(fps));
	});
	connect(m_ui, &SimInstrument::acqModeChanged, this, [this](int idx) {
		m_engine->setMode(idx == 0
			? sim::AcquisitionEngine::Mode::Continuous
			: sim::AcquisitionEngine::Mode::Triggered);
	});

	// ---- Wire engine signals → UI state ----
	connect(m_engine, &sim::AcquisitionEngine::started,      m_ui, &SimInstrument::onStarted);
	connect(m_engine, &sim::AcquisitionEngine::stopped,      m_ui, &SimInstrument::onStopped);
	connect(m_engine, &sim::AcquisitionEngine::forceStopped, m_ui, &SimInstrument::onForceStopped);

	// ---- Wire engine cycleComplete → plot update ----
	connect(m_engine, &sim::AcquisitionEngine::cycleComplete,
		this, &SimInstrumentController::onCycleComplete, Qt::QueuedConnection);

	// ---- Log errors ----
	connect(m_engine, &sim::AcquisitionEngine::error, this,
		[this](int severity, const QString &id, const QString &message) {
			const auto sev = static_cast<sim::AcquisitionError::Severity>(severity);
			const char *sevStr =
				sev == sim::AcquisitionError::Severity::Critical ? "CRITICAL" :
				sev == sim::AcquisitionError::Severity::Warning  ? "WARNING"  :
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
	connect(m_engine, &sim::AcquisitionEngine::started, this, [this]() {
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
	});
	connect(m_engine, &sim::AcquisitionEngine::started,      m_displayTimer, QOverload<>::of(&QTimer::start));
	connect(m_engine, &sim::AcquisitionEngine::stopped,      m_displayTimer, &QTimer::stop);
	connect(m_engine, &sim::AcquisitionEngine::forceStopped, m_displayTimer, &QTimer::stop);
	auto flushDirty = [this]() {
		if(m_dataDirty) {
			m_dataDirty = false;
			m_ui->m_plot->replot();
		}
	};
	connect(m_engine, &sim::AcquisitionEngine::stopped,      this, flushDirty);
	connect(m_engine, &sim::AcquisitionEngine::forceStopped, this, flushDirty);

	// ---- Cursors ----
	m_cursorCtrl = new scopy::CursorController(m_ui->m_plot, this);
	auto *cursorSettings = new scopy::CursorSettings(m_ui);
	m_cursorCtrl->connectSignals(cursorSettings);
	m_ui->m_tool->rightStack()->add("cursor-config", cursorSettings);
	connect(m_ui->m_cursorBtn, &QPushButton::toggled, m_cursorCtrl, &scopy::CursorController::setVisible);

	// ---- Build the auto-generated settings panel ----
	// Curve 1: sim-adc channels, ScaleOffsetProcessor settings
	SimInstrument::CurveDescriptor curve1;
	curve1.name  = "Curve 1";
	curve1.color = QColor(0x4a, 0xb8, 0xff);
	curve1.processors << m_scaleProc;

	// Curve 2: pluto / FFT channels (may have no processor if ctx==null)
	// MathProcessor settings are also here so the formula editor is always accessible.
	SimInstrument::CurveDescriptor curve2;
	curve2.name  = "Curve 2";
	curve2.color = QColor(0xff, 0x7e, 0x40);
	if(m_fftProc)
		curve2.processors << m_fftProc;
	curve2.processors << m_mathProc;

	m_ui->buildControlPanel(m_engine, {curve1, curve2});
}

void SimInstrumentController::stop()
{
	if(!m_engine)
		return;
	if(m_displayTimer)
		m_displayTimer->stop();
	disconnect(m_engine, &sim::AcquisitionEngine::cycleComplete,
		   this, &SimInstrumentController::onCycleComplete);
	QCoreApplication::removePostedEvents(this);
	m_engine->stop();
	m_dataDirty = false;
}

SimInstrument *SimInstrumentController::ui() const
{
	return m_ui;
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
	const QList<sim::DataKey> currentKeys = m_store->keys();
	if(currentKeys != m_lastKeys) {
		m_lastKeys = currentKeys;
		m_ui->updateCurveKeyCombos(currentKeys);
	}

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
		const sim::SampleBuffer xBuf = m_store->read(sim::DataKey(xKeyStr));
		if(xBuf.empty()) return;
		const auto &v = xBuf.sample(0);
		if(!std::holds_alternative<QVector<float>>(v)) return;
		xVec = std::get<QVector<float>>(v);
		if(xVec.isEmpty()) return;
	}

	if(!yIsIndex) {
		const sim::SampleBuffer yBuf = m_store->read(sim::DataKey(yKeyStr));
		if(yBuf.empty()) return;
		const auto &v = yBuf.sample(0);
		if(!std::holds_alternative<QVector<float>>(v)) return;
		yVec = std::get<QVector<float>>(v);
		if(yVec.isEmpty()) return;
	}

	if(!y2IsIndex) {
		const sim::SampleBuffer y2Buf = m_store->read(sim::DataKey(y2KeyStr));
		if(!y2Buf.empty()) {
			const auto &v = y2Buf.sample(0);
			if(std::holds_alternative<QVector<float>>(v))
				y2Vec = std::get<QVector<float>>(v);
		}
	}

	if(!x2IsIndex) {
		const sim::SampleBuffer x2Buf = m_store->read(sim::DataKey(x2KeyStr));
		if(!x2Buf.empty()) {
			const auto &v = x2Buf.sample(0);
			if(std::holds_alternative<QVector<float>>(v))
				x2Vec = std::get<QVector<float>>(v);
		}
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

	m_dataDirty = true;
}

} // namespace adc
} // namespace scopy
