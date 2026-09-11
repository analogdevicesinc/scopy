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

#include "core/resultcommand.h"
#include "iioutil/ibufferops.h"

#include <cerrno>
#include "iioutil/controller_iioutils_export.h"

namespace scopy::iio {

class CONTROLLER_IIOUTILS_EXPORT BufferPushCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	BufferPushCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{}

protected:
	void run() override { setResult(m_ops->push(m_handle)); }

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
