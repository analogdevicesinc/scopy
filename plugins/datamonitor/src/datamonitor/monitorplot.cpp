#include "monitorplot.hpp"

#include <QDateTime>
#include <QwtScaleDraw>
#include <QwtScaleEngine>
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

	m_plot->yAxis()->setInterval(DataMonitorUtils::getAxisDefaultMinValue(),
				     DataMonitorUtils::getAxisDefaultMaxValue());
	m_plot->yAxis()->setVisible(true);

	m_xAxisIntervalMin = DataMonitorUtils::getAxisDefaultMinValue();
	m_xAxisIntervalMax = DataMonitorUtils::getAxisDefaultMaxValue();

	m_startTime = QwtDate::toDouble(QDateTime::currentDateTimeUtc());

	setupXAxis();

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
	m_xAxisIntervalMin = min;
	m_plot->xAxis()->setMin(m_startTime + (min * 1000));
	m_plot->replot();
}

void MonitorPlot::updateXAxisIntervalMax(double max)
{
	m_xAxisIntervalMax = max;
	m_plot->xAxis()->setMax(m_startTime + (max * 1000));
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

	// make the min/max value to be 10% extra so the curve is not on the edge of the axis
	min = min - min * 0.1;
	max = max + max * 0.1;
	updateYAxisIntervalMin(min);
	updateYAxisIntervalMax(max);
}

double MonitorPlot::getYAxisIntervalMin() { return m_plot->yAxis()->min(); }

double MonitorPlot::getYAxisIntervalMax() { return m_plot->yAxis()->max(); }

void MonitorPlot::setIsRealTime(bool newIsRealTime)
{
	m_isRealTime = newIsRealTime;
	updateAxisScaleDraw();
}

void MonitorPlot::setupXAxis()
{
	m_plot->xAxis()->setVisible(true);

	scaleEngine = new QwtDateScaleEngine(Qt::OffsetFromUTC);
	m_plot->plot()->setAxisScaleEngine(m_plot->xAxis()->axisId(), scaleEngine);

	updateAxisScaleDraw();

	m_plot->replot();
}

QwtDateScaleDraw *MonitorPlot::genereateScaleDraw(QString format, double offset, QwtDate::IntervalType intervalType)
{
	QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw(Qt::OffsetFromUTC);
	scaleDraw->setDateFormat(intervalType, format);
	scaleDraw->setUtcOffset(offset);

	return scaleDraw;
}

void MonitorPlot::setStartTime()
{
	m_startTime = QwtDate::toDouble(QDateTime::currentDateTimeUtc());
	updateAxisScaleDraw();
}

void MonitorPlot::updateAxisScaleDraw()
{
	if(m_isRealTime) {

		QwtDateScaleDraw *scaleDraw = genereateScaleDraw(
			"hh:mm:ss", QDateTime::currentDateTime().offsetFromUtc(), QwtDate::IntervalType::Second);
		m_plot->plot()->setAxisScaleDraw(m_plot->xAxis()->axisId(), scaleDraw);

	} else {

		double offset = (-1) * m_startTime / 1000;
		QwtDateScaleDraw *scaleDraw = genereateScaleDraw("hh:mm:ss", offset, QwtDate::IntervalType::Second);
		m_plot->plot()->setAxisScaleDraw(m_plot->xAxis()->axisId(), scaleDraw);
	}

	m_plot->xAxis()->setInterval(m_startTime + (m_xAxisIntervalMin * 1000),
				     m_startTime + (m_xAxisIntervalMax * 1000));

	//make label more readable
	m_plot->plot()->setAxisLabelRotation(m_plot->xAxis()->axisId(), -50.0);
	m_plot->plot()->setAxisLabelAlignment(m_plot->xAxis()->axisId(), Qt::AlignLeft | Qt::AlignBottom);
	m_plot->replot();
}
