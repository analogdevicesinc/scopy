#include "monitorplot.hpp"

#include <cursorcontroller.h>
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

	// TODO set this and set autoscale
	m_plot->xAxis()->setInterval(0, 10);
	m_plot->xAxis()->setVisible(true);

	m_plotInfo = new TimePlotInfo(m_plot, this);

	layout->addWidget(m_plotInfo);
	layout->addWidget(m_plot);

	TimePlotStatusInfo *status = new TimePlotStatusInfo(this);
	layout->addWidget(status);

	m_monitorCurves = new QMap<QString, MonitorPlotCurve *>();

	m_mainMonitor = "";
}

PlotWidget *MonitorPlot::plot() const { return m_plot; }

void MonitorPlot::addMonitor(DataMonitorModel *dataMonitorModel)
{
	MonitorPlotCurve *plotCurve = new MonitorPlotCurve(dataMonitorModel, m_plot);

	// ?? move to function
	if(m_mainMonitor.isEmpty()) {
		m_mainMonitor = dataMonitorModel->getName();
		plotCurve->togglePlotAxisVisible(true);
	}

	// TODO CLEAR PLOT ON CLEAR DATA
	connect(dataMonitorModel, &DataMonitorModel::dataCleared, this, [=]() {
		plotCurve->clearCurveData();
		m_plot->replot();
	});

	m_monitorCurves->insert(dataMonitorModel->getName(), plotCurve);

	m_plot->replot();
}

void MonitorPlot::removeMonitor(QString monitorName)
{
	m_plot->removePlotChannel(m_monitorCurves->value(monitorName)->plotch());
	m_monitorCurves->remove(monitorName);
	delete m_monitorCurves->value(monitorName);
}

void MonitorPlot::toggleMonitor(bool toggled, QString monitorName)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->toggleActive(toggled);
		m_plot->replot();
	}
}

bool MonitorPlot::hasMonitor(QString title) { return m_monitorCurves->contains(title); }

void MonitorPlot::changeCurveStyle(QString monitorName, int style)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->changeCurveStyle(style);
		m_plot->replot();
	}
}

void MonitorPlot::setMainMonitor(QString newMainMonitor)
{
	if(m_monitorCurves->contains(newMainMonitor)) {
		m_monitorCurves->value(newMainMonitor)->refreshCurve();
		m_monitorCurves->value(newMainMonitor)->setCurveAxisVisible(true);
		m_monitorCurves->value(m_mainMonitor)->setCurveAxisVisible(false);
		m_mainMonitor = newMainMonitor;
	}
	m_plot->replot();
}

void MonitorPlot::clearMonitor() {}

void MonitorPlot::updateXAxis(int newValue)
{
	m_plot->xAxis()->setInterval(0, newValue);
	m_plot->replot();
}

void MonitorPlot::updateYAxisIntervalMin(QString monitorName, double min)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->updateAxisIntervalMin(min);
		m_plot->replot();
	}
}

void MonitorPlot::updateYAxisIntervalMax(QString monitorName, double max)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->updateAxisIntervalMax(max);
		m_plot->replot();
	}
}

QString MonitorPlot::mainMonitor() const { return m_mainMonitor; }

void MonitorPlot::plotYAxisAutoscaleToggled(QString monitorName, bool toggled)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_plot->plot()->setAxisAutoScale(m_monitorCurves->value(m_mainMonitor)->getAxisId(), toggled);
		m_plot->replot();
	}
}
