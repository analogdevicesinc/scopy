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

void DockableArea::addDockWrapper(DockWrapperInterface *dockWrapper)
{
	KDDockWidgets::QtWidgets::DockWidget *dockWrapperWidget =
		dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dockWrapper);
	if(dockWrapperWidget) {
		addDockWidget(dockWrapperWidget, KDDockWidgets::Location_OnRight);
	} else {
		qWarning() << "Cannot cast dockWrapperInterface to DockWidget";
	}
}

void DockableArea::setAllDockWrappers(const QList<DockWrapperInterface *> &wrappers)
{
	QString affinitiesName = uniqueName(); // Set this uname as affinity to all DockWidgets
	for(DockWrapperInterface *dockWrapper : wrappers) {
		KDDockWidgets::QtWidgets::DockWidget *dockWrapperWidget =
			dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dockWrapper);
		if(dockWrapperWidget) {
			dockWrapperWidget->setAffinities({affinitiesName});
			addDockWidget(dockWrapperWidget, KDDockWidgets::Location_OnRight);
		} else {
			qWarning() << "Cannot cast dockWrapperInterface to DockWidget";
		}
	}
}

#endif // USE_KDDOCKWIDGETS
