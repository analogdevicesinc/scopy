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

#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QUuid>
#include <cstdint>

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IDeviceOps;
class RegWriteCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

// Register-write capability of a device (debug register access). IIO-only, no
// generic base — addressed by number, attaches straight to the IIODevice
// (discovered via findChild<IIORegisterWriter*>()).
class IIORegisterWriter : public QObject
{
	Q_OBJECT
public:
	IIORegisterWriter(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle, scopy::ICmdExecutor *executor,
			  QObject *parent = nullptr);

	Q_INVOKABLE QCoro::Task<CommandResponse<void>> writeAsync(uint32_t addr, uint32_t value);

Q_SIGNALS:
	void writeSucceeded(uint32_t addr);
	void writeFailed(const scopy::Error &error);

private:
	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
