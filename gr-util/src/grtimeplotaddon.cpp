#include "grtimeplotaddon.h"
#include <grlog.h>
#include <grtimechanneladdon.h>
#include <grdeviceaddon.h>
#include <grtopblock.h>
#include <grlog.h>
#include <gr-gui/scope_sink_f.h>
#include <gui/plotwidget.h>
#include <QTimer>

using namespace scopy;
using namespace scopy::grutil;
GRTimePlotAddon::GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent) : QObject(parent), m_top(top){

	this->name = name;
	m_plotWidget = new PlotWidget();
	m_plotWidget->xAxis()->setInterval(0,1);
	m_plotWidget->leftHandlesArea()->setVisible(true);
	m_plotWidget->bottomHandlesArea()->setVisible(true);
	widget = m_plotWidget;
	m_plotTimer = new QTimer(this);
	connect(m_plotTimer, &QTimer::timeout, this, &GRTimePlotAddon::replot);

}

GRTimePlotAddon::~GRTimePlotAddon() { }

QString GRTimePlotAddon::getName() { return name; }

QWidget *GRTimePlotAddon::getWidget() { return widget; }

PlotWidget *GRTimePlotAddon::plot() { return m_plotWidget;}

void GRTimePlotAddon::enable() {}

void GRTimePlotAddon::disable() {}

void GRTimePlotAddon::onStart() {
	QElapsedTimer tim;
	tim.start();
	connect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	// connect sink stopped ?
	m_top->build();
	m_top->start();
	m_plotTimer->setInterval(1/60.0);	
	m_plotTimer->start();

}

void GRTimePlotAddon::replot() {
	// copies data from sink to plot
	time_sink->updateData();
	plot()->replot();
}

 void GRTimePlotAddon::onStop() {
	m_plotTimer->stop();
	m_top->stop();
	m_top->teardown();
	disconnect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
}

void GRTimePlotAddon::onAdd() {}

void GRTimePlotAddon::onRemove() {}

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
		qInfo()<<"Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;
		sigpaths.append(sigpath);
		qInfo()<<"Appended " << sigpath->name();

	}



	time_sink = time_sink_f::make(1024,1000,name.toStdString(),sigpaths.count());
	// create and configure time_sink_f
	// allocate memory for data to be plotted - could be done by time_sink ?


	int i=0;
	/*	REGISTER SINK !*/
	for(GRTimeChannelAddon* gr : qAsConst(grChannels)) {
		if(gr->signalPath()->enabled()) {
			m_top->connect(gr->signalPath()->getGrEndPoint(), 0, time_sink, i);
			gr->plotCh()->curve()->setRawSamples(time_sink->time().data(), time_sink->data()[i].data(), 1024);
			i++;
		}
	}
}

void GRTimePlotAddon::tearDownSignalPaths() {
	/*for(auto &sink : sinks) {
	}
	sinks.clear();*/

}

void GRTimePlotAddon::onNewData() {
	float sum0 = 0;
	float sum1 = 0;	
}
