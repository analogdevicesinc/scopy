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

#include "docking/dockwrapperkdab.h"

#include <QHBoxLayout>
#include <kddockwidgets/Config.h>
#include <kddockwidgets/KDDockWidgets.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/core/Group.h>
#include <kddockwidgets/core/TitleBar.h>
#include <kddockwidgets/core/TabBar.h>
#include <kddockwidgets/core/MainWindow.h>

using namespace scopy::kdab;

int DockWrapper::s_dockWrapperId{0};

DockWrapper::DockWrapper(QString name, QWidget *parent)
	: KDDockWidgets::QtWidgets::DockWidget(name + QString::number(s_dockWrapperId++),
					       KDDockWidgets::DockWidgetOption_NotClosable)
{
	this->setTitle(name); // Not unique name
	// HACKISH: This connect is required as the DockWrapper cannot be closed (hidden) until it is
	// added to a layout (and gets a parent). So any call to setActivated has not effect. This is
	// just to propagate the effect set before the layouting.
	m_isActive = true;
	connect(this, &DockWrapper::isFloatingChanged, this, [this]() { setActivated(m_isActive); });
}

void DockWrapper::setInnerWidget(QWidget *innerWidget)
{
	m_innerWidget = innerWidget;
	KDDockWidgets::QtWidgets::DockWidget::setWidget(m_innerWidget);
}

QWidget *DockWrapper::innerWidget() const { return m_innerWidget; }

void DockWrapper::setActivated(bool isActivated)
{
	m_isActive = isActivated;
	if(isActivated) {
		KDDockWidgets::QtWidgets::DockWidget::open();
		KDDockWidgets::Core::MainWindow *mainWindow = asDockWidgetController()->mainWindow();
		if(mainWindow) {
			mainWindow->layoutEqually();
		}
	} else {
		KDDockWidgets::QtWidgets::DockWidget::close();
	}
}

#endif // USE_KDDOCKWIDGETS
