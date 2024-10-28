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

#include "compositeheaderwidget.h"

using namespace scopy;

CompositeHeaderWidget::CompositeHeaderWidget(QString title, QWidget *parent)
	: BaseHeaderWidget(title, parent)
{}

CompositeHeaderWidget::~CompositeHeaderWidget() {}

void CompositeHeaderWidget::add(QWidget *w)
{
	int titleIndex = m_lay->indexOf(m_label);
	m_lay->insertWidget(titleIndex + 1, w);
}

void CompositeHeaderWidget::remove(QWidget *w) { m_lay->removeWidget(w); }

#include "moc_compositeheaderwidget.cpp"
