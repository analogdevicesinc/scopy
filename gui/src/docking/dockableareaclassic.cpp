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
#include <QHBoxLayout>
#include <QDebug>

using namespace scopy::classic;

DockableArea::DockableArea(QWidget *parent)
	: QWidget(parent)
{
	init();
}

void DockableArea::addDockWrapper(DockWrapperInterface *dockWrapper)
{
	QWidget *dockWrapperWidget = dynamic_cast<QWidget *>(dockWrapper);
	if(dockWrapperWidget) {
		layout()->addWidget(dockWrapperWidget);
	} else {
		qWarning() << "Cannot cast dockWrapperInterface to QWidget*";
	}
}

void DockableArea::setAllDockWrappers(const QList<DockWrapperInterface *> &wrappers)
{
	for(DockWrapperInterface *dockWrapper : wrappers) {
		addDockWrapper(dockWrapper);
	}
}

void DockableArea::init()
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
}
