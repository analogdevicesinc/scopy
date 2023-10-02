#include "grtimeplotaddon.h"
#include "grtimeplotaddonsettings.h"
#include "hoverwidget.h"
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
#include <plotinfo.h>

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
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	m_plotWidget = new PlotWidget(widget);
	widget->setLayout(m_lay);

	m_plotWidget->xAxis()->setInterval(0,1);
	m_plotWidget->leftHandlesArea()->setVisible(true);
	m_plotWidget->rightHandlesArea()->setVisible(true);
	m_plotWidget->bottomHandlesArea()->setVisible(true);
	m_plotWidget->xAxis()->setVisible(true);
//	m_plotWidget->topHandlesArea()->setVisible(true);

	m_info = new TimePlotInfo(m_plotWidget, widget);
	m_plotWidget->addPlotInfoSlot(m_info);

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

void GRTimePlotAddon::drawTags() {

	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		gr->plotCh()->clearMarkers();
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
	if(!time_sink)
		return;
	setRawSamplesPtr();
	drawTags();
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
//			gr->plotCh()->curve()->setRawSamples(
//				{}); // assign no data curve
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

 void GRTimePlotAddon::updateBufferPreviewer() {
	 m_info->updateBufferPreviewer();
 }

 void GRTimePlotAddon::onInit() {
	 qDebug(CAT_GRTIMEPLOT)<<"Init";
	 m_currentSamplingInfo.sampleRate = 1;
	 m_currentSamplingInfo.bufferSize = 32;
	 m_currentSamplingInfo.plotSize = 32;
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

	time_sink = time_sink_f::make(m_currentSamplingInfo.plotSize, m_currentSamplingInfo.sampleRate, name.toStdString(), sigpaths.count());
	time_sink->setRollingMode(m_rollingMode);
	time_sink->setSingleShot(m_singleShot);
	time_sink->setComputeTags(m_showPlotTags);
	updateXAxis();

	int i=0;

	time_channel_map.clear();
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			m_top->connect(gr->signalPath()->getGrEndPoint(), 0, time_sink, i);
			time_channel_map.insert(gr->signalPath()->name(),i);
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

void GRTimePlotAddon::tearDownSignalPaths() {
}

void GRTimePlotAddon::onNewData() {
	float sum0 = 0;
	float sum1 = 0;	
}

void GRTimePlotAddon::updateXAxis() {
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
	auto max = x->max();
	auto min = x->min();
	auto divs = x->divs();
	double hdiv = abs(max - min) / divs;

	m_info->update(m_currentSamplingInfo);
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

double GRTimePlotAddon::sampleRate() {
	return m_currentSamplingInfo.sampleRate;
}

void GRTimePlotAddon::setSampleRate(double val) {
	m_currentSamplingInfo.sampleRate = val;
}

void GRTimePlotAddon::setBufferSize(uint32_t size)
{
	m_currentSamplingInfo.bufferSize = size;
//	std::unique_lock lock(refillMutex);
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

