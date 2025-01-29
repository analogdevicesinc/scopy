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

#ifndef DOCKWRAPPER_H
#define DOCKWRAPPER_H

#ifdef USE_KDDOCKWIDGETS
#include "docking/dockwrapperkdab.h"
#include "dockwrapperclassic.h"
#include "dockwrapperinterface.h"
#include <pluginbase/preferences.h>

namespace scopy {
using DockWrapper = kdab::DockWrapper;

// If you want the user to choose (preferences) what dockable area to use
static DockWrapperInterface *createDockWrapper(QString name)
{
	if(Preferences::get("general_use_docking_if_available").toBool()) {
		return new DockWrapper(name);
	}
	return new classic::DockWrapper(name);
}
} // namespace scopy
#else
#include "docking/dockwrapperclassic.h"
namespace scopy {
using DockWrapper = classic::DockWrapper;
static DockWrapperInterface *createDockWrapper(QString name) { return new DockWrapper(name); }
} // namespace scopy
#endif // USE_KDDOCKWIDGETS
#endif // DOCKWRAPPER_H
