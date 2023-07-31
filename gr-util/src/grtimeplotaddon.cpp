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

Q_LOGGING_CATEGORY(CAT_GRTIMEPLOT, "GRTimePlotAddon");


using namespace scopy;
using namespace scopy::grutil;
GRTimePlotAddon::GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent)
	: QObject(parent),
	m_top(top),
	time_sink(nullptr),
	m_rollingMode(false){

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

//	setupBufferPreviewer();
	m_lay->addWidget(m_plotWidget);
	m_plotTimer = new QTimer(this);
	connect(m_plotTimer, &QTimer::timeout, this, &GRTimePlotAddon::replot);

}

void GRTimePlotAddon::setupBufferPreviewer() {
	AnalogBufferPreviewer* m_bufferPreviewer = new AnalogBufferPreviewer(widget);
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setCursorPos(0.5);
	m_bufferPreviewer->setHighlightPos(0.05);
	m_bufferPreviewer->setHighlightWidth(0.2);
	m_bufferPreviewer->setCursorVisible(false);
	m_bufferPreviewer->setWaveformPos(0.1);
	m_bufferPreviewer->setWaveformWidth(0.5);

//	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, this, [=]() {
//		horiz_offset = m_bufferPreviewer->highlightPos();
//	});
//	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int value) {
//		qInfo()<<value;
//		double moveTo = 0.0;
//		double min = xAxis()->min();
//		double max = xAxis()->max();
//		int width = m_bufferPreviewer->width();
//		double xA0xisWidth = max - min;

//		moveTo = value * xAxisWidth / width;
//		xAxis()->setInterval(min - moveTo, max - moveTo);
//		m_plot->replot();

//		auto delta = horiz_offset + (value/(float)width);

//		qInfo()<< delta << value << width;
//		m_bufferPreviewer->setHighlightPos(delta);


//		updateBufferPreviewer();
//	} );
	m_lay->addWidget(m_bufferPreviewer);
}

GRTimePlotAddon::~GRTimePlotAddon() {
	onStop();
}

QString GRTimePlotAddon::getName() { return name; }

QWidget *GRTimePlotAddon::getWidget() { return widget; }

PlotWidget *GRTimePlotAddon::plot() { return m_plotWidget;}

void GRTimePlotAddon::enable() {}

void GRTimePlotAddon::disable() {}

void GRTimePlotAddon::stopPlotRefresh() {
	qInfo(CAT_GRTIMEPLOT)<<"Stopped plotting";
	m_plotTimer->stop();
}

void GRTimePlotAddon::startPlotRefresh() {
	qInfo(CAT_GRTIMEPLOT)<<"Start plotting";
	replot();
	int timeout = (1.0/60.0)*1000;
	m_plotTimer->setInterval(timeout);
	m_plotTimer->start();
}

void GRTimePlotAddon::onStart() {
	QElapsedTimer tim;
	tim.start();
	connect(this, &GRTimePlotAddon::requestRebuild, m_top, &GRTopBlock::rebuild);
	connect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	connect(m_top, &GRTopBlock::aboutToStop, this, &GRTimePlotAddon::stopPlotRefresh);
	connect(m_top, &GRTopBlock::started, this, &GRTimePlotAddon::startPlotRefresh);

	m_top->build();
	m_top->start();

}

void GRTimePlotAddon::setRawSamplesPtr() {
	int i=0;
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			gr->plotCh()->curve()->setRawSamples(time_sink->time().data(), time_sink->data()[i].data(), time_sink->data()[i].size());
			i++;
		}
	}
}

void GRTimePlotAddon::replot() {
	time_sink->updateData();
	setRawSamplesPtr();
	plot()->replot();
}

 void GRTimePlotAddon::onStop() {
	m_top->stop();
	m_top->teardown();
	disconnect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	disconnect(m_top, &GRTopBlock::aboutToStop,this, &GRTimePlotAddon::stopPlotRefresh);
	disconnect(m_top, &GRTopBlock::started,this, &GRTimePlotAddon::startPlotRefresh);
}

 void GRTimePlotAddon::onInit() {
	qInfo()<<"INIT";
 }

 void GRTimePlotAddon::onDeinit() {}

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

	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRTIMEPLOT)<<"Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRTIMEPLOT)<<"Appended " << sigpath->name();

	}

	time_sink = time_sink_f::make(m_plotSize,1,name.toStdString(),sigpaths.count());
	time_sink->setRollingMode(m_rollingMode);
	updateXAxis();

	int i=0;
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			m_top->connect(gr->signalPath()->getGrEndPoint(), 0, time_sink, i);
			i++;
		}
	}
}

void GRTimePlotAddon::tearDownSignalPaths() {
	time_sink = nullptr;
}

void GRTimePlotAddon::onNewData() {
	float sum0 = 0;
	float sum1 = 0;	
}

void GRTimePlotAddon::updateXAxis() {
	if(m_rollingMode) {
		plot()->xAxis()->setMin(time_sink->time().front());
		plot()->xAxis()->setMax(time_sink->time().back());
	} else {
		plot()->xAxis()->setMin(time_sink->time().back());
		plot()->xAxis()->setMax(time_sink->time().front());
	}
}

void GRTimePlotAddon::setRollingMode(bool b)
{
	m_rollingMode = b;
	if(time_sink) {
		time_sink->setRollingMode(b);
		updateXAxis();
	}

}

void GRTimePlotAddon::setBufferSize(uint32_t size)
{
	m_bufferSize = size;
	Q_EMIT requestRebuild();
}

void GRTimePlotAddon::setPlotSize(uint32_t size)
{
	m_plotSize = size;
	Q_EMIT requestRebuild();
}
