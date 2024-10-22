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

#include "edittextmenuheader.h"

#include <stylehelper.h>

using namespace scopy;

scopy::EditTextMenuHeader::EditTextMenuHeader(QString title, QPen pen, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(3);

	m_lineEdit = new QLineEdit(title, this);
	m_line = new QFrame(this);
	m_pen = pen;

	lay->addWidget(m_lineEdit);
	lay->addWidget(m_line);
	applyStylesheet();
}

scopy::EditTextMenuHeader::~EditTextMenuHeader() {}

QLineEdit *EditTextMenuHeader::lineEdit() const { return m_lineEdit; }

void EditTextMenuHeader::applyStylesheet()
{
	StyleHelper::MenuHeaderLine(m_line, m_pen, "menuSeparator");
	StyleHelper::MenuEditTextHeaderWidget(this, "menuHeader");
}

#include "moc_edittextmenuheader.cpp"
