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

#include "menus/monitorselectionmenu.hpp"

#include <QLabel>
#include <datamonitorstylehelper.hpp>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <style.h>
#include <datamonitor/readabledatamonitormodel.hpp>

using namespace scopy;
using namespace datamonitor;

MonitorSelectionMenu::MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QButtonGroup *monitorsGroup,
					   QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	QWidget *settingsBody = new QWidget(this);
	layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	deviceChannelsWidget = new QWidget(this);
	QVBoxLayout *deviceChannelsWidgetLayout = new QVBoxLayout(deviceChannelsWidget);
	deviceChannelsWidgetLayout->setMargin(0);
	deviceChannelsWidgetLayout->setSpacing(10);
	deviceChannelsWidget->setLayout(deviceChannelsWidgetLayout);

	importedChannelsWidget = new QWidget(this);
	QVBoxLayout *importedChannelsWidgetLayout = new QVBoxLayout(importedChannelsWidget);
	importedChannelsWidgetLayout->setMargin(0);
	importedChannelsWidgetLayout->setSpacing(10);
	importedChannelsWidget->setLayout(importedChannelsWidgetLayout);

	m_monitorsGroup = monitorsGroup;

	m_monitorsGroup = monitorsGroup;

	foreach(QString monitor, monitorList->keys()) {
		addMonitor(monitorList->value(monitor));
	}

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

	layout->addWidget(deviceChannelsWidget);
	layout->addItem(spacer);
	layout->addWidget(importedChannelsWidget);
}

void MonitorSelectionMenu::generateDeviceSection(QString device, bool import)
{
	CollapsableMenuControlButton *devMonitorsSection = new CollapsableMenuControlButton(this);
	devMonitorsSection->getControlBtn()->button()->setVisible(false);
	devMonitorsSection->getControlBtn()->setName(device);

	if(import) {

		QPushButton *removeBtn = new QPushButton(devMonitorsSection);
		removeBtn->setMaximumSize(25, 25);
		removeBtn->setIcon(Style::getPixmap(":/gui/icons/orange_close.svg",
						    Style::getColor(json::theme::content_inverse)));

		HoverWidget *removeHover = new HoverWidget(removeBtn, devMonitorsSection, devMonitorsSection);
		removeHover->setStyleSheet("background-color: transparent; border: 0px;");
		removeHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		removeHover->setContentPos(HoverPosition::HP_CENTER);
		removeHover->setAnchorOffset(QPoint(-20, 20));
		removeHover->setVisible(true);
		removeHover->raise();

		connect(removeBtn, &QPushButton::clicked, this,
			[=, this]() { Q_EMIT requestRemoveImportedDevice(device); });

		importedChannelsWidget->layout()->addWidget(devMonitorsSection);
	} else {
		deviceChannelsWidget->layout()->addWidget(devMonitorsSection);
	}

	devMonitorsSection->header()->setChecked(false);
	deviceMap.insert(device, devMonitorsSection);
}

void MonitorSelectionMenu::addMonitor(DataMonitorModel *monitor)
{
	if(!deviceMap.contains(monitor->getDeviceName())) {
		generateDeviceSection(monitor->getDeviceName(), !qobject_cast<ReadableDataMonitorModel *>(monitor));
	}

	MenuControlButton *monitorChannel = new MenuControlButton(deviceMap.value(monitor->getDeviceName()));
	monitorChannel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	deviceMap.value(monitor->getDeviceName())->add(monitorChannel);
	monitorChannel->setName(monitor->getShortName());
	monitorChannel->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	monitorChannel->setOpenMenuChecksThis(true);
	monitorChannel->setColor(monitor->getColor());
	monitorChannel->button()->setVisible(false);
	monitorChannel->setCheckable(true);

	monitorChannel->setToolTip(monitor->getName());

	m_monitorsGroup->addButton(monitorChannel);

	connect(monitorChannel, &MenuControlButton::clicked, this, [=, this](bool toggled) {
		if(!monitorChannel->checkBox()->isChecked()) {
			monitorChannel->checkBox()->setChecked(true);
		}
		Q_EMIT requestMonitorMenu(toggled, monitor->getName());
	});

	connect(monitorChannel->checkBox(), &QCheckBox::toggled, this,
		[=, this](bool toggled) { Q_EMIT monitorToggled(toggled, monitor->getName()); });

	// when removing the monitor disable all active monitors
	connect(this, &MonitorSelectionMenu::removeMonitor, monitorChannel, [=, this]() {
		if(monitorChannel->checkBox()->isChecked()) {
			Q_EMIT monitorToggled(false, monitor->getName());
		}
	});

	connect(this, &MonitorSelectionMenu::requestMonitorToggled, this, [=, this](bool toggled, QString monitorName) {
		if(monitorName == monitor->getName()) {
			monitorChannel->checkBox()->setChecked(toggled);
		}
	});
}

void MonitorSelectionMenu::removeDevice(QString device)
{
	if(deviceMap.contains(device)) {
		delete deviceMap.value(device);
		deviceMap.remove(device);
	}
}

QButtonGroup *MonitorSelectionMenu::monitorsGroup() const { return m_monitorsGroup; }

#include "moc_monitorselectionmenu.cpp"
