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

// Avoid compiling this file if option not enabled
#include "dockableareainterface.h"
#ifdef USE_KDDOCKWIDGETS

#include "docking/dockableareakdab.h"
#include "docking/dockwrapperinterface.h"

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>
#include <kddockwidgets/KDDockWidgets.h>

using namespace scopy::kdab;

int DockableArea::s_dockableAreaId{0};

DockableArea::DockableArea(QWidget *parent)
	: KDDockWidgets::QtWidgets::MainWindow("DockableArea" + QString::number(s_dockableAreaId++))
{
	setAffinities({uniqueName()});
}

void DockableArea::addDockWrapper(DockWrapperInterface *dockWrapper, Direction direction)
{
	KDDockWidgets::QtWidgets::DockWidget *dockWrapperWidget =
		dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dockWrapper);
	dockWrapperWidget->setAffinities({uniqueName()});
	if(dockWrapperWidget) {
		addDockWidget(dockWrapperWidget, translateLocation(direction));
	} else {
		qWarning() << "Cannot cast dockWrapperInterface to DockWidget";
	}
}

KDDockWidgets::Location DockableArea::translateLocation(Direction direction)
{
	switch(direction) {
	case DockableAreaInterface::Direction_LEFT:
		return KDDockWidgets::Location::Location_OnLeft;
	case DockableAreaInterface::Direction_RIGHT:
		return KDDockWidgets::Location::Location_OnRight;
	case DockableAreaInterface::Direction_TOP:
		return KDDockWidgets::Location::Location_OnTop;
	case DockableAreaInterface::Direction_BOTTOM:
		return KDDockWidgets::Location::Location_OnBottom;
	}
	return KDDockWidgets::Location::Location_OnRight;
}

#endif // USE_KDDOCKWIDGETS
