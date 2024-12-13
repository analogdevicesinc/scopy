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

#ifndef DOCKABLEAREACLASSIC_H
#define DOCKABLEAREACLASSIC_H

#include "scopy-gui_export.h"
#include <QWidget>
#include "docking/dockableareainterface.h"

namespace scopy::classic {
class SCOPY_GUI_EXPORT DockableArea : public QWidget, public DockableAreaInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DockableAreaInterface)
public:
	explicit DockableArea(QWidget *parent = nullptr);
	~DockableArea() override = default;

	void addDockWrapper(DockWrapperInterface *wrapper, Direction direction = Direction_RIGHT) override;

private:
	void init(Direction direction);
	bool m_isInitialized;
};
} // namespace scopy::classic

#endif // DOCKABLEAREACLASSIC_H
