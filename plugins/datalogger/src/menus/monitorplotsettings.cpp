/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "monitorplotsettings.h"

#include <datamonitorutils.hpp>
#include <menuplotaxisrangecontrol.h>
#include <menuplotchannelcurvestylecontrol.h>
#include <style.h>
#include <timemanager.hpp>
#include <QLineEdit>

using namespace scopy;
using namespace datamonitor;

MonitorPlotSettings::MonitorPlotSettings(MonitorPlot *m_plot, QWidget *parent)
	: m_plot(m_plot)
	, QWidget{parent}
{
	mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	// Plot title edit
	mainLayout->addWidget(generateEditTitleSettings(this));

	// YAxis settings
	mainLayout->addWidget(generateYAxisSettings(this));

	/// Curve style
	mainLayout->addWidget(generateCurveStyleSettings(this));

	////Plot style
	mainLayout->addWidget(generatePlotUiSettings(this));

	///// delete plot button //////
	m_deletePlot = new QPushButton("DELETE PLOT", this);
	m_deletePlot->setVisible(false);
	mainLayout->addWidget(m_deletePlot);
	Style::setStyle(m_deletePlot, style::properties::button::basicButton);

	connect(m_deletePlot, &QPushButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(m_plot->uuid()); });

	MouseWheelWidgetGuard *mouseWheelWidgetGuard = new MouseWheelWidgetGuard(this);
	mouseWheelWidgetGuard->installEventRecursively(this);

	QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding);
	mainLayout->addItem(spacer);
}

void MonitorPlotSettings::plotYAxisMinValueUpdate(double value) { m_ymin->setValue(value); }

void MonitorPlotSettings::plotYAxisMaxValueUpdate(double value) { m_ymax->setValue(value); }

void MonitorPlotSettings::toggleDeleteButtonVisible(bool visible) { m_deletePlot->setVisible(visible); }

QWidget *MonitorPlotSettings::generateEditTitleSettings(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(widget);
	widget->setLayout(layout);

	QLineEdit *titleEdit = new QLineEdit(m_plot->name(), widget);
	connect(titleEdit, &QLineEdit::textChanged, this, [=](const QString &text) { m_plot->setName(text); });

	layout->addWidget(new QLabel("Title:"));
	layout->addWidget(titleEdit);

	return widget;
}

QWidget *MonitorPlotSettings::generateYAxisSettings(QWidget *parent)
{
	MenuSectionWidget *yaxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yAxisSection = new MenuCollapseSection(
		"Y-AXIS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, yaxisContainer);

	yaxisContainer->contentLayout()->addWidget(yAxisSection);
	yaxisContainer->contentLayout()->setMargin(0);

	gui::MenuPlotAxisRangeControl *plotYAxisController =
		new gui::MenuPlotAxisRangeControl(m_plot->plot()->yAxis(), this);

	plotYAxisController->setMin(DataMonitorUtils::getAxisDefaultMinValue());
	plotYAxisController->setMax(DataMonitorUtils::getAxisDefaultMaxValue());

	gui::PlotAutoscaler *plotAutoscaler = new gui::PlotAutoscaler(yAxisSection);
	connect(m_plot, &MonitorPlot::monitorCurveAdded, plotAutoscaler, &gui::PlotAutoscaler::addChannels);
	connect(m_plot, &MonitorPlot::monitorCurveRemoved, plotAutoscaler, &gui::PlotAutoscaler::removeChannels);

	plotAutoscaler->setTolerance(10);

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yAxisSection, false);

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		plotYAxisController->setEnabled(!toggled);
		if(toggled) {
			plotAutoscaler->start();
		} else {
			plotAutoscaler->stop();
		}
	});

	autoscale->onOffswitch()->setChecked(true);

	auto &&timeTracker = TimeManager::GetInstance();

	connect(timeTracker, &TimeManager::toggleRunning, this, [=, this](bool toggled) {
		if(toggled) {
			if(autoscale->onOffswitch()->isChecked()) {
				plotAutoscaler->start();
			}
		} else {
			plotAutoscaler->stop();
		}
	});

	connect(plotAutoscaler, &gui::PlotAutoscaler::newMin, m_plot, &MonitorPlot::updateYAxisIntervalMin);
	connect(plotAutoscaler, &gui::PlotAutoscaler::newMax, m_plot, &MonitorPlot::updateYAxisIntervalMax);

	yAxisSection->contentLayout()->addWidget(autoscale);
	yAxisSection->contentLayout()->addWidget(plotYAxisController);

	return yaxisContainer;
}

QWidget *MonitorPlotSettings::generateCurveStyleSettings(QWidget *parent)
{
	MenuSectionWidget *curveStylecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curveStyleSection = new MenuCollapseSection(
		"CURVE", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, curveStylecontainer);

	curveStylecontainer->contentLayout()->setMargin(0);
	gui::MenuPlotChannelCurveStyleControl *curveMenu = new gui::MenuPlotChannelCurveStyleControl(curveStyleSection);

	connect(m_plot, &MonitorPlot::monitorCurveAdded, curveMenu,
		&gui::MenuPlotChannelCurveStyleControl::addChannels);
	connect(m_plot, &MonitorPlot::monitorCurveRemoved, curveMenu,
		&gui::MenuPlotChannelCurveStyleControl::removeChannels);

	curveStyleSection->contentLayout()->addWidget(curveMenu);
	curveStylecontainer->contentLayout()->addWidget(curveStyleSection);

	return curveStylecontainer;
}

QWidget *MonitorPlotSettings::generatePlotUiSettings(QWidget *parent)
{
	MenuSectionWidget *plotStylecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *plotStyleSection =
		new MenuCollapseSection("PLOT SETTINGS", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MHW_BASEWIDGET, plotStylecontainer);

	plotStyleSection->contentLayout()->setMargin(0);
	plotStyleSection->contentLayout()->setSpacing(10);

	MenuOnOffSwitch *showYAxisLabel = new MenuOnOffSwitch(tr("Y-AXIS label"), plotStyleSection, false);
	showYAxisLabel->onOffswitch()->setChecked(true);

	connect(showYAxisLabel->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->plot()->yAxis()->setVisible(toggled); });

	MenuOnOffSwitch *showXAxisLabel = new MenuOnOffSwitch(tr("X-AXIS label"), plotStyleSection, false);
	showXAxisLabel->onOffswitch()->setChecked(true);

	connect(showXAxisLabel->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->plot()->xAxis()->setVisible(toggled); });

	MenuOnOffSwitch *showBufferPreview = new MenuOnOffSwitch(tr("Buffer Preview"), plotStyleSection, false);
	showBufferPreview->onOffswitch()->setChecked(true);

	connect(showBufferPreview->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->toggleBufferPreview(toggled); });

	plotStyleSection->contentLayout()->addWidget(showBufferPreview);
	plotStyleSection->contentLayout()->addWidget(showXAxisLabel);
	plotStyleSection->contentLayout()->addWidget(showYAxisLabel);

	plotStylecontainer->contentLayout()->addWidget(plotStyleSection);

	return plotStylecontainer;
}

#include "moc_monitorplotsettings.cpp"
