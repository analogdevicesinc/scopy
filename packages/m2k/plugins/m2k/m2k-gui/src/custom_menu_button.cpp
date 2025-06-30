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

#include "custom_menu_button.hpp"

#include "ui_custom_menu_button.h"

using namespace scopy::m2kgui;

CustomMenuButton::CustomMenuButton(QString labelText, bool checkboxVisible, bool checkBoxChecked, QWidget *parent)
	: CustomMenuButton(parent)
{

	m_ui->lblCustomMenuButton->setText(labelText);
	m_ui->checkBoxCustomMenuButton->setVisible(checkboxVisible);

	if(checkboxVisible) {
		checkBoxToggled(checkBoxChecked);
		m_ui->checkBoxCustomMenuButton->setChecked(checkBoxChecked);
	}
}

CustomMenuButton::CustomMenuButton(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::CustomMenuButton)
	, m_floatingMenu(false)
{
	m_ui->setupUi(this);
	connect(m_ui->checkBoxCustomMenuButton, &QCheckBox::toggled, this, &CustomMenuButton::checkBoxToggled);
}

CustomMenuButton::~CustomMenuButton() { delete m_ui; }

void CustomMenuButton::setLabel(const QString &text) { m_ui->lblCustomMenuButton->setText(text); }

void CustomMenuButton::setCheckboxVisible(bool visible) { m_ui->checkBoxCustomMenuButton->setVisible(visible); }

CustomPushButton *CustomMenuButton::getBtn() { return m_ui->btnCustomMenuButton; }

QCheckBox *CustomMenuButton::getCheckBox() { return m_ui->checkBoxCustomMenuButton; }

bool CustomMenuButton::getCheckBoxState() { return getCheckBox()->isChecked(); }

void CustomMenuButton::setCheckBoxState(bool checked) { m_ui->checkBoxCustomMenuButton->setChecked(checked); }

void CustomMenuButton::setMenuFloating(bool floating) { m_floatingMenu = floating; }

void CustomMenuButton::checkBoxToggled(bool toggled)
{
	if(!toggled) {
		m_ui->btnCustomMenuButton->setChecked(false);
	}

	if(!m_floatingMenu) {
		m_ui->btnCustomMenuButton->setEnabled(toggled);
	}
}

#include "moc_custom_menu_button.cpp"
