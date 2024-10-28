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

#include <baseheaderwidget.h>
#include <stylehelper.h>

using namespace scopy;

BaseHeaderWidget::BaseHeaderWidget(QString title, QWidget *parent)
	: QWidget(parent)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_label = new QLineEdit(title, this);
	m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_label->setEnabled(false);
	m_label->setReadOnly(false);
	StyleHelper::MenuCollapseHeaderLineEdit(m_label, "menuCollapseLabel");
	m_label->setTextMargins(0, 0, 0, 0);

	m_lay->addWidget(m_label);
}

BaseHeaderWidget::~BaseHeaderWidget() {}

void BaseHeaderWidget::setTitle(QString title) { m_label->setText(title); }

QString BaseHeaderWidget::title() { return m_label->text(); }

#include "moc_baseheaderwidget.cpp"
