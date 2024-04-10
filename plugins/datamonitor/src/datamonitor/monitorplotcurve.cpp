#include "monitorplotcurve.hpp"

#include <datamonitorutils.hpp>
#include <plotaxis.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlotCurve::MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent)
	: m_dataMonitorModel(dataMonitorModel)
{
	m_curveStyleIndex = 0;
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

void MonitorPlotCurve::changeCurveStyle(int style)
{
	m_curveStyleIndex = style;
	m_plotch->curve()->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
	m_plotch->curve()->setCurveAttribute(QwtPlotCurve::Fitted, false);

	switch(style) {
	case 0:
		m_plotch->curve()->setStyle(QwtPlotCurve::CurveStyle::Lines);
		break;
	case 1:
		m_plotch->curve()->setStyle(QwtPlotCurve::CurveStyle::Dots);
		break;
	case 2:
		m_plotch->curve()->setStyle(QwtPlotCurve::CurveStyle::Steps);
		break;
	case 3:
		m_plotch->curve()->setStyle(QwtPlotCurve::CurveStyle::Sticks);
		break;
	case 4:
		m_plotch->curve()->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
		m_plotch->curve()->setCurveAttribute(QwtPlotCurve::Fitted, true);
		m_plotch->curve()->setStyle(QwtPlotCurve::CurveStyle::Lines);
		break;
	}
}

void MonitorPlotCurve::changeCurveThickness(double thickness) { m_plotch->setThickness(thickness); }

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

int MonitorPlotCurve::curveStyleIndex() const { return m_curveStyleIndex; }

double MonitorPlotCurve::curveMinVal() { return m_dataMonitorModel->minValue(); }

double MonitorPlotCurve::curveMaxVal() { return m_dataMonitorModel->maxValue(); }
