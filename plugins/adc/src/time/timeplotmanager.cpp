#include <timeplotmanager.h>
#include <timeplotcomponentchannel.h>
#include <plotaxis.h>
#include "plotmanagercombobox.h"
#include <timeplotcomponentsettings.h>

using namespace scopy;
using namespace scopy::adc;


TimePlotManager::TimePlotManager(QString name, QWidget *parent) : PlotManager(name, parent){

}

TimePlotManager::~TimePlotManager() {}

uint32_t TimePlotManager::addPlot(QString name)
{
	TimePlotComponent *plt = new TimePlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);

	plt->setXInterval(m_xInterval);

	connect(plt, &TimePlotComponent::requestDeletePlot, this, [=]() {
		Q_EMIT plotRemoved(plt->uuid());
		removePlot(plt->uuid());

		delete plt->plotMenu();
		delete plt;
	});

	addComponent(plt);

	int idx = m_lay->indexOf(m_statsPanel);
	m_lay->insertWidget(idx, plt);
	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->addPlot(plt);
	}

	multiPlotUpdate();
	Q_EMIT plotAdded(plt->uuid());
	return plt->uuid();
}

void TimePlotManager::removePlot(uint32_t uuid)
{
	PlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);

	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->removePlot(plt);
	}

	multiPlotUpdate();
}

TimePlotComponent *TimePlotManager::plot(uint32_t uuid)
{
	return dynamic_cast<TimePlotComponent*>(PlotManager::plot(uuid));
}

void TimePlotManager::multiPlotUpdate() {
	bool b = m_plots.count() > 1;
	for(PlotComponent *p : qAsConst(m_plots)) {
		auto plt = dynamic_cast<TimePlotComponent*>(p);
		plt->plotMenu()->showDeleteButtons(b);
	}

	for(PlotManagerCombobox *cb : m_channelPlotcomboMap) {
		cb->setVisible(b);
	}
}
