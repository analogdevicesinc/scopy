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

#include "component/device.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// IIO device identity. Carries the libiio device handle on top of the shared
// Device identity (id/name/label).
class IIODevice : public Device
{
	Q_OBJECT
public:
	explicit IIODevice(QObject *parent = nullptr)
		: Device(parent)
	{}

	scopy::iio::DeviceHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::DeviceHandle handle) { m_handle = handle; }

private:
	scopy::iio::DeviceHandle m_handle;
};

} // namespace scopy::component::iio
