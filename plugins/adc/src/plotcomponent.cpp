#include "plotcomponent.h"
#include "channelcomponent.h"
#include "plotaxis.h"

using namespace scopy;
using namespace adc;

PlotComponent::PlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: QWidget(parent)
	, MetaComponent()
	, m_uuid(uuid)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_plotLayout = new QHBoxLayout(this);
	m_plotLayout->setMargin(0);
	m_plotLayout->setSpacing(0);
	setLayout(m_plotLayout);
	m_name = name;
}

PlotComponent::~PlotComponent() {}

QPair<double, double> PlotComponent::xInterval()
{
	double min = m_plots[0]->xAxis()->min();
	double max = m_plots[0]->xAxis()->max();
	return QPair<double,double>(min,max);
}

void PlotComponent::replot()
{
	for(auto plot : m_plots) {
		plot->replot();
	}
}

void PlotComponent::refreshAxisLabels() {
	for(auto plot : m_plots) {
		plot->showAxisLabels();
	}
}


void PlotComponent::showPlotLabels(bool b)
{
	for(auto plot : m_plots) {
		plot->setShowXAxisLabels(b);
		plot->setShowYAxisLabels(b);
		plot->showAxisLabels();
	}
}


void PlotComponent::setName(QString s)
{
	m_name = s;
	Q_EMIT nameChanged(s);
}


void PlotComponent::onStart() { MetaComponent::onStart(); }

void PlotComponent::onStop() { MetaComponent::onStop(); }

void PlotComponent::onInit() {}

void PlotComponent::onDeinit() {}

void PlotComponent::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
}

void PlotComponent::selectChannel(ChannelComponent *c) {
	for(auto plot : m_plots) {
		plot->selectChannel(c->plotChannelCmpt()->plotChannel());
	}
}

void PlotComponent::setXInterval(QPair<double,double> p) {
	setXInterval(p.first,p.second);
}

void PlotComponent::setXInterval(double min, double max)
{
	for(auto plot : m_plots) {
		plot->xAxis()->setInterval(min,max);
	}
}

void PlotComponent::removeChannel(ChannelComponent *c)
{
	PlotComponentChannel *toRemove;
	for(PlotComponentChannel *ch : qAsConst(m_channels)) {
		if(ch->channelComponent() == c) {
			toRemove = ch;
			break;
		}
	}
	m_channels.removeAll(toRemove);
}

uint32_t PlotComponent::uuid() { return m_uuid; }

CursorController *PlotComponent::cursor() const
{
	return m_cursor;
}

QList<PlotWidget *> PlotComponent::plots() const
{
	return m_plots;
}
