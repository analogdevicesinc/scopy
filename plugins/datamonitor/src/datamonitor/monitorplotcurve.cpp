#include "monitorplotcurve.hpp"

#include <datamonitorutils.hpp>
#include <plotaxis.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlotCurve::MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent)
{
	yAxisMin = DataMonitorUtils::getYAxisDefaultMinValue();
	yAxisMax = DataMonitorUtils::getYAxisDefaultMaxValue();

	QPen chpen = QPen(dataMonitorModel->getColor(), 1);
	chPlotAxis = new PlotAxis(QwtAxis::YLeft, plot, chpen);

	plot->plot()->setAxisScale(chPlotAxis->axisId(), yAxisMin, yAxisMax);

	m_plotch = new PlotChannel(dataMonitorModel->getName(), chpen, plot, plot->xAxis(), chPlotAxis, this);
	m_plotch->setEnabled(true);

	xdata = dataMonitorModel->getXdata();
	ydata = dataMonitorModel->getYdata();

	m_plotch->curve()->setRawSamples(xdata->data(), ydata->data(), ydata->size());

	connect(dataMonitorModel, &DataMonitorModel::valueUpdated, plot, [=]() {
		m_plotch->curve()->setRawSamples(xdata->data(), ydata->data(), ydata->size());
		plot->replot();
	});
}

void MonitorPlotCurve::togglePlotAxisVisible(bool toggle) { chPlotAxis->setVisible(toggle); }

void MonitorPlotCurve::changeCurveStyle(int style)
{
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

void MonitorPlotCurve::clearCurveData()
{
	m_plotch->curve()->setRawSamples(xdata->data(), ydata->data(), ydata->size());
}

void MonitorPlotCurve::refreshCurve()
{
	m_plotch->detach();
	m_plotch->attach();
}

void MonitorPlotCurve::toggleActive(bool toggled) { m_plotch->setEnabled(toggled); }

void MonitorPlotCurve::setCurveAxisVisible(bool visible) { m_plotch->yAxis()->setVisible(visible); }

void MonitorPlotCurve::updateAxisIntervalMin(double min) { m_plotch->yAxis()->setMin(min); }

void MonitorPlotCurve::updateAxisIntervalMax(double max) { m_plotch->yAxis()->setMax(max); }

const QwtAxisId MonitorPlotCurve::getAxisId() { return m_plotch->yAxis()->axisId(); }

PlotChannel *MonitorPlotCurve::plotch() const { return m_plotch; }
