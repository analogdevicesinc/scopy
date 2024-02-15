#include "monitorplot.hpp"

#include <cursorcontroller.h>
#include <datamonitorutils.hpp>
#include <monitorplotcurve.hpp>
#include <plotinfo.h>

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlot::MonitorPlot(QWidget *parent)
	: QWidget{parent}
{
	installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	m_plot = new PlotWidget(this);

	m_plot->xAxis()->setInterval(DataMonitorUtils::getAxisDefaultMaxValue(),
				     DataMonitorUtils::getAxisDefaultMinValue());
	m_plot->xAxis()->setVisible(true);

	m_plot->yAxis()->setInterval(DataMonitorUtils::getAxisDefaultMinValue(),
				     DataMonitorUtils::getAxisDefaultMaxValue());
	m_plot->yAxis()->setVisible(true);

	m_plotInfo = new TimePlotInfo(m_plot, this);

	layout->addWidget(m_plotInfo);
	layout->addWidget(m_plot);

	TimePlotStatusInfo *status = new TimePlotStatusInfo(this);
	layout->addWidget(status);

	m_monitorCurves = new QMap<QString, MonitorPlotCurve *>();
}

PlotWidget *MonitorPlot::plot() const { return m_plot; }

void MonitorPlot::addMonitor(DataMonitorModel *dataMonitorModel)
{
	MonitorPlotCurve *plotCurve = new MonitorPlotCurve(dataMonitorModel, m_plot);
	plotCurve->changeCurveStyle(m_currentCurveStyle);
	plotCurve->changeCurveThickness(m_currentCurveThickness);

	connect(dataMonitorModel, &DataMonitorModel::dataCleared, this, [=]() {
		plotCurve->clearCurveData();
		m_plot->replot();
	});

	m_monitorCurves->insert(dataMonitorModel->getName(), plotCurve);

	m_plot->replot();
}

void MonitorPlot::removeMonitor(QString monitorName)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_plot->removePlotChannel(m_monitorCurves->value(monitorName)->plotch());
		m_monitorCurves->remove(monitorName);
		delete m_monitorCurves->value(monitorName);
	}
}

void MonitorPlot::toggleMonitor(bool toggled, QString monitorName)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->toggleActive(toggled);
		m_plot->replot();
	}
}

bool MonitorPlot::hasMonitor(QString title) { return m_monitorCurves->contains(title); }

void MonitorPlot::changeCurveStyle(int style)
{
	m_currentCurveStyle = style;
	foreach(QString curve, m_monitorCurves->keys()) {
		m_monitorCurves->value(curve)->changeCurveStyle(style);
	}
	m_plot->replot();
}

void MonitorPlot::changeCurveThickness(double thickness)
{
	m_currentCurveThickness = thickness;
	foreach(QString curve, m_monitorCurves->keys()) {
		m_monitorCurves->value(curve)->changeCurveThickness(thickness);
	}
	m_plot->replot();
}

void MonitorPlot::updateXAxisIntervalMin(double min)
{
	m_plot->xAxis()->setMax(min);
	m_plot->replot();
}

void MonitorPlot::updateXAxisIntervalMax(double max)
{
	m_plot->xAxis()->setMin(max);
	m_plot->replot();
}

void MonitorPlot::updateYAxisIntervalMin(double min)
{
	m_plot->yAxis()->setMin(min);
	m_plot->replot();
}

void MonitorPlot::updateYAxisIntervalMax(double max)
{
	m_plot->yAxis()->setMax(max);
	m_plot->replot();
}

void MonitorPlot::plotYAxisAutoscale()
{
	double max = getYAxisIntervalMin();
	double min = getYAxisIntervalMax();

	foreach(QString curve, m_monitorCurves->keys()) {
		double curveMin = m_monitorCurves->value(curve)->curveMinVal();
		double curveMax = m_monitorCurves->value(curve)->curveMaxVal();

		if(max < curveMax)
			max = curveMax;
		if(min > curveMin)
			min = curveMin;
	}

	updateYAxisIntervalMin(min);
	updateYAxisIntervalMax(max);
}

double MonitorPlot::getYAxisIntervalMin() { return m_plot->yAxis()->min(); }

double MonitorPlot::getYAxisIntervalMax() { return m_plot->yAxis()->max(); }
