#include "grtimeplotaddon.h"
#include <grlog.h>
#include <grtimechanneladdon.h>
#include <grdeviceaddon.h>
#include <grtopblock.h>
#include <grlog.h>
#include <gr-gui/scope_sink_f.h>

using namespace scopy;
using namespace scopy::grutil;
GRTimePlotAddon::GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent) : QObject(parent), m_top(top){

	this->name = name;
	QWidget *m_plotWidget = new QWidget();

	m_plot = new CapturePlot(m_plotWidget,false,16,10, new TimePrefixFormatter, new MetricPrefixFormatter);
	m_plotWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot->disableLegend();


	QGridLayout *gridPlot = new QGridLayout(m_plotWidget);
	gridPlot->setVerticalSpacing(0);
	gridPlot->setHorizontalSpacing(0);
	gridPlot->setContentsMargins(0, 0, 0, 0);
	m_plotWidget->setLayout(gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
						  QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(m_plot->topArea(), 0, 0, 1, 4);
	gridPlot->addWidget(m_plot->leftHandlesArea(), 0, 0, 4, 1);
	gridPlot->addWidget(m_plot, 1, 1, 1, 1);
	gridPlot->addItem(plotSpacer, 2, 0, 1, 4);

	m_plot->setSampleRate(1000, 1, "Hz");
	m_plot->setActiveVertAxis(0);
	//	m_plot->setAxisVisible(QwtAxis::YLeft, false);
	//	m_plot->setAxisVisible(QwtAxis::XBottom, false);
	m_plot->setUsingLeftAxisScales(false);
	m_plot->enableTimeTrigger(false);
	m_plot->setActiveVertAxis(0, true);
	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, 0.1);
	m_plot->setOffsetInterval(-65535, 65535);
	//	m_plot->setAutoScale(true);
	connect(m_plot, SIGNAL(newData()),this, SLOT(onNewData()));
	m_plot->replot();

	widget = m_plotWidget;
}

GRTimePlotAddon::~GRTimePlotAddon() { }

QString GRTimePlotAddon::getName() { return name; }

QWidget *GRTimePlotAddon::getWidget() { return widget; }

CapturePlot *GRTimePlotAddon::plot() { return m_plot;}

void GRTimePlotAddon::enable() {}

void GRTimePlotAddon::disable() {}

void GRTimePlotAddon::onStart() {
	connect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));

	// PlotChannel Structure - { // Create Plot in TestPlugin, addPlotChannel, removePlotChannel
	// single axis vs multiaxis (?)
	// QwtPlot*
	// QwtCurve
	// QwtAxis
	// QwtZoomer (?)
	// float *data
	//
	// }


	// populate reference channels // when creating reference channel - do this
	// QwtCurve[].setSamples(X/Y)

	// connect sink stopped ?

	m_top->build();
	m_top->start();

	// connect timer to time sink to get data - or data driven - connect to sink_new data
	// start timer


}

/* void GRTimePlotAddon::getData() {
	data = sink.getData(); // copies data from sink to plot
	replot();
	// Q_EMIT replot ?

*/



    void GRTimePlotAddon::onStop() {
	m_top->stop();
	m_top->teardown();
	disconnect(m_top,SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top,SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
}

void GRTimePlotAddon::onAdd() {}

void GRTimePlotAddon::onRemove() {}

void GRTimePlotAddon::onChannelAdded(ToolAddon *t) {
	GRTimeChannelAddon *ch = dynamic_cast<GRTimeChannelAddon*>(t);
	QString sinkName = (name /*+ ch->getDevice()->getName() +  t->getName()*/);
	// create new PlotChannel object - this could also be created

}

void GRTimePlotAddon::onChannelRemoved(ToolAddon *) {
	// remove PlotChannel object

}

void GRTimePlotAddon::connectSignalPaths() {
	QList<GRSignalPath*> sigpaths;
	for(auto &sigpath : m_top->signalPaths()) {
		qInfo()<<sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;
		sigpaths.append(sigpath);


		qInfo()<<"created scope_sink_f with name" << sigpath->name();

	}



	// create and configure time_sink_f
	// allocate memory for data to be plotted - could be done by time_sink ?
	// 	for(int i=0;i<sigpaths.count();i++) {
	//	QwtCurve[].setRawSamples(data[i]) // just get pointer from time_sink
	// }

	auto sink = scope_sink_f::make(1024,1000,name.toStdString(),sigpaths.count(),m_plot);
	sinks.append(sink);
	sink->set_update_time(0.01);
	sink->set_trigger_mode(TRIG_MODE_FREE,1,"");
	m_plot->registerSink(name.toStdString(),sigpaths.count(),0);
	m_plot->setAllYAxis(-(1<<11),1<<11);

	int i=0;
	for(auto &sigpath : sigpaths) {
		m_top->connect(sigpath->getGrEndPoint(), 0, sink, i);
		i++;
	}

	for(int i=0;i<sigpaths.count();i++) {
		auto curveId = m_plot->getAnalogChannels() - 1;
		auto color = m_plot->getLineColor(curveId);
		//	btn->setColor(color);
		m_plot->Curve(curveId)->setAxes(
		    QwtAxisId(QwtAxis::XBottom, 0),
		    QwtAxisId(QwtAxis::YLeft, curveId));
		m_plot->DetachCurve(curveId);
		m_plot->AttachCurve(curveId);
		m_plot->addZoomer(curveId);


	}
	m_plot->setSampleRatelabelValue(1234);
	m_plot->setBufferSizeLabelValue(4321);
	m_plot->setTimeBaseLabelValue(12.5e-5);
	// TO DO: Give user the option to make these axes visible
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::YLeft, 0), false);
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::XBottom,0), false);
	//	m_plot->setUsingLeftAxisScales(false);


}

void GRTimePlotAddon::tearDownSignalPaths() {
	for(auto &sink : sinks) {
	}
	sinks.clear();

}

void GRTimePlotAddon::onNewData() {
	float sum0 = 0;
	float sum1 = 0;
	for(int i=0;i<m_plot->Curve(0)->data()->size();i++) {
		sum0 += m_plot->Curve(0)->data()->sample(i).y();
	}
	//	for(int i=0;i<m_plot->Curve(1)->data()->size();i++) {
	//		sum1 += m_plot->Curve(1)->data()->sample(i).y();
	//	}


	qInfo()<<"new dataa! avgs - "
		<< sum0/m_plot->Curve(0)->data()->size()
	    //		<< sum1 / m_plot->Curve(1)->data()->size()
	    ;
}
