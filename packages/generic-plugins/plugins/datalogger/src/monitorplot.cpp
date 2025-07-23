/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "monitorplot.hpp"

#include <QDateTime>
#include <QwtScaleDraw>
#include <QwtScaleEngine>
#include <cursorcontroller.h>
#include <datamonitorutils.hpp>
#include <monitorplotcurve.hpp>
#include <plotinfo.h>
#include <timemanager.hpp>
#include <plotnavigator.hpp>
#include <style.h>
#include <pluginbase/preferences.h>
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlot::MonitorPlot(QString name, uint32_t uuid, QWidget *parent)
	: PlotComponent(name, uuid, parent)
{
	layout = new QVBoxLayout();
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);

	Preferences *p = Preferences::GetInstance();
	dateTimeFormat = p->get("dataloggerplugin_date_time_format").toString();

	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("dataloggerplugin_date_time_format")) {
			dateTimeFormat = p->get("dataloggerplugin_date_time_format").toString();
			setStartTime();
		}
	});

	m_dockableArea = createDockableArea(this);
	QWidget *dockableAreaWidget = dynamic_cast<QWidget *>(m_dockableArea);
	Style::setBackgroundColor(dockableAreaWidget, json::theme::background_subtle, true);
	layout->addWidget(dockableAreaWidget);

	m_monitorPlotWidget = new QWidget(this);
	m_monitorPlotWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *monitorPlotWidgetLayout = new QVBoxLayout(m_monitorPlotWidget);
	monitorPlotWidgetLayout->setContentsMargins(0, 0, 0, 10);

	m_plot = new PlotWidget(m_monitorPlotWidget);
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto nameLbl = m_plot->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	nameLbl->setText(m_name);
	connect(this, &PlotComponent::nameChanged, nameLbl, &QLabel::setText);

	m_plot->yAxis()->setInterval(DataMonitorUtils::getAxisDefaultMinValue(),
				     DataMonitorUtils::getAxisDefaultMaxValue());
	m_plot->yAxis()->setVisible(true);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);

	m_plots.append(m_plot);

	m_xAxisIntervalMin = DataMonitorUtils::getAxisDefaultMaxValue();
	m_xAxisIntervalMax = DataMonitorUtils::getAxisDefaultMinValue();

	startTimeLabel = new QLabel(this);

	HoverWidget *startTime = new HoverWidget(nullptr, m_plot, m_monitorPlotWidget);
	startTime->setContent(startTimeLabel);
	startTime->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	startTime->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	startTime->setAnchorOffset(QPoint(-20, 20));
	startTime->show();
	startTime->setAttribute(Qt::WA_TransparentForMouseEvents);

	setStartTime();
	setupXAxis();

	generateBufferPreviewer();

	m_dockWidet = createDockWrapper("Data Monitor Plot");
	m_dockWidet->setInnerWidget(m_monitorPlotWidget);
	m_dockableArea->addDockWrapper(m_dockWidet);

	m_monitorPlotWidget->layout()->addWidget(m_plot);
	m_plotLayout->addLayout(layout);

	m_monitorCurves = new QMap<QString, MonitorPlotCurve *>();
}

PlotWidget *MonitorPlot::plot() const { return m_plot; }

void MonitorPlot::addMonitor(DataMonitorModel *dataMonitorModel)
{
	MonitorPlotCurve *plotCurve = new MonitorPlotCurve(dataMonitorModel, m_plot);

	connect(dataMonitorModel, &DataMonitorModel::dataCleared, this, [=, this]() {
		plotCurve->clearCurveData();
		updateBufferPreviewer(m_startTime);
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
		if(m_bufferPreviewer->isVisible()) {
			m_bufferPreviewer->updateBufferPreviewer();
		}
		m_plot->replot();
	}
}

bool MonitorPlot::hasMonitor(QString title) { return m_monitorCurves->contains(title); }

void MonitorPlot::updateXAxisIntervalMin(double min)
{
	m_xAxisIntervalMax = min;
	refreshXAxisInterval();
	if(m_bufferPreviewer->isVisible()) {
		m_bufferPreviewer->updateBufferPreviewer();
	}
	m_plot->replot();
}

void MonitorPlot::updateXAxisIntervalMax(double max)
{
	m_xAxisIntervalMin = max;
	refreshXAxisInterval();
	if(m_bufferPreviewer->isVisible()) {
		m_bufferPreviewer->updateBufferPreviewer();
	}
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
	refreshXAxisInterval();
}

void MonitorPlot::setupXAxis()
{
	m_plot->xAxis()->setVisible(true);

	scaleEngine = new QwtDateScaleEngine(Qt::OffsetFromUTC);
	m_plot->plot()->setAxisScaleEngine(m_plot->xAxis()->axisId(), scaleEngine);

	updateAxisScaleDraw();
	refreshXAxisInterval();
	m_plot->setAlignCanvasToScales(false);

	m_plot->replot();
}

void MonitorPlot::genereateScaleDraw(QString format, double offset)
{
	m_scaleDraw = new QwtDateScaleDraw(Qt::OffsetFromUTC);
	m_scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	m_scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);

	// set time format for time interval types
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Millisecond, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Second, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Minute, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Hour, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Day, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Month, format);
	m_scaleDraw->setDateFormat(QwtDate::IntervalType::Year, format);
	m_scaleDraw->setUtcOffset(offset);

	// apply scale draw to axis
	m_plot->plot()->setAxisScaleDraw(m_plot->xAxis()->axisId(), m_scaleDraw);

	// make label more readable
	m_plot->plot()->setAxisLabelRotation(m_plot->xAxis()->axisId(), -50.0);
	m_plot->plot()->setAxisLabelAlignment(m_plot->xAxis()->axisId(), Qt::AlignLeft | Qt::AlignBottom);
}

void MonitorPlot::setStartTime()
{
	auto &&timeTracker = TimeManager::GetInstance();
	m_startTime = QwtDate::toDouble(timeTracker->startTime());

	QString formattedTime = timeTracker->startTime().toString(dateTimeFormat);
	QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
	startTimeLabel->setText(QString("Start time: " + formattedTimeMsg));
	genereateScaleDraw(dateTimeFormat, QDateTime::currentDateTime().offsetFromUtc());
	setupXAxis();
}

void MonitorPlot::updateAxisScaleDraw()
{
	if(m_isRealTime) {
		m_scaleDraw->setUtcOffset(QDateTime::currentDateTime().offsetFromUtc());
	} else {
		double offset = (-1) * m_startTime / 1000;
		m_scaleDraw->setUtcOffset(offset);
	}

	m_plot->replot();
}

void MonitorPlot::refreshXAxisInterval()
{
	double time = QwtDate::toDouble(QDateTime::currentDateTime());
	double delta = m_xAxisIntervalMin - m_xAxisIntervalMax;
	if(m_plot->navigator()->isZoomed()) {
		delta = std::abs(m_plot->xAxis()->visibleMax() - m_plot->xAxis()->visibleMin()) / 1000;
	}
	m_plot->xAxis()->setInterval(time - (delta * 1000), time);
	m_plot->replot();
}

void MonitorPlot::updatePlotStartingPoint(double time, double delta)
{
	if(m_plot->navigator()->isZoomed()) {
		delta = std::abs(m_plot->xAxis()->visibleMax() - m_plot->xAxis()->visibleMin());
	} else {
		delta = delta * 1000;
	}

	if(m_isRealTime) {
		m_plot->xAxis()->setInterval(time - delta, time);
	} else {
		double offset = (-1) * m_startTime / 1000;
		m_scaleDraw->setUtcOffset(offset);

		m_plot->xAxis()->setInterval(time - delta, time);
	}

	updateBufferPreviewer(time);
	m_plot->replot();
}

void MonitorPlot::toggleBufferPreview(bool toggled)
{
	m_bufferPreviewer->setVisible(toggled);
	Q_EMIT bufferPreviewerToggled(toggled);
}

void MonitorPlot::updateBufferPreviewer(double time)
{
	if(m_bufferPreviewer->isVisible()) {
		m_bufferPreviewer->updateDataLimits(m_startTime, time);
	}
}

PlotBufferPreviewer *MonitorPlot::bufferPreviewer() const { return m_bufferPreviewer; }

void MonitorPlot::generateBufferPreviewer()
{
	AnalogBufferPreviewer *analogBufferPreviewer = new AnalogBufferPreviewer(this);
	m_bufferPreviewer = new PlotBufferPreviewer(m_plot, analogBufferPreviewer, this);
	m_bufferPreviewer->setManualDataLimits(true);

	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this, [=, this]() {
		double time = QwtDate::toDouble(QDateTime::currentDateTime());
		updateBufferPreviewer(time);
	});
	m_plot->navigator()->setResetOnNewBase(false);
	m_monitorPlotWidget->layout()->addWidget(m_bufferPreviewer);
}

#include "moc_monitorplot.cpp"
