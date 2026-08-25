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

#ifndef SWIOTDEVICEIMPL_H
#define SWIOTDEVICEIMPL_H

#include "iiodeviceimpl.h"

namespace scopy {

/**
 * @brief SWIOT-specific IIO device.
 *
 * SWIOT switches its context in place: applying a configuration writes the `mode`
 * attribute and reconnects the same uri to a fresh iio_context (config -> runtime).
 * The device object persists across that switch, so the component tree built at ctor
 * time would go stale. This impl rebuilds the component context on every connect and
 * drops it on every disconnect, so each connect reflects the current mode.
 */
class SCOPY_CORE_EXPORT SWIOTDeviceImpl : public IIODeviceImpl
{
	Q_OBJECT
public:
	explicit SWIOTDeviceImpl(QString param, QObject *parent = nullptr)
		: IIODeviceImpl(param, parent)
	{}
	~SWIOTDeviceImpl() {}

public Q_SLOTS:
	void connectDev() override;
	void disconnectDev() override;
};

} // namespace scopy

#endif // SWIOTDEVICEIMPL_H
