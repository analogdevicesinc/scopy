/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "customplotpositionbutton.h"
#include "ui_customplotpositionbutton.h"
#include "dynamicWidget.hpp"

using namespace adiscope;

CustomPlotPositionButton::CustomPlotPositionButton(QWidget *parent) :
	ColoredQWidget(parent),
	ui(new Ui::CustomPlotPositionButton)
{
	ui->setupUi(this);

	setDynamicProperty(ui->leftTop, "selected" , true);
	setDynamicProperty(ui->rightTop, "selected" , false);
	setDynamicProperty(ui->rightBot, "selected" , false);
	setDynamicProperty(ui->leftBot, "selected" , false);

	ui->leftTop->setChecked(true);

	btns = new QButtonGroup(this);

	connect(ui->leftTop, &QPushButton::toggled, [=](bool on){
		setDynamicProperty(ui->leftTop, "selected" , on);
		if (on)
			Q_EMIT positionChanged(topLeft);
	});
	connect(ui->rightTop, &QPushButton::toggled, [=](bool on){
		setDynamicProperty(ui->rightTop, "selected" , on);
		if (on)
			Q_EMIT positionChanged(topRight);
	});
	connect(ui->rightBot, &QPushButton::toggled, [=](bool on){
		setDynamicProperty(ui->rightBot, "selected" , on);
		if (on)
			Q_EMIT positionChanged(bottomRight);
	});
	connect(ui->leftBot, &QPushButton::toggled, [=](bool on){
		setDynamicProperty(ui->leftBot, "selected" , on);
		if (on)
			Q_EMIT positionChanged(bottomLeft);
	});

	btns->addButton(ui->leftBot);
	btns->addButton(ui->leftTop);
	btns->addButton(ui->rightBot);
	btns->addButton(ui->rightTop);
}

CustomPlotPositionButton::~CustomPlotPositionButton()
{
	delete ui;
}

void CustomPlotPositionButton::setPosition(ReadoutsPosition position)
{
	switch (position) {
	case topLeft:
	default:
		ui->leftTop->setChecked(true);
		break;
	case topRight:
		ui->rightTop->setChecked(true);
		break;
	case bottomLeft:
		ui->leftBot->setChecked(true);
		break;
	case bottomRight:
		ui->rightBot->setChecked(true);
		break;
	}
}
