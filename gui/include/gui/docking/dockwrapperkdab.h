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

#ifndef DOCKWRAPPERKDAB_H
#define DOCKWRAPPERKDAB_H

// Avoid compiling this file if option not enabled
#ifdef USE_KDDOCKWIDGETS

#include "scopy-gui_export.h"
#include <QWidget>
#include "docking/dockwrapperinterface.h"
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

namespace scopy::kdab {
class SCOPY_GUI_EXPORT DockWrapper : public KDDockWidgets::QtWidgets::DockWidget, public DockWrapperInterface
{
public:
	explicit DockWrapper(QString name, QWidget *parent = nullptr);

	void setInnerWidget(QWidget *innerWidget) override;
	QWidget *innerWidget() const override;
	void setActivated(bool isActive) override;

private:
	static int s_dockWrapperId;
	bool m_isActive;
	QWidget *m_innerWidget;
};
} // namespace scopy::kdab

#endif // USE_KDDOCKWIDGETS
#endif // DOCKWRAPPERKDAB_H
