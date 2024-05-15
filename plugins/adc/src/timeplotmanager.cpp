#include <timeplotmanager.h>
#include <timeplotcomponentchannel.h>
#include <plotaxis.h>
#include <timeplotmanagercombobox.h>
#include <timeplotcomponentsettings.h>

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
	// tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(80);
	// tool->bottomStack()->add(statsMenuId, m_statsPanel);
	m_lay->addWidget(m_measurePanel);
	m_measurePanel->setVisible(false);
	m_statsPanel->setVisible(false);
	m_lay->addWidget(m_statsPanel);
}

TimePlotManager::~TimePlotManager() {}

void TimePlotManager::enableMeasurementPanel(bool b) { m_measurePanel->setVisible(b); }

void TimePlotManager::enableStatsPanel(bool b) { m_statsPanel->setVisible(b); }

void TimePlotManager::setXInterval(double xMin, double xMax)
{
	for(auto plt : qAsConst(m_plots)) {
		plt->timePlot()->xAxis()->setInterval(xMin, xMax);
	}
}

void TimePlotManager::selectChannel(ChannelComponent *c)
{
	for(TimePlotComponentChannel *pcc : qAsConst(m_channels)) {
		if(pcc->m_ch == c) {
			PlotChannel *ch = pcc->m_timePlotCh;
			PlotWidget *w = pcc->m_plotComponent->timePlot();
			w->selectChannel(ch);
		}
	}
}

MeasurementsPanel *TimePlotManager::measurePanel() const { return m_measurePanel; }

StatsPanel *TimePlotManager::statsPanel() const { return m_statsPanel; }

QWidget *TimePlotManager::plotCombo(ChannelComponent *c) { return m_channelPlotcomboMap[c]; }

uint32_t TimePlotManager::addPlot(QString name)
{
	TimePlotComponent *plt = new TimePlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);

	connect(plt, &TimePlotComponent::requestDeletePlot, this, [=]() {
		Q_EMIT plotRemoved(plt->uuid());
		removePlot(plt->uuid());

		delete plt->plotMenu();
		delete plt;
	});

	addComponent(plt);

	int idx = m_lay->indexOf(m_statsPanel);
	m_lay->insertWidget(idx, plt);
	for(TimePlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->addPlot(plt);
	}

	bool b = m_plots.count() > 1;
	for(TimePlotComponent *plt : qAsConst(m_plots)) {
		plt->plotMenu()->showDeleteButtons(b);
	}

	return plt->uuid();
}

void TimePlotManager::removePlot(uint32_t uuid)
{
	TimePlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);

	for(TimePlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->removePlot(plt);
	}

	bool b = m_plots.count() > 1;
	for(TimePlotComponent *plt : qAsConst(m_plots)) {
		plt->plotMenu()->showDeleteButtons(b);
	}
}

void TimePlotManager::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	TimePlotComponent *plt = c->plotChannelCmpt()->m_plotComponent;
	plt->addChannel(c);
	m_channelPlotcomboMap.insert(c, new TimePlotManagerCombobox(this, c));
	c->addChannelToPlot();
}

void TimePlotManager::removeChannel(ChannelComponent *c)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
	m_channelPlotcomboMap.remove(c);
}

void TimePlotManager::moveChannel(ChannelComponent *c, uint32_t uuid)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	TimePlotComponent *plt = plot(uuid);
	c->plotChannelCmpt()->initPlotComponent(plt);
	c->addChannelToPlot();
	plt->addChannel(c);
	plt->replot();
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

QList<TimePlotComponent *> TimePlotManager::plots() const { return m_plots; }

void TimePlotManager::replot()
{
	for(TimePlotComponent *p : m_plots) {
		p->replot();
	}
}
