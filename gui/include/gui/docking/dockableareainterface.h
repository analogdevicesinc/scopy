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

#ifndef DOCKABLEAREAINTERFACE_H
#define DOCKABLEAREAINTERFACE_H

#include "docking/dockwrapperinterface.h"
#include "scopy-gui_export.h"
#include <QObject>

namespace scopy {
class SCOPY_GUI_EXPORT DockableAreaInterface
{
public:
	enum Direction
	{
		Direction_LEFT,
		Direction_RIGHT,
		Direction_TOP,
		Direction_BOTTOM,
	};

	virtual ~DockableAreaInterface() = default;
	virtual void addDockWrapper(DockWrapperInterface *, Direction = Direction_RIGHT) = 0;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::DockableAreaInterface, "scopy::DockableAreaInterface")
#endif // DOCKABLEAREAINTERFACE_H
