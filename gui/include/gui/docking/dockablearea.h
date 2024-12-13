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

#ifndef DOCKABLEAREA_H
#define DOCKABLEAREA_H

#ifdef USE_KDDOCKWIDGETS
#include "docking/dockableareakdab.h"
#include "docking/dockableareaclassic.h"
#include "docking/dockableareainterface.h"
#include <pluginbase/preferences.h>

namespace scopy {
using DockableArea = kdab::DockableArea;

// If you want the user to choose (preferences) what dockable area to use
static DockableAreaInterface *createDockableArea(QWidget *parent = nullptr)
{
	if(Preferences::get("general_use_docking_if_available").toBool()) {
		return new DockableArea(parent);
	}
	return new classic::DockableArea(parent);
}
} // namespace scopy
#else
#include "docking/dockableareaclassic.h"
namespace scopy {
using DockableArea = classic::DockableArea;
static DockableAreaInterface *createDockableArea(QWidget *parent = nullptr) { return new DockableArea(parent); }
} // namespace scopy
#endif

#endif // DOCKABLEAREA_H
