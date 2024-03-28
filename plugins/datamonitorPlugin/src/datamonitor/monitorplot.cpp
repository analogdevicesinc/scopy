#include "monitorplot.hpp"

#include <QDateTime>
#include <QwtScaleDraw>
#include <QwtScaleEngine>
#include <cursorcontroller.h>
#include <datamonitorutils.hpp>
#include <monitorplotcurve.hpp>
#include <plotinfo.h>
#include <timemanager.hpp>

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlot::MonitorPlot(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	m_plot = new PlotWidget(this);

	m_plot->yAxis()->setInterval(DataMonitorUtils::getAxisDefaultMinValue(),
				     DataMonitorUtils::getAxisDefaultMaxValue());
	m_plot->yAxis()->setVisible(true);

	m_xAxisIntervalMin = DataMonitorUtils::getAxisDefaultMaxValue();
	m_xAxisIntervalMax = DataMonitorUtils::getAxisDefaultMinValue();

	startTimeLabel = new QLabel(this);

	HoverWidget *startTime = new HoverWidget(nullptr, m_plot, this);
	startTime->setContent(startTimeLabel);
	startTime->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	startTime->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	startTime->setAnchorOffset(QPoint(-20, 20));
	startTime->show();
	startTime->setAttribute(Qt::WA_TransparentForMouseEvents);

	setStartTime();

	setupXAxis();

	m_plotInfo = new TimePlotInfo(m_plot, this);

	layout->addWidget(m_plotInfo);
	layout->addWidget(m_plot);

	m_monitorCurves = new QMap<QString, MonitorPlotCurve *>();
}

PlotWidget *MonitorPlot::plot() const { return m_plot; }

void MonitorPlot::addMonitor(DataMonitorModel *dataMonitorModel)
{
	MonitorPlotCurve *plotCurve = new MonitorPlotCurve(dataMonitorModel, m_plot);

	connect(dataMonitorModel, &DataMonitorModel::dataCleared, this, [=, this]() {
		plotCurve->clearCurveData();
		m_plot->replot();
	});

	m_monitorCurves->insert(dataMonitorModel->getName(), plotCurve);

	Q_EMIT monitorCurveAdded(plotCurve->plotch());

	m_plot->replot();
}

void MonitorPlot::removeMonitor(QString monitorName)
{
	if(m_monitorCurves->contains(monitorName)) {
		Q_EMIT monitorCurveRemoved(m_monitorCurves->value(monitorName)->plotch());
		m_plot->removePlotChannel(m_monitorCurves->value(monitorName)->plotch());
		m_monitorCurves->value(monitorName)->plotch()->curve()->detach();
		m_monitorCurves->remove(monitorName);
		delete m_monitorCurves->value(monitorName);
	}
	m_plot->replot();
}

void MonitorPlot::toggleMonitor(bool toggled, QString monitorName)
{
	if(m_monitorCurves->contains(monitorName)) {
		m_monitorCurves->value(monitorName)->toggleActive(toggled);
		m_plot->replot();
	}
}

bool MonitorPlot::hasMonitor(QString title) { return m_monitorCurves->contains(title); }

void MonitorPlot::updateXAxisIntervalMin(double min)
{
	m_xAxisIntervalMax = min;
	refreshXAxisInterval();
	m_plotInfo->updateBufferPreviewer();
	m_plot->replot();
}

void MonitorPlot::updateXAxisIntervalMax(double max)
{
	m_xAxisIntervalMin = max;
	refreshXAxisInterval();
	m_plotInfo->updateBufferPreviewer();
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
	refreshXAxisInterval();

	m_plot->replot();
}

void MonitorPlot::genereateScaleDraw(QString format, double offset)
{
	QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw(Qt::OffsetFromUTC);
	scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);

	// set time format for time interval types
	scaleDraw->setDateFormat(QwtDate::IntervalType::Second, format);
	scaleDraw->setDateFormat(QwtDate::IntervalType::Minute, format);
	scaleDraw->setDateFormat(QwtDate::IntervalType::Hour, format);
	scaleDraw->setDateFormat(QwtDate::IntervalType::Day, format);
	scaleDraw->setDateFormat(QwtDate::IntervalType::Month, format);
	scaleDraw->setDateFormat(QwtDate::IntervalType::Year, format);
	scaleDraw->setUtcOffset(offset);

	// apply scale draw to axis
	m_plot->plot()->setAxisScaleDraw(m_plot->xAxis()->axisId(), scaleDraw);

	// make label more readable
	m_plot->plot()->setAxisLabelRotation(m_plot->xAxis()->axisId(), -50.0);
	m_plot->plot()->setAxisLabelAlignment(m_plot->xAxis()->axisId(), Qt::AlignLeft | Qt::AlignBottom);
}

void MonitorPlot::setStartTime()
{
	auto &&timeTracker = TimeManager::GetInstance();
	m_startTime = QwtDate::toDouble(timeTracker->startTime());

	QString formattedTime = timeTracker->startTime().toString("MM/dd/yy hh:mm:ss");
	QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
	startTimeLabel->setText(QString(formattedTimeMsg));

	updateAxisScaleDraw();
}

void MonitorPlot::updateAxisScaleDraw()
{
	if(m_isRealTime) {
		genereateScaleDraw(DataMonitorUtils::getPlotDateTimeFormat(),
				   QDateTime::currentDateTime().offsetFromUtc());
	} else {
		double offset = (-1) * m_startTime / 1000;
		genereateScaleDraw(DataMonitorUtils::getPlotDateTimeFormat(), offset);
	}

	m_plot->replot();
}

void MonitorPlot::refreshXAxisInterval()
{
	double time = QwtDate::toDouble(QDateTime::currentDateTime());
	double delta = m_xAxisIntervalMin - m_xAxisIntervalMax;
	m_plot->xAxis()->setInterval(time - (delta * 1000), time);
	m_plot->replot();
}

void MonitorPlot::updatePlotStartingPoint(double time, double delta)
{
	if(m_isRealTime) {
		m_plot->xAxis()->setInterval(time - (delta * 1000), time);
	} else {
		double offset = (-1) * m_startTime / 1000;
		genereateScaleDraw(DataMonitorUtils::getPlotDateTimeFormat(), offset);

		m_plot->xAxis()->setInterval(time - (delta * 1000), time);
	}

	m_plotInfo->updateBufferPreviewer();
	m_plot->replot();
}
