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
#include "dynamic_widget.hpp"

#include "ui_custom_plot_position_button.h"

#include <scopy/gui/custom_plot_position_button.hpp>

using namespace scopy::gui;

CustomPlotPositionButton::CustomPlotPositionButton(QWidget* parent)
	: ColoredQWidget(parent)
	, m_ui(new Ui::CustomPlotPositionButton)
{
	m_ui->setupUi(this);

	DynamicWidget::setDynamicProperty(m_ui->btnLeftTop, "selected", true);
	DynamicWidget::setDynamicProperty(m_ui->btnRightTop, "selected", false);
	DynamicWidget::setDynamicProperty(m_ui->btnRightBot, "selected", false);
	DynamicWidget::setDynamicProperty(m_ui->btnLeftBot, "selected", false);

	m_ui->btnLeftTop->setChecked(true);

	m_btns = new QButtonGroup(this);

	connect(m_ui->btnLeftTop, &QPushButton::toggled, [=](bool on) {
		DynamicWidget::setDynamicProperty(m_ui->btnLeftTop, "selected", on);
		if (on)
			Q_EMIT positionChanged(topLeft);
	});
	connect(m_ui->btnRightTop, &QPushButton::toggled, [=](bool on) {
		DynamicWidget::setDynamicProperty(m_ui->btnRightTop, "selected", on);
		if (on)
			Q_EMIT positionChanged(topRight);
	});
	connect(m_ui->btnRightBot, &QPushButton::toggled, [=](bool on) {
		DynamicWidget::setDynamicProperty(m_ui->btnRightBot, "selected", on);
		if (on)
			Q_EMIT positionChanged(bottomRight);
	});
	connect(m_ui->btnLeftBot, &QPushButton::toggled, [=](bool on) {
		DynamicWidget::setDynamicProperty(m_ui->btnLeftBot, "selected", on);
		if (on)
			Q_EMIT positionChanged(bottomLeft);
	});

	m_btns->addButton(m_ui->btnLeftBot);
	m_btns->addButton(m_ui->btnLeftTop);
	m_btns->addButton(m_ui->btnRightBot);
	m_btns->addButton(m_ui->btnRightTop);
}

CustomPlotPositionButton::~CustomPlotPositionButton() { delete m_ui; }

void CustomPlotPositionButton::setPosition(ReadoutsPosition position)
{
	switch (position) {
	case topLeft:
	default:
		m_ui->btnLeftTop->setChecked(true);
		break;
	case topRight:
		m_ui->btnRightTop->setChecked(true);
		break;
	case bottomLeft:
		m_ui->btnLeftBot->setChecked(true);
		break;
	case bottomRight:
		m_ui->btnRightBot->setChecked(true);
		break;
	}
}
