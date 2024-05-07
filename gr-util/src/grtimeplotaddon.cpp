#include "grtimeplotaddon.h"

#include "grtimeplotaddonsettings.h"

#include <QLoggingCategory>
#include <QTimer>
#include <QwtWeedingCurveFitter>
#include <plotnavigator.hpp>

#include <grdeviceaddon.h>
#include <grlog.h>
#include <grtimechanneladdon.h>
#include <grtopblock.h>
#include <gui/buffer_previewer.hpp>
#include <gui/plotwidget.h>
#include <plotinfo.h>
#include <pluginbase/preferences.h>
#include <stylehelper.h>
#include <grtimechanneladdon.h>

Q_LOGGING_CATEGORY(CAT_GRTIMEPLOTADDON, "GRTimePlotAddon");

//#define GUI_THREAD_SAMPLING

using namespace scopy;
using namespace scopy::grutil;
GRTimePlotAddon::GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent)
	: QObject(parent)
	, m_top(top)
	, time_sink(nullptr)
	, m_rollingMode(false)
	, m_started(false)
	, m_singleShot(false)
	, m_showPlotTags(false)
	, m_refreshTimerRunning(false)
	, fftComplexMode(false)
	, m_xmode(GRTimePlotAddonSettings::XMODE_SAMPLES)
	, m_fftwindow(gr::fft::window::WIN_HAMMING)
{
	Preferences *p = Preferences::GetInstance();

	this->name = name;
	widget = new QTabWidget();
	m_lay = new QVBoxLayout(widget);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_plotWidget = new PlotWidget(widget);
	dynamic_cast<QTabWidget *>(widget)->addTab(m_plotWidget, "Time");

	/////
	m_fftPlotWidget = new PlotWidget(widget);
	m_fftPlotWidget->xAxis()->setVisible(false);
	m_fftPlotWidget->yAxis()->setVisible(false);
	dynamic_cast<QTabWidget *>(widget)->addTab(m_fftPlotWidget, "FFT");
	QPen fftchannel_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	fft_xPlotAxis = new PlotAxis(QwtAxis::XBottom, m_fftPlotWidget, fftchannel_pen);
	fft_yPlotAxis = new PlotAxis(QwtAxis::YLeft, m_fftPlotWidget, fftchannel_pen);
	fft_yPlotAxis->setInterval(-145, 5);

	m_fft_channel = new PlotChannel("FFT", fftchannel_pen, fft_xPlotAxis, fft_yPlotAxis);
	m_fftPlotWidget->addPlotChannel(m_fft_channel);
	//	m_fft_channel->setThickness(5);
	//	m_fft_channel->setStyle(1);

	m_fft_channel->setEnabled(true);
	m_fftPlotWidget->selectChannel(m_fft_channel);
	m_fftPlotWidget->replot();

	m_fftPlotWidget->setShowXAxisLabels(true);
	m_fftPlotWidget->setShowYAxisLabels(true);
	m_fftPlotWidget->showAxisLabels();

	////
	m_xyPlotWidget = new PlotWidget(widget);
	dynamic_cast<QTabWidget *>(widget)->addTab(m_xyPlotWidget, "X-Y");
	QPen xychannel_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	xy_xPlotAxis = new PlotAxis(QwtAxis::XBottom, m_xyPlotWidget, xychannel_pen);
	xy_yPlotAxis = new PlotAxis(QwtAxis::YLeft, m_xyPlotWidget, xychannel_pen);

	m_xy_channel = new PlotChannel("X-Y", xychannel_pen, xy_xPlotAxis, xy_yPlotAxis, this);
	m_xyPlotWidget->addPlotChannel(m_xy_channel);
	//	m_xy_channel->setHandle(new PlotAxisHandle(xychannel_pen, xy_yPlotAxis, m_xyPlotWidget, QwtAxis::YLeft,
	// this)); 	m_xyPlotWidget->addPlotAxisHandle(m_xy_channel->handle());

	//	m_xy_channel->setThickness(5);
	//	m_xy_channel->setStyle(1);

	m_xy_channel->setEnabled(true);
	m_xyPlotWidget->selectChannel(m_xy_channel);
	m_xyPlotWidget->replot();

	m_xyPlotWidget->setShowXAxisLabels(true);
	m_xyPlotWidget->setShowYAxisLabels(true);
	m_xyPlotWidget->showAxisLabels();

	//	widget->setLayout(m_lay);

	m_plotWidget->xAxis()->setInterval(0, 1);
	m_plotWidget->xAxis()->setVisible(true);
	//	m_plotWidget->topHandlesArea()->setVisible(true);

	QWidget *plotInfoSlot = createPlotInfoSlot(m_plotWidget);
	m_plotWidget->addPlotInfoSlot(plotInfoSlot);

	connect(m_plotWidget->navigator(), &PlotNavigator::rectChanged, this, [=]() {
		m_info->update(m_currentSamplingInfo);
		m_bufferPreviewer->updateDataLimits();
	});

	//	m_lay->addWidget(m_plotWidget);
	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &GRTimePlotAddon::replot);
	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	fw = new QFutureWatcher<void>(this);
	futureWatcherConn = connect(
		fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			drawPlot();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);

	connect(this, &GRTimePlotAddon::newData, m_plotWidget, &PlotWidget::newData);
}

GRTimePlotAddon::~GRTimePlotAddon() {}

QString GRTimePlotAddon::getName() { return name; }

QWidget *GRTimePlotAddon::getWidget() { return widget; }

PlotWidget *GRTimePlotAddon::plot() { return m_plotWidget; }
PlotWidget *GRTimePlotAddon::fftplot() { return m_fftPlotWidget; }

PlotChannel *GRTimePlotAddon::fftplotch() { return m_fft_channel; }

PlotWidget *GRTimePlotAddon::xyplot() { return m_xyPlotWidget; }

PlotChannel *GRTimePlotAddon::xyplotch() { return m_xy_channel; }

void GRTimePlotAddon::enable() {}

void GRTimePlotAddon::disable() {}

void GRTimePlotAddon::stopPlotRefresh()
{
	qInfo(CAT_GRTIMEPLOTADDON) << "Stopped plotting";
	m_refreshTimerRunning = false;
#ifdef GUI_THREAD_SAMPLING
#else
	refillFuture.cancel();
//	disconnect(futureWatcherConn);
#endif
	m_plotTimer->stop();
}

void GRTimePlotAddon::startPlotRefresh()
{
	qInfo(CAT_GRTIMEPLOTADDON) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;

#ifdef GUI_THREAD_SAMPLING
#else

#endif
	drawPlot();
	m_plotTimer->start();
}

void GRTimePlotAddon::drawTags()
{

	for(GRTimeChannelAddon *gr : qAsConst(grChannels)) {
		gr->plotCh()->clearMarkers();
	}

	if(!m_showPlotTags)
		return;

	for(GRTimeChannelAddon *gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {

			int index = time_channel_map.value(gr->signalPath()->name(), -1);
			if(index == -1)
				continue;

			for(int j = 0; j < time_sink->tags()[index].size(); j++) {
				PlotTag_t tag = time_sink->tags()[index][j];
				auto *m = gr->plotCh()->buildMarker(tag.str, QwtSymbol::Diamond, tag.offset,
								    time_sink->data()[index][tag.offset]);
				gr->plotCh()->addMarker(m);
			}
		}
	}
}

QWidget *GRTimePlotAddon::createPlotInfoSlot(QWidget *parent)
{
	QWidget *plotInfoSlot = new QWidget(parent);
	QVBoxLayout *plotInfoLayout = new QVBoxLayout(plotInfoSlot);
	plotInfoLayout->setSpacing(2);
	plotInfoLayout->setMargin(4);
	plotInfoSlot->setLayout(plotInfoLayout);

	AnalogBufferPreviewer *bufferPreviewer = new AnalogBufferPreviewer(plotInfoSlot);
	m_bufferPreviewer = new PlotBufferPreviewer(m_plotWidget, bufferPreviewer, plotInfoSlot);
	m_info = new TimePlotInfo(m_plotWidget, plotInfoSlot);
	plotInfoLayout->addWidget(m_bufferPreviewer);
	plotInfoLayout->addWidget(m_info);
	return plotInfoSlot;
}

void GRTimePlotAddon::drawPlot()
{
	//	qInfo(CAT_GRTIMEPLOT)<<"Draw plot";
	if(!time_sink)
		return;
	setRawSamplesPtr();
	drawTags();
	plot()->replot();
	m_xyPlotWidget->replot();
	m_fftPlotWidget->replot();
	if(time_sink->finishedAcquisition())
		Q_EMIT requestStop();
}

QList<GRTimeChannelAddon *> GRTimePlotAddon::getGrChannels() const { return grChannels; }

void GRTimePlotAddon::onStart()
{
	if(!m_started) {
		QElapsedTimer tim;
		tim.start();
		connect(this, &GRTimePlotAddon::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
		connect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
		connect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));

		m_top->build();
		m_top->start();
		m_started = true;

		m_info->updateStatus("running");
	}
}

void GRTimePlotAddon::onStop()
{
	if(m_started) {
		drawPlot();
		m_top->stop();
		m_top->teardown();
		disconnect(this, &GRTimePlotAddon::requestRebuild, m_top, &GRTopBlock::rebuild);
		disconnect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
		disconnect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
		m_started = false;

		m_info->updateStatus("stopped");
	}
}

void GRTimePlotAddon::setRawSamplesPtr()
{
	int xy_xindex = -1;
	int xy_yindex = -1;
	int fft_iindex = -1;
	int fft_qindex = -1;

	for(GRTimeChannelAddon *gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {

			int index = time_channel_map.value(gr->signalPath()->name(), -1);
			if(index != -1) {

				gr->plotCh()->curve()->setRawSamples(time_sink->time().data(),
								     time_sink->data()[index].data(),
								     time_sink->data()[index].size());

				gr->onNewData(time_sink->time().data(), time_sink->data()[index].data(),
					      time_sink->data()[index].size());

				if(gr == m_xy_source[0])
					xy_xindex = index;
				if(gr == m_xy_source[1])
					xy_yindex = index;
				if(gr == m_fft_source[0])
					fft_iindex = index;
				if(gr == m_fft_source[1])
					fft_qindex = index;

			} else {
				//			gr->plotCh()->curve()->setRawSamples(
				//				{}); // assign no data curve
			}
		}
	}
	if(xy_xindex != -1 && xy_yindex != -1) {
		m_xy_channel->curve()->setRawSamples(time_sink->data()[xy_xindex].data(),
						     time_sink->data()[xy_yindex].data(),
						     time_sink->data()[xy_yindex].size());

	} else {
		//		m_xy_channel->curve()->setSamples(0,0,0);
	}
	if(vector_sink->data().size() > m_currentSamplingInfo.bufferSize) {
		m_fft_channel->curve()->setSamples(time_sink->freq().data(), vector_sink->data().data(),
						   m_currentSamplingInfo.bufferSize);
		vector_sink->reset();
	}
	Q_EMIT newData();
}

void GRTimePlotAddon::replot()
{
	if(!time_sink)
		return;
#ifdef GUI_THREAD_SAMPLING
	time_sink->updateData();
	drawPlot();
	if(m_refreshTimerRunning)
		m_plotTimer->start();
#else
	refillFuture = QtConcurrent::run([=]() {
		//		qInfo(CAT_GRTIMEPLOT)<<"UpdateData";
		std::unique_lock lock(refillMutex);

		time_sink->updateData();
	});
	fw->setFuture(refillFuture);
#endif
}

void GRTimePlotAddon::updateBufferPreviewer() { m_bufferPreviewer->updateBufferPreviewer(); }

void GRTimePlotAddon::onInit()
{
	qDebug(CAT_GRTIMEPLOTADDON) << "Init";
	m_currentSamplingInfo.sampleRate = 1;
	m_currentSamplingInfo.bufferSize = 32;
	m_currentSamplingInfo.plotSize = 32;
	updateBufferPreviewer();
	//	 m_top->build();
}

void GRTimePlotAddon::onDeinit()
{
	qDebug(CAT_GRTIMEPLOTADDON) << "Deinit";
	onStop();
}

void GRTimePlotAddon::preFlowStart() {}

void GRTimePlotAddon::postFlowStart() { startPlotRefresh(); }

void GRTimePlotAddon::preFlowStop() { stopPlotRefresh(); }

void GRTimePlotAddon::postFlowStop() {}

void GRTimePlotAddon::onChannelAdded(ChannelAddon *t)
{
	auto ch = dynamic_cast<GRTimeChannelAddon *>(t);
	if(ch)
		grChannels.append(ch);
}

void GRTimePlotAddon::onChannelRemoved(ChannelAddon *t)
{

	if(plot()->selectedChannel() == t->plotCh())
		plot()->selectChannel(nullptr);
	auto ch = dynamic_cast<GRTimeChannelAddon *>(t);
	if(ch)
		grChannels.removeAll(ch);
}

void GRTimePlotAddon::connectSignalPaths()
{
	QList<GRSignalPath *> sigpaths;

	// for through grdevices - get sampleRate;
	std::unique_lock lock(refillMutex);
	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRTIMEPLOTADDON) << "Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRTIMEPLOTADDON) << "Appended " << sigpath->name();
	}

	time_sink = time_sink_f::make(m_currentSamplingInfo.plotSize, m_currentSamplingInfo.sampleRate,
				      name.toStdString(), sigpaths.count());
	time_sink->setRollingMode(m_rollingMode);
	time_sink->setSingleShot(m_singleShot);
	time_sink->setComputeTags(m_showPlotTags);

	time_sink->setFftComplex(fftComplexMode);
	time_sink->setFreqOffset(m_currentSamplingInfo.freqOffset);
	updateXAxis();

	auto fft_size = m_currentSamplingInfo.bufferSize;
	f2c = gr::blocks::float_to_complex::make();
	auto window = gr::fft::window::build(m_fftwindow, fft_size);

	vector_sink = gr::blocks::vector_sink_f::make(fft_size);
	s2v_complex = gr::blocks::stream_to_vector::make(sizeof(gr_complex), fft_size);
	fft_complex = gr::fft::fft_v<gr_complex, true>::make(fft_size, window, fftComplexMode);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);

	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / (fft_size * fft_size), fft_size);

	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);

	m_top->connect(f2c, 0, s2v_complex, 0);
	m_top->connect(s2v_complex, 0, fft_complex, 0);
	m_top->connect(fft_complex, 0, ctm, 0);
	m_top->connect(ctm, 0, mult_const1, 0);
	m_top->connect(mult_const1, 0, nlog10, 0);
	m_top->connect(nlog10, 0, vector_sink, 0);

	int i = 0;
	time_channel_map.clear();
	for(GRTimeChannelAddon *gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			m_top->connect(gr->signalPath()->getGrEndPoint(), 0, time_sink, i);
			time_channel_map.insert(gr->signalPath()->name(), i);

			if(!fftComplexMode) {
				if(gr == m_fft_source[0]) {
					m_top->connect(gr->signalPath()->getGrEndPoint(), 0, f2c, 0);
					m_top->connect(gr->signalPath()->getGrEndPoint(), 0, f2c, 1);
				}
			} else {
				if(gr == m_fft_source[0]) {
					m_top->connect(gr->signalPath()->getGrEndPoint(), 0, f2c, 0);
				}
				if(gr == m_fft_source[1]) {
					m_top->connect(gr->signalPath()->getGrEndPoint(), 0, f2c, 1);
				}
			}

			if(m_currentSamplingInfo.plotSize >= 1000000) {
				gr->plotCh()->curve()->setPaintAttribute(QwtPlotCurve::ClipPolygons);
				gr->plotCh()->curve()->setPaintAttribute(QwtPlotCurve::ImageBuffer);
				gr->plotCh()->curve()->setPaintAttribute(QwtPlotCurve::FilterPoints);
				gr->plotCh()->curve()->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive);
				auto curveFitter = new QwtWeedingCurveFitter(100000000);
				curveFitter->setChunkSize(100000000);
				gr->plotCh()->curve()->setCurveFitter(curveFitter);
			}
			i++;
		}
	}
}

void GRTimePlotAddon::tearDownSignalPaths() {}

void GRTimePlotAddon::onNewData()
{
	float sum0 = 0;
	float sum1 = 0;
}

void GRTimePlotAddon::updateXAxis()
{
	auto x = plot()->xAxis();
	if(m_rollingMode) {
		// not normal mode - rolling mode
		x->setMin(time_sink->time().back());
		x->setMax(time_sink->time().front());
	} else {
		// normal mode
		x->setMin(time_sink->time().front());
		x->setMax(time_sink->time().back());
	}

	if(fftComplexMode) {
		fft_xPlotAxis->setMin(time_sink->freqOffset() - m_currentSamplingInfo.sampleRate / 2);
		fft_xPlotAxis->setMax(time_sink->freqOffset() + m_currentSamplingInfo.sampleRate / 2);
	} else {
		fft_xPlotAxis->setMin(time_sink->freqOffset());
		fft_xPlotAxis->setMax(time_sink->freqOffset() + m_currentSamplingInfo.sampleRate / 2);
	}

	qInfo() << fft_xPlotAxis->min() << fft_xPlotAxis->max();

	m_info->update(m_currentSamplingInfo);
	m_bufferPreviewer->updateDataLimits();
	Q_EMIT xAxisUpdated();
}

void GRTimePlotAddon::setRollingMode(bool b)
{
	m_rollingMode = b;
	if(time_sink) {
		time_sink->setRollingMode(b);
		updateXAxis();
	}
}

void GRTimePlotAddon::setDrawPlotTags(bool b)
{
	m_showPlotTags = b;
	if(time_sink) {
		time_sink->setComputeTags(m_showPlotTags);
	}
	drawPlot();
}

double GRTimePlotAddon::sampleRate() { return m_currentSamplingInfo.sampleRate; }

void GRTimePlotAddon::setSampleRate(double val)
{
	m_currentSamplingInfo.sampleRate = val;
	Q_EMIT requestRebuild();
}

double GRTimePlotAddon::freqOffset() { return m_currentSamplingInfo.freqOffset; }

void GRTimePlotAddon::setFreqOffset(double val)
{
	m_currentSamplingInfo.freqOffset = val;
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setBufferSize(uint32_t size)
{
	m_currentSamplingInfo.bufferSize = size;
	//	std::unique_lock lock(refillMutex);
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setComplexMode(bool b)
{
	fftComplexMode = b;
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setPlotSize(uint32_t size)
{
	m_currentSamplingInfo.plotSize = size;
	//	std::unique_lock lock(refillMutex);
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::handlePreferences(QString key, QVariant v)
{
	if(key == "general_plot_target_fps") {
		updateFrameRate();
	}
}

void GRTimePlotAddon::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRTimePlotAddon::updateFrameRate()
{
	Preferences *p = Preferences::GetInstance();
	double framerate = p->get("general_plot_target_fps").toDouble();
	setFrameRate(framerate);
}

void GRTimePlotAddon::setFrameRate(double val)
{
	int timeout = (1.0 / val) * 1000;
	m_plotTimer->setInterval(timeout);
}

void GRTimePlotAddon::setXMode(int mode) { m_xmode = mode; }

void GRTimePlotAddon::setXYSource(GRTimeChannelAddon *x, GRTimeChannelAddon *y)
{
	disconnect(xy_min_max_connections[0]);
	disconnect(xy_min_max_connections[1]);
	disconnect(xy_min_max_connections[2]);
	disconnect(xy_min_max_connections[3]);
	m_xy_source[0] = x;
	m_xy_source[1] = y;
	xy_min_max_connections[0] =
		connect(x->plotCh()->yAxis(), &PlotAxis::minChanged, xy_xPlotAxis, &PlotAxis::setMin);
	xy_min_max_connections[1] =
		connect(x->plotCh()->yAxis(), &PlotAxis::maxChanged, xy_xPlotAxis, &PlotAxis::setMax);

	xy_min_max_connections[2] =
		connect(y->plotCh()->yAxis(), &PlotAxis::minChanged, xy_yPlotAxis, &PlotAxis::setMin);
	xy_min_max_connections[3] =
		connect(y->plotCh()->yAxis(), &PlotAxis::maxChanged, xy_yPlotAxis, &PlotAxis::setMax);
}

void GRTimePlotAddon::setFFTSource(GRTimeChannelAddon *i, GRTimeChannelAddon *q)
{
	m_fft_source[0] = i;
	m_fft_source[1] = q;
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setFftWindow(int idx)
{
	m_fftwindow = static_cast<gr::fft::window::win_type>(idx);
	Q_EMIT requestRebuild();
}

int GRTimePlotAddon::xMode() { return m_xmode; }

#include "moc_grtimeplotaddon.cpp"
