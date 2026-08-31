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

#include "iioutil/handles.h"

#include <QObject>
#include <QString>

namespace scopy::iio {
class IDeviceOps;
}

namespace scopy::component::iio {

// Trigger SOURCE marker: an IIO device where iio_device_is_trigger is true.
// IIO-only, no generic base — pure metadata (name()), no I/O. Carries the device
// handle so an IIOTriggerable can pass it to iio_device_set_trigger. Parented to
// its IIODevice; discovered via findChild<IIOTrigger*>().
class IIOTrigger : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name CONSTANT)
public:
	IIOTrigger(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, QObject *parent = nullptr);

	QString name() const;
	scopy::iio::DeviceHandle handle() const { return m_dev; }

private:
	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_dev;
};

} // namespace scopy::component::iio
