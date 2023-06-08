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
	gridPlot->setContentsMargins(9, 0, 9, 0);
	m_plotWidget->setLayout(gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
						  QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(m_plot->topArea(), 1, 0, 1, 4);
	gridPlot->addWidget(m_plot->leftHandlesArea(), 1, 0, 4, 1);
	gridPlot->addWidget(m_plot, 3, 1, 1, 1);
	gridPlot->addItem(plotSpacer, 5, 0, 1, 4);

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

	m_top->build();
	m_top->start();
}

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
	QString sinkName = (name + ch->getDevice()->getName() +  t->getName());
	bool ret = m_plot->registerSink(sinkName.toStdString(),1,0);
	qInfo()<<"created plot_sinks "<<sinkName << ret;
}

void GRTimePlotAddon::onChannelRemoved(ToolAddon *) {}

void GRTimePlotAddon::connectSignalPaths() {
	for(auto &sigpath : m_top->signalPaths()) {
		qInfo()<<sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(name))
			continue;

		auto sink = scope_sink_f::make(1024,1000,sigpath->name().toStdString(),1,m_plot);
		sinks.append(sink);
		sink->set_update_time(0.1);
		sink->set_trigger_mode(TRIG_MODE_FREE,1,"");

		qInfo()<<"created scope_sink_f with name" << sigpath->name();
		m_top->connect(sigpath->getGrEndPoint(), 0, sink, 0);
		m_plot->setAllYAxis(-1000000,1000000);
	}
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
