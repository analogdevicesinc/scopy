#include "fftplotmanager.h"
#include "fftplotcomponentchannel.h"
#include "fftplotcomponentsettings.h"
#include <plotaxis.h>
#include "plotmanagercombobox.h"

using namespace scopy;
using namespace scopy::adc;

FFTPlotManager::FFTPlotManager(QString name, QWidget *parent)
	: PlotManager(name, parent)
{}

FFTPlotManager::~FFTPlotManager() {}

uint32_t FFTPlotManager::addPlot(QString name)
{
	FFTPlotComponent *plt = new FFTPlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);

	plt->setXInterval(m_xInterval);

	connect(plt, &FFTPlotComponent::requestDeletePlot, this, [=]() {
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

void FFTPlotManager::removePlot(uint32_t uuid)
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

FFTPlotComponent *FFTPlotManager::plot(uint32_t uuid)
{
	return dynamic_cast<FFTPlotComponent *>(PlotManager::plot(uuid));
}

void FFTPlotManager::multiPlotUpdate()
{
	bool b = m_plots.count() > 1;
	for(PlotComponent *p : qAsConst(m_plots)) {
		auto plt = dynamic_cast<FFTPlotComponent *>(p);
		plt->plotMenu()->showDeleteButtons(b);
	}

	for(PlotManagerCombobox *cb : m_channelPlotcomboMap) {
		cb->setVisible(b);
	}
}
