/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "diosettingstab.h"

#include "ui_swiotmax14906settingstab.h"

using namespace scopy::swiot;

DioSettingsTab::DioSettingsTab(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DioSettingsTab)
{
	ui->setupUi(this);

	this->m_max_spin_button = new PositionSpinButton({{tr("s"), 1}}, tr("Timespan"), 1, 300, true, false, this);
	this->m_max_spin_button->setValue(10); // default value
	this->ui->verticalLayout_maxTimer->addWidget(this->m_max_spin_button);

	QObject::connect(m_max_spin_button, &PositionSpinButton::valueChanged,
			 [this]() { Q_EMIT timeValueChanged(this->m_max_spin_button->value()); });
}

DioSettingsTab::~DioSettingsTab() { delete ui; }

double DioSettingsTab::getTimeValue() const { return m_max_spin_button->value(); }
