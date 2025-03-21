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

#include "datamonitor/sevensegmentdisplay.hpp"

#include <QLabel>
#include <QScrollArea>
#include <datamonitorstylehelper.hpp>

using namespace scopy::datamonitor;

SevenSegmentDisplay::SevenSegmentDisplay(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	QWidget *mainContainer = new QWidget();
	QVBoxLayout *mainContainerLayout = new QVBoxLayout();
	mainContainerLayout->setMargin(0);
	mainContainerLayout->setSpacing(10);
	mainContainer->setLayout(mainContainerLayout);

	QWidget *widgetBody = new QWidget(this);
	layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	widgetBody->setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(mainContainer);
	mainLayout->addWidget(scrollArea);

	mainContainerLayout->addWidget(widgetBody);
	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	mainContainerLayout->addItem(spacer);

	monitorList = new QMap<QString, SevenSegmentMonitor *>();
}

SevenSegmentDisplay::~SevenSegmentDisplay() {}

void SevenSegmentDisplay::generateSegment(DataMonitorModel *model)
{
	if(!monitorList->contains(model->getName())) {
		SevenSegmentMonitor *monitor = new SevenSegmentMonitor(model, this);
		layout->addWidget(monitor);
		monitorList->insert(model->getName(), monitor);
	}
}

void SevenSegmentDisplay::removeSegment(QString monitor)
{
	if(monitorList->contains(monitor)) {
		delete monitorList->value(monitor);
		monitorList->remove(monitor);
	}
}

void SevenSegmentDisplay::updatePrecision(int precision)
{
	foreach(auto monitor, monitorList->keys()) {
		monitorList->value(monitor)->updatePrecision(precision);
	}
}

void SevenSegmentDisplay::togglePeakHolder(bool toggle)
{
	foreach(auto monitor, monitorList->keys()) {
		monitorList->value(monitor)->togglePeakHolder(toggle);
	}
}

#include "moc_sevensegmentdisplay.cpp"
