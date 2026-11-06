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

#include "component/registerreader.h"
#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QUuid>
#include <cstdint>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IDeviceOps;
class RegReadCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

// Register-read capability of an IIO device (debug register access), 32-bit address and value.
// Attaches to the IIODevice; discovered via findChild<RegisterReader*>() (or the IIO type).
class COMPONENT_EXPORT IIORegisterReader : public RegisterReader
{
	Q_OBJECT
public:
	IIORegisterReader(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle, scopy::ICmdExecutor *executor,
			  QObject *parent = nullptr);

	QCoro::Task<CommandResponse<uint32_t>> readAsync(uint32_t addr) override;

private:
	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
