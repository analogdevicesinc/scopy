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

#include "menu_header.hpp"

#include "ui_menu_header.h"

#include <QColor>
#include <QIcon>

using namespace scopy::m2kgui;

MenuHeader::MenuHeader(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::MenuHeader)
{
	m_ui->setupUi(this);
	this->setEnableBtnVisible(false);
}

MenuHeader::MenuHeader(const QString &label, const QColor *color, bool enableBtnVisible, QWidget *parent)
	: MenuHeader(parent)
{
	this->setLabel(label);
	this->setLineColor(color);
	this->setEnableBtnVisible(enableBtnVisible);
}

MenuHeader::~MenuHeader() { delete m_ui; }

void MenuHeader::setEnabledBtnState(bool state) { m_ui->btnEnabled->setChecked(state); }

void MenuHeader::setLabel(const QString &text) { m_ui->lblTitle->setText(text); }

void MenuHeader::setLineColor(const QColor *color)
{
	m_ui->lineSeparator->setStyleSheet("border: 2px solid " + color->name());
}

void MenuHeader::setEnableBtnVisible(bool visible) { m_ui->btnEnabled->setVisible(visible); }

QPushButton *MenuHeader::getEnableBtn() { return m_ui->btnEnabled; }

void MenuHeader::addNewHeaderWidget(QWidget *widget)
{
	m_ui->stackedWidget->addWidget(widget);
	m_ui->stackedWidget->setCurrentWidget(widget);
}

#include "moc_menu_header.cpp"
