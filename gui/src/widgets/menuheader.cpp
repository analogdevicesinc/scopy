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

#include "qlineedit.h"
#include <stylehelper.h>
#include <widgets/menuheader.h>

using namespace scopy;

MenuHeaderWidget::MenuHeaderWidget(QString title, QPen pen, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(3);
	//		lay->setContentsMargins(3,3,3,3);

	m_title = new QLineEdit(title, this);
	m_title->setEnabled(false);
	m_title->setReadOnly(false);

	m_line = new QFrame(this);
	m_pen = pen;

	lay->addWidget(m_title);
	lay->addWidget(m_line);
	applyStylesheet();
}

MenuHeaderWidget::~MenuHeaderWidget() {}

QLineEdit *MenuHeaderWidget::title() { return m_title; }

void MenuHeaderWidget::applyStylesheet()
{
	StyleHelper::MenuCollapseHeaderLineEdit(m_title, "menuLabel");
	StyleHelper::MenuHeaderLine(m_line, m_pen, "menuSeparator");
	StyleHelper::MenuHeaderWidget(this, "menuHeader");
}

#include "moc_menuheader.cpp"
