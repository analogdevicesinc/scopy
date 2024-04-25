#include "monitorplotcurve.hpp"

#include <datamonitorutils.hpp>
#include <plotaxis.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlotCurve::MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent)
	: m_dataMonitorModel(dataMonitorModel)
	, m_plot(plot)
{
	QPen chpen = QPen(dataMonitorModel->getColor(), 1);

	m_plotch = new PlotChannel(dataMonitorModel->getName(), chpen, plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(m_plotch);
	m_plotch->setEnabled(true);

	m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(), m_dataMonitorModel->getYdata()->data(),
					 m_dataMonitorModel->getYdata()->size());

	connect(dataMonitorModel, &DataMonitorModel::valueUpdated, plot, [=, this]() {
		m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(),
						 m_dataMonitorModel->getYdata()->data(),
						 m_dataMonitorModel->getYdata()->size());
		plot->replot();
	});
}

MonitorPlotCurve::~MonitorPlotCurve() { m_plot->removePlotChannel(m_plotch); }

void MonitorPlotCurve::clearCurveData()
{
	m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(), m_dataMonitorModel->getYdata()->data(),
					 m_dataMonitorModel->getYdata()->size());
}

void MonitorPlotCurve::refreshCurve()
{
	m_plotch->detach();
	m_plotch->attach();
}

void MonitorPlotCurve::toggleActive(bool toggled) { m_plotch->setEnabled(toggled); }

PlotChannel *MonitorPlotCurve::plotch() const { return m_plotch; }

double MonitorPlotCurve::curveMinVal() { return m_dataMonitorModel->minValue(); }

double MonitorPlotCurve::curveMaxVal() { return m_dataMonitorModel->maxValue(); }
