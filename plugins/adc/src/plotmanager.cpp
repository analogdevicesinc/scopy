#include "plotmanager.h"
#include "plotmanagercombobox.h"
#include "plotaxis.h"

using namespace scopy;
using namespace scopy::adc;

PlotManager::PlotManager(QString name, QWidget *parent)
	: QWidget(parent)
	, MetaComponent()
	, m_plotIdx(0)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_measurePanel = new MeasurementsPanel(this);
	m_measurePanel->setFixedHeight(110);
	// tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(100);
	// tool->bottomStack()->add(statsMenuId, m_statsPanel);
	m_lay->addWidget(m_measurePanel);
	m_measurePanel->setVisible(false);
	m_statsPanel->setVisible(false);
	m_lay->addWidget(m_statsPanel);
}

PlotManager::~PlotManager() {}

void PlotManager::enableMeasurementPanel(bool b) { m_measurePanel->setVisible(b); }

void PlotManager::enableStatsPanel(bool b) { m_statsPanel->setVisible(b); }

void PlotManager::setXInterval(double xMin, double xMax)
{
	m_xInterval.first = xMin;
	m_xInterval.second = xMax;
	for(auto plt : qAsConst(m_plots)) {
		plt->setXInterval(xMin, xMax);
	}
}

void PlotManager::setXUnit(QString s)
{
	for(PlotComponent *p : m_plots) {
		p->setXUnit(s);
	}


}

void PlotManager::selectChannel(ChannelComponent *c) {
	for(PlotComponentChannel *pcc : qAsConst(m_channels)) {
		if(pcc->channelComponent() == c) {
			pcc->plotComponent()->selectChannel(c);
		}
	}
}

MeasurementsPanel *PlotManager::measurePanel() const { return m_measurePanel; }

StatsPanel *PlotManager::statsPanel() const { return m_statsPanel; }

QWidget *PlotManager::plotCombo(ChannelComponent *c) { return m_channelPlotcomboMap[c]; }

void PlotManager::updateAxisScales()
{
	for(PlotComponent *plt : plots()) {
		for(PlotWidget *pw : plt->plots()) {
		pw->yAxis()->scaleDraw()->invalidateCache();
		pw->xAxis()->scaleDraw()->invalidateCache();
		if(pw->selectedChannel()) {
			pw->selectedChannel()->yAxis()->scaleDraw()->invalidateCache();
			pw->selectedChannel()->xAxis()->scaleDraw()->invalidateCache();
		}
		pw->replot();
		}
	}
}

void PlotManager::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	PlotComponent *plt = c->plotChannelCmpt()->plotComponent();
	plt->addChannel(c);
	m_channelPlotcomboMap.insert(c, new PlotManagerCombobox(this, c));
	c->addChannelToPlot();
}

void PlotManager::removeChannel(ChannelComponent *c)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->plotComponent()->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
	m_channelPlotcomboMap.remove(c);
}

void PlotManager::moveChannel(ChannelComponent *c, uint32_t uuid)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->plotComponent()->removeChannel(c);
	PlotComponent *plt = plot(uuid);
	c->plotChannelCmpt()->initPlotComponent(plt);
	c->addChannelToPlot();
	plt->addChannel(c);
	plt->replot();
}

PlotComponent *PlotManager::plot(uint32_t uuid)
{
	PlotComponent *plt = nullptr;
	for(PlotComponent *p : qAsConst(m_plots)) {
		if(p->uuid() == uuid) {
			plt = p;
		}
	}
	return plt;
}

QList<PlotComponent *> PlotManager::plots() const { return m_plots; }

void PlotManager::replot()
{
	for(PlotComponent *p : m_plots) {
		p->replot();
	}
}
