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

#include <customSwitch.h>
#include <smallOnOffSwitch.h>
#include <style.h>
#include <widgets/menuonoffswitch.h>

using namespace scopy;

MenuOnOffSwitch::MenuOnOffSwitch(QString title, QWidget *parent, bool medium)
	: QWidget(parent)
{
	auto lay = new QHBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_label = new QLabel(title, this);
	if(medium == false) {
		m_switch = new SmallOnOffSwitch(this);
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(m_switch), "menuButton");
		StyleHelper::MenuSmallLabel(m_label, "menuLabel");
	} else {
		m_switch = new CustomSwitch(this);
		Style::setStyle(m_label, style::properties::label::menuMedium);
	}

	lay->addWidget(m_label);
	lay->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(m_switch);
	applyStylesheet();
}

MenuOnOffSwitch::~MenuOnOffSwitch() {}

QAbstractButton *MenuOnOffSwitch::onOffswitch() { return m_switch; }

void MenuOnOffSwitch::applyStylesheet() { StyleHelper::MenuOnOffSwitch(this, "menuOnOffSwitch"); }

#include "moc_menuonoffswitch.cpp"
