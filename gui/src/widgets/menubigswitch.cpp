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

#include "widgets/menubigswitch.h"

using namespace scopy;

MenuBigSwitch::MenuBigSwitch(QString on, QString off, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QHBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_switch = new CustomSwitch(this);
	m_switch->setOn(on);
	m_switch->setOff(off);

	lay->addWidget(m_switch);
	applyStylesheet();
}

MenuBigSwitch::~MenuBigSwitch() {}

CustomSwitch *MenuBigSwitch::onOffswitch() { return m_switch; }

void MenuBigSwitch::applyStylesheet()
{
	StyleHelper::MenuBigSwitchButton(m_switch, "menuButton");
	StyleHelper::MenuBigSwitch(this, "menuBigSwitch");
}

#include "moc_menubigswitch.cpp"
