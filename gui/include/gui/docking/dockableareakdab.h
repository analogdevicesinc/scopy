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

#ifndef DOCKABLEAREAKDAB_H
#define DOCKABLEAREAKDAB_H

// Avoid compiling this file if option not enabled
#ifdef USE_KDDOCKWIDGETS

#include "scopy-gui_export.h"
#include <QWidget>
#include <QList>
#include "docking/dockableareainterface.h"
#include "docking/dockwrapperinterface.h"
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

namespace scopy::kdab {
class SCOPY_GUI_EXPORT DockableArea : public KDDockWidgets::QtWidgets::MainWindow, public DockableAreaInterface
{
public:
	explicit DockableArea(QWidget *parent = nullptr);
	~DockableArea() override = default;

	// Does not set affinities
	void addDockWrapper(DockWrapperInterface *wrapper) override;

	// Preffered as it also sets affinities
	void setAllDockWrappers(const QList<DockWrapperInterface *> &wrappers) override;

private:
	static int s_dockableAreaId;
};
} // namespace scopy::kdab
#endif // USE_KDDOCKWIDGETS
#endif // DOCKABLEAREAKDAB_H
