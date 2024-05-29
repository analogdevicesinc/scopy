#include <timeplotmanager.h>
#include <timeplotcomponentchannel.h>
#include <plotaxis.h>

using namespace scopy;
using namespace scopy::adc;

TimePlotManager::TimePlotManager(QString name, QWidget *parent)
	: QWidget(parent)
	, MetaComponent()
	, m_plotIdx(0)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_measurePanel = new MeasurementsPanel(this);
	m_measurePanel->setFixedHeight(100);
	//tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(80);
	//tool->bottomStack()->add(statsMenuId, m_statsPanel);
	m_lay->addWidget(m_measurePanel);
	m_measurePanel->setVisible(false);
	m_statsPanel->setVisible(false);
	m_lay->addWidget(m_statsPanel);
}

TimePlotManager::~TimePlotManager()
{

}

void TimePlotManager::enableMeasurementPanel(bool b)
{
	m_measurePanel->setVisible(b);
}

void TimePlotManager::enableStatsPanel(bool b)
{
	m_statsPanel->setVisible(b);
}

void TimePlotManager::setXInterval(double xMin, double xMax)
{
	for(auto plt : qAsConst(m_plots)) {
		plt->timePlot()->xAxis()->setInterval(xMin, xMax);
	}
}

void TimePlotManager::selectChannel(ChannelComponent *c)
{
	for(TimePlotComponentChannel *pcc : m_channels) {
		if(pcc->m_ch == c) {
			PlotChannel *ch = pcc->m_timePlotCh;
			PlotWidget* w = pcc->m_plotComponent->timePlot();
			w->selectChannel(ch);
		}
	}
}

MeasurementsPanel *TimePlotManager::measurePanel() const
{
	return m_measurePanel;
}

StatsPanel *TimePlotManager::statsPanel() const
{
	return m_statsPanel;
}

uint32_t TimePlotManager::addPlot(QString name)
{
	TimePlotComponent *plt = new TimePlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);
	addComponent(plt);
	m_lay->insertWidget(1, plt);
	return plt->uuid();
}

void TimePlotManager::removePlot(uint32_t uuid)
{	TimePlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);
}

void TimePlotManager::addChannel(ChannelComponent *c, uint32_t uuid)
{
	m_channels.append(c->plotChannelCmpt());
	TimePlotComponent *plt = plot(uuid);
	plt->addChannel(c);
}


void TimePlotManager::removeChannel(ChannelComponent *c)
{
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
}

void TimePlotManager::moveChannel(ChannelComponent *c, uint32_t uuid) {
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	TimePlotComponent *plt = plot(uuid);
	c->plotChannelCmpt()->initPlotComponent(plt);
	plt->addChannel(c);
}

TimePlotComponent *TimePlotManager::plot(uint32_t uuid)
{
	TimePlotComponent *plt = nullptr;
	for(TimePlotComponent *p : qAsConst(m_plots)) {
		if(p->uuid() == uuid) {
			plt = p;
		}
	}
	return plt;
}

QList<TimePlotComponent *> TimePlotManager::plots() const
{
	return m_plots;
}

void TimePlotManager::replot()
{
	for(TimePlotComponent *p : m_plots) {
		p->replot();
	}
}
