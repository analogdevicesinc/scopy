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

#include "swiotinfopage.h"

#include <QWidget>

using namespace scopy::swiotrefactor;

SwiotInfoPage::SwiotInfoPage(QWidget *parent)
	: InfoPage(parent)
{
	// not enough attributes for correct display with normal size policies, there is too much spacing without this
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->initTempCheckBox();
	this->layout()->addWidget(m_enTempReadCheckBox);
	this->setAdvancedMode(true);
}

void SwiotInfoPage::enableTemperatureReadBtn(bool enable)
{
	m_enTempReadCheckBox->setEnabled(enable);
	m_enTempReadCheckBox->setVisible(enable);
}

void SwiotInfoPage::initTempCheckBox()
{
	m_enTempReadCheckBox = new QCheckBox("Enable temperature read");
	m_enTempReadCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(m_enTempReadCheckBox, &QCheckBox::toggled, this, &SwiotInfoPage::temperatureReadEnabled);
	m_enTempReadCheckBox->setEnabled(false);
	m_enTempReadCheckBox->setVisible(true);
	m_enTempReadCheckBox->setChecked(true);
}

#include "moc_swiotinfopage.cpp"
