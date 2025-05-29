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

#include "menus/datamonitorsettings.hpp"
#include "menus/plottimeaxiscontroller.hpp"

#include <QDateTimeEdit>
#include <QDebug>
#include <QLineEdit>
#include <QScrollArea>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menuplotaxisrangecontrol.h>
#include <menuplotchannelcurvestylecontrol.h>
#include <menusectionwidget.h>
#include <monitorplotsettings.h>
#include <mousewheelwidgetguard.h>
#include <plotautoscaler.h>
#include <plotchannel.h>
#include <style.h>
#include <timemanager.hpp>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS, "DataMonitorSettings")

DataMonitorSettings::DataMonitorSettings(MonitorPlotManager *m_plotManager, QWidget *parent)
	: m_plotManager(m_plotManager)
	, QWidget{parent}
{
	mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);
}

DataMonitorSettings::~DataMonitorSettings() {}

void DataMonitorSettings::init(QString title, QColor color)
{
	header = new EditTextMenuHeader(title, color, this);
	mainLayout->addWidget(header);

	connect(header->lineEdit(), &QLineEdit::textChanged, this,
		[=, this]() { Q_EMIT titleUpdated(header->lineEdit()->text()); });

	settingsBody = new QWidget(this);
	layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	////// 7 segment settings ///////////////////
	sevenSegmentMonitorSettings = new SevenSegmentMonitorSettings(this);
	layout->addWidget(sevenSegmentMonitorSettings);

	/////// data logging /////////////////
	dataLoggingMenu = new DataLoggingMenu(this);
	layout->addWidget(dataLoggingMenu);

	///////////////// MonitorPlotSettings /////////////////

	QComboBox *plotSelectorCombo = new QComboBox(this);
	QStackedWidget *monitorPlotsSettings = new QStackedWidget(this);
	layout->addWidget(plotSelectorCombo);
	layout->addWidget(monitorPlotsSettings);

	// Add initial plot(s)
	QList<QPair<uint32_t, QString>> plotList = m_plotManager->plotList();
	for(const auto &pair : plotList) {
		plotSelectorCombo->addItem(pair.second, pair.first);
		MonitorPlot *plt = m_plotManager->plot(pair.first);
		if(plt) {
			MonitorPlotSettings *monitorPlotSettings = new MonitorPlotSettings(plt, this);
			if(plt != m_plotManager->plot()) {
				monitorPlotSettings->toggleDeleteButtonVisible(true);
			}
			monitorPlotsSettings->addWidget(monitorPlotSettings);
			connect(monitorPlotSettings, &MonitorPlotSettings::requestDeletePlot, this,
				[=](uint32_t uuid) { m_plotManager->removePlot(uuid); });
		}
	}
	if(plotSelectorCombo->count() > 0)
		monitorPlotsSettings->setCurrentIndex(0);

	connect(plotSelectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[=](int idx) { monitorPlotsSettings->setCurrentIndex(idx); });

	// Add new menu when a new plot is added
	connect(m_plotManager, &MonitorPlotManager::plotAdded, this, [=](uint32_t uuid) {
		MonitorPlot *plt = m_plotManager->plot(uuid);
		if(plt) {
			plotSelectorCombo->addItem(plt->name(), uuid);
			MonitorPlotSettings *monitorPlotSettings = new MonitorPlotSettings(plt, this);
			if(plt != m_plotManager->plot()) {
				monitorPlotSettings->toggleDeleteButtonVisible(true);
			}
			monitorPlotsSettings->addWidget(monitorPlotSettings);
			// Connect delete signal for new plots
			connect(monitorPlotSettings, &MonitorPlotSettings::requestDeletePlot, this,
				[=](uint32_t uuid) { m_plotManager->removePlot(uuid); });
		}
	});

	// Remove menu when a plot is removed
	connect(m_plotManager, &MonitorPlotManager::plotRemoved, this, [=](uint32_t uuid) {
		for(int i = 0; i < plotSelectorCombo->count(); ++i) {
			if(plotSelectorCombo->itemData(i).toUInt() == uuid) {
				plotSelectorCombo->removeItem(i);
				QWidget *w = monitorPlotsSettings->widget(i);
				monitorPlotsSettings->removeWidget(w);
				w->deleteLater();
				break;
			}
		}
		if(plotSelectorCombo->count() > 0 && plotSelectorCombo->currentIndex() >= plotSelectorCombo->count())
			plotSelectorCombo->setCurrentIndex(0);
	});

	// Keep plotSelectorCombo text in sync with plot name changes
	for(const auto &pair : plotList) {
		uint32_t uuid = pair.first;
		MonitorPlot *plt = m_plotManager->plot(uuid);
		if(plt) {
			QObject::connect(plt, &MonitorPlot::nameChanged, plotSelectorCombo,
					 [plotSelectorCombo, uuid](const QString &newName) {
						 for(int j = 0; j < plotSelectorCombo->count(); ++j) {
							 if(plotSelectorCombo->itemData(j).toUInt() == uuid) {
								 plotSelectorCombo->setItemText(j, newName);
								 break;
							 }
						 }
					 });
		}
	}

	/////// add plot ////////////////////////

	m_addPlotBtn = new QPushButton("ADD PLOT", this);
	Style::setStyle(m_addPlotBtn, style::properties::button::basicButton);

	connect(m_addPlotBtn, &QPushButton::clicked, m_plotManager, [=]() { m_plotManager->addPlot("Monitor Plot"); });
	layout->addWidget(m_addPlotBtn);

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("dataloggerplugin_add_remove_plot")) {
			bool en = p->get("dataloggerplugin_add_remove_plot").toDouble();
			setEnableAddRemovePlot(en);
		}
	});

	MouseWheelWidgetGuard *mouseWheelWidgetGuard = new MouseWheelWidgetGuard(this);
	mouseWheelWidgetGuard->installEventRecursively(this);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

void DataMonitorSettings::setEnableAddRemovePlot(bool en) { m_addPlotBtn->setVisible(en); }

DataLoggingMenu *DataMonitorSettings::getDataLoggingMenu() const { return dataLoggingMenu; }

SevenSegmentMonitorSettings *DataMonitorSettings::getSevenSegmentMonitorSettings() const
{
	return sevenSegmentMonitorSettings;
}

bool DataMonitorSettings::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::Wheel) {
		return true;
	}

	return QWidget::eventFilter(watched, event);
}

#include "moc_datamonitorsettings.cpp"
