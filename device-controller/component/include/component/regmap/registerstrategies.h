/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 *
 */

#pragma once

#include "component/regmap/iregisterreadstrategy.h"
#include "component/regmap/iregisterwritestrategy.h"

#include <QPointer>
#include <QString>

#include <cstdint>
#include "component/component_export.h"

namespace scopy::component {

class Device;

// A read and a write strategy, owned by the device controller. Either can be null.
struct RegisterStrategies
{
	QPointer<IRegisterReadStrategy> read;
	QPointer<IRegisterWriteStrategy> write;
};

// The register strategies of a device, for one address space (OR-ed into the device call only).
// They are children of the device, one pair per address space, created once and then reused.
// Empty if the device has no register access.
COMPONENT_EXPORT RegisterStrategies registerStrategies(Device *device, uint32_t addressSpace = 0);

// Strategies on a register values file ("<address>,<value>" lines), one pair per path, created once and then
// reused. They are owned by the device controller and live until it goes.
COMPONENT_EXPORT RegisterStrategies fileRegisterStrategies(const QString &path);

} // namespace scopy::component
