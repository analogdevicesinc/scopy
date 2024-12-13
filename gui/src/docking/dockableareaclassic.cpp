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

#include "docking/dockableareaclassic.h"
#include "docking/dockableareainterface.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QBoxLayout>

using namespace scopy::classic;

DockableArea::DockableArea(QWidget *parent)
	: QWidget(parent)
{
	// Simulate lazy init in order to decide de layout type (V or H)
	m_isInitialized = false;
}

void DockableArea::addDockWrapper(DockWrapperInterface *dockWrapper, Direction direction)
{
	if(!m_isInitialized) {
		init(direction);
	}

	QWidget *dockWrapperWidget = dynamic_cast<QWidget *>(dockWrapper);
	if(dockWrapperWidget) {
		if(direction == Direction_RIGHT || direction == Direction_BOTTOM) {
			layout()->addWidget(dockWrapperWidget);
		} else {
			QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout *>(layout());
			if(vlayout) {
				vlayout->insertWidget(0, dockWrapperWidget);
			} else {
				QHBoxLayout *hlayout = dynamic_cast<QHBoxLayout *>(layout());
				hlayout->insertWidget(0, dockWrapperWidget);
			}
		}
	} else {
		qWarning() << "Cannot cast dockWrapperInterface to QWidget*";
	}
}

void DockableArea::init(Direction direction)
{
	if(direction == Direction_LEFT || direction == Direction_RIGHT) {
		setLayout(new QHBoxLayout(this));
	} else {
		setLayout(new QVBoxLayout(this));
	}
	layout()->setContentsMargins(0, 0, 0, 0);
}
