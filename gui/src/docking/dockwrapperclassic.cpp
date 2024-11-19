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
 */

#include "docking/dockwrapperclassic.h"
#include <QHBoxLayout>

using namespace scopy::classic;

DockWrapper::DockWrapper(QString name, QWidget *parent)
	: QWidget(parent)
{
	init();
}

void DockWrapper::setInnerWidget(QWidget *innerWidget)
{
	m_innerWidget = innerWidget;
	layout()->addWidget(m_innerWidget);
}

QWidget *DockWrapper::innerWidget() const { return m_innerWidget; }

void DockWrapper::init()
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
}

void DockWrapper::setActivated(bool isActivated) { setVisible(isActivated); }
