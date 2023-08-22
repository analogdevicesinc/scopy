#include "grtimeplotaddon.h"
#include <grlog.h>
#include <grtimechanneladdon.h>
#include <grdeviceaddon.h>
#include <grtopblock.h>
#include <grlog.h>
#include <gr-gui/scope_sink_f.h>
#include <gui/plotwidget.h>
#include <gui/buffer_previewer.hpp>
#include <QTimer>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>
#include <QwtWeedingCurveFitter>

Q_LOGGING_CATEGORY(CAT_GRTIMEPLOT, "GRTimePlotAddon");

//#define GUI_THREAD_SAMPLING

using namespace scopy;
using namespace scopy::grutil;
GRTimePlotAddon::GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent)
    : QObject(parent),
      m_top(top),
      time_sink(nullptr),
      m_rollingMode(false),
      m_started(false),
      m_singleShot(false),
      m_showPlotTags(false),
      m_refreshTimerRunning(false),
      m_xmode(GRTimePlotAddonSettings::XMODE_SAMPLES)
{

	Preferences *p = Preferences::GetInstance();

	this->name = name;
	widget = new QWidget();
	m_lay = new QVBoxLayout(widget);
	m_plotWidget = new PlotWidget(widget);
	widget->setLayout(m_lay);

	m_plotWidget->xAxis()->setInterval(0,1);
	m_plotWidget->leftHandlesArea()->setVisible(true);
	m_plotWidget->rightHandlesArea()->setVisible(true);
	m_plotWidget->bottomHandlesArea()->setVisible(true);
	m_plotWidget->xAxis()->setVisible(true);
//	m_plotWidget->topHandlesArea()->setVisible(true);

	setupBufferPreviewer();
	m_cursors = new PlotCursors(m_plotWidget);
	m_cursors->setVisible(false);
	m_lay->addWidget(m_plotWidget);
	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &GRTimePlotAddon::replot);
	connect(p, SIGNAL(preferenceChanged(QString,QVariant)), this, SLOT(handlePreferences(QString,QVariant)));

	fw = new QFutureWatcher<void>(this);
	futureWatcherConn = connect(fw, &QFutureWatcher<void>::finished, this, [=](){
			drawPlot();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		}, Qt::QueuedConnection);
}

void GRTimePlotAddon::setupBufferPreviewer() {
	m_bufferPreviewer = new AnalogBufferPreviewer(widget);
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setCursorPos(0.5);
	m_bufferPreviewer->setHighlightPos(0.05);
	m_bufferPreviewer->setHighlightWidth(0.2);
	m_bufferPreviewer->setCursorVisible(false);
	m_bufferPreviewer->setWaveformPos(0.1);
	m_bufferPreviewer->setWaveformWidth(0.5);

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, this, [=]() {
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStartDrag, this, [=]() {
		m_bufferPrevInitMin = plot()->xAxis()->min();
		m_bufferPrevInitMax = plot()->xAxis()->max();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int value) {
		qInfo()<<value;
		double moveTo = 0.0;

		int width = m_bufferPreviewer->width();
		double xAxisWidth = m_plotSize;

		moveTo = value * xAxisWidth / width;
		plot()->xAxis()->setInterval(m_bufferPrevInitMin + moveTo,  m_bufferPrevInitMax + moveTo);
		plot()->replot();

		updateBufferPreviewer();
	} );

	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, this, [=]() {
		plot()->xAxis()->setInterval(0,  m_plotSize);
		plot()->replot();

		updateBufferPreviewer();
	} );

	plot()->addBufferPreviewerSlot(m_bufferPreviewer);
//	m_lay->addWidget(m_bufferPreviewer);
	m_bufferPreviewer->setVisible(Preferences::get("adc_plot_show_buffer_previewer").toBool());
}

void GRTimePlotAddon::updateBufferPreviewer() {
	// Time interval within the plot canvas
	QwtInterval plotInterval(plot()->xAxis()->min(), plot()->xAxis()->max());

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, m_plotSize);

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
			      fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
			      fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();


	m_bufferPreviewer->setWaveformWidth(wWidth);
	m_bufferPreviewer->setWaveformPos(wPos);
	m_bufferPreviewer->setHighlightWidth(hWidth);
	m_bufferPreviewer->setHighlightPos(hPos);
}

void GRTimePlotAddon::showCursors(bool b)
{
	m_cursors->setVisible(b);
}

GRTimePlotAddon::~GRTimePlotAddon() {
}

QString GRTimePlotAddon::getName() { return name; }

QWidget *GRTimePlotAddon::getWidget() { return widget; }

PlotWidget *GRTimePlotAddon::plot() { return m_plotWidget;}

void GRTimePlotAddon::enable() {}

void GRTimePlotAddon::disable() {}

void GRTimePlotAddon::stopPlotRefresh() {
	qInfo(CAT_GRTIMEPLOT)<<"Stopped plotting";
	m_refreshTimerRunning = false;
#ifdef GUI_THREAD_SAMPLING
#else
	refillFuture.cancel();
//	disconnect(futureWatcherConn);
#endif	
	m_plotTimer->stop();
}


void GRTimePlotAddon::startPlotRefresh() {
	qInfo(CAT_GRTIMEPLOT)<<"Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;

#ifdef GUI_THREAD_SAMPLING
#else

#endif
	drawPlot();
	m_plotTimer->start();
}

void GRTimePlotAddon::setShowPlotTags() {
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			gr->plotCh()->clearMarkers();
		}
	}

	if(!m_showPlotTags)
		return;

	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {

			int index = time_channel_map.value(gr->signalPath()->name(),-1);
			if(index == -1)
				continue;

			for(int j = 0; j < time_sink->tags()[index].size();j++) {
				PlotTag_t tag = time_sink->tags()[index][j];
				auto *m = gr->plotCh()->buildMarker(tag.str, QwtSymbol::Diamond, tag.offset, time_sink->data()[index][tag.offset]);
				gr->plotCh()->addMarker(m);
			}

		}
	}

}

void GRTimePlotAddon::drawPlot() {
//	qInfo(CAT_GRTIMEPLOT)<<"Draw plot";
	setRawSamplesPtr();
	setShowPlotTags();
	plot()->replot();
	if(time_sink->finishedAcquisition())
		Q_EMIT requestStop();
}

void GRTimePlotAddon::onStart() {
	if(!m_started) {
		QElapsedTimer tim;
		tim.start();
		connect(this, &GRTimePlotAddon::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
		connect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
		connect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));

		m_top->build();
		m_top->start();
		m_started = true;
	}

}

 void GRTimePlotAddon::onStop() {
	 if(m_started) {
		drawPlot();
		m_top->stop();
		m_top->teardown();
		disconnect(this, &GRTimePlotAddon::requestRebuild, m_top, &GRTopBlock::rebuild);
		disconnect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
		disconnect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
		m_started = false;
	}
}

 void GRTimePlotAddon::setRawSamplesPtr() {
	 for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		 if(gr->signalPath()->enabled()) {

			int index = time_channel_map.value(gr->signalPath()->name(),-1);
			if(index != -1) {

			gr->plotCh()->curve()->setRawSamples(
				time_sink->time().data(),
				time_sink->data()[index].data(),
				time_sink->data()[index].size());

			gr->onNewData(time_sink->time().data(),
				time_sink->data()[index].data(),
				time_sink->data()[index].size());

			} else {
			gr->plotCh()->curve()->setRawSamples(
				time_sink->time().data(), 0, 0); // assign no data curve
			}
		 }
	 }
 }

 void GRTimePlotAddon::replot() {
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

 void GRTimePlotAddon::onInit() {
	 qDebug(CAT_GRTIMEPLOT)<<"Init";
	 m_sampleRate = 1;
	 m_bufferSize = 32;
	 m_plotSize = 32;
	 updateBufferPreviewer();
//	 m_top->build();
 }

 void GRTimePlotAddon::onDeinit() {
	 qDebug(CAT_GRTIMEPLOT)<<"Deinit";
	 onStop();
 }

 void GRTimePlotAddon::preFlowStart() {

 }

 void GRTimePlotAddon::postFlowStart() {
	 startPlotRefresh();
 }

 void GRTimePlotAddon::preFlowStop() {
	 stopPlotRefresh();
 }

 void GRTimePlotAddon::postFlowStop() {

 }

void GRTimePlotAddon::onChannelAdded(ToolAddon *t) {
	auto ch = dynamic_cast<GRTimeChannelAddon*> (t);
	if(ch)
		grChannels.append(ch);
}

void GRTimePlotAddon::onChannelRemoved(ToolAddon *t) {
	auto ch = dynamic_cast<GRTimeChannelAddon*> (t);
	if(ch)
		grChannels.removeAll(ch);
}

void GRTimePlotAddon::connectSignalPaths() {
	QList<GRSignalPath*> sigpaths;

	// for through grdevices - get sampleRate;
	std::unique_lock lock(refillMutex);
	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRTIMEPLOT)<<"Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRTIMEPLOT)<<"Appended " << sigpath->name();

	}

	time_sink = time_sink_f::make(m_plotSize, m_sampleRate, name.toStdString(), sigpaths.count());
	time_sink->setRollingMode(m_rollingMode);
	time_sink->setSingleShot(m_singleShot);
	updateXAxis();

	int i=0;

	time_channel_map.clear();
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			m_top->connect(gr->signalPath()->getGrEndPoint(), 0, time_sink, i);
			time_channel_map.insert(gr->signalPath()->name(),i);
			if(m_plotSize >= 1000000) {
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

void GRTimePlotAddon::tearDownSignalPaths() {
}

void GRTimePlotAddon::onNewData() {
	float sum0 = 0;
	float sum1 = 0;	
}

void GRTimePlotAddon::updateXAxis() {
	if(m_rollingMode) {
		// not normal mode - rolling mode
		plot()->xAxis()->setMin(time_sink->time().back());
		plot()->xAxis()->setMax(time_sink->time().front());
	} else {
		// normal mode
		plot()->xAxis()->setMin(time_sink->time().front());
		plot()->xAxis()->setMax(time_sink->time().back());
	}
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
	drawPlot();
}

double GRTimePlotAddon::sampleRate() {
	return m_sampleRate;
}

void GRTimePlotAddon::setSampleRate(double val) {
	m_sampleRate = val;
}

void GRTimePlotAddon::setBufferSize(uint32_t size)
{
	m_bufferSize = size;
//	std::unique_lock lock(refillMutex);
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setPlotSize(uint32_t size)
{
	m_plotSize = size;
//	std::unique_lock lock(refillMutex);
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::handlePreferences(QString key, QVariant v)
{
	if(key == "general_plot_target_fps") {
		updateFrameRate();
	} else if(key == "adc_plot_show_buffer_previewer") {
		m_bufferPreviewer->setVisible(v.toBool());
	}
}

void GRTimePlotAddon::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRTimePlotAddon::updateFrameRate() {
	Preferences *p = Preferences::GetInstance();
	double framerate = p->get("general_plot_target_fps").toDouble();
	setFrameRate(framerate);

}

void GRTimePlotAddon::setFrameRate(double val) {
	int timeout = (1.0/val)*1000;
	m_plotTimer->setInterval(timeout);
}

void GRTimePlotAddon::setXMode(int mode)
{
	m_xmode = mode;
}

int GRTimePlotAddon::xMode()
{
	return m_xmode;
}

