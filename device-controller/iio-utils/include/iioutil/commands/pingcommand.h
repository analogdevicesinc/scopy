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
#include "iioutil/handles.h"

#include <cerrno>
#include <functional>
#include "iioutil/controller_iioutils_export.h"

namespace scopy::iio {

class CONTROLLER_IIOUTILS_EXPORT PingCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	PingCommand(ContextHandle handle, std::function<bool()> fn, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_fn(std::move(fn))
	{}

protected:
	void run() override
	{
		setResult(m_fn() ? Result<void>()
				 : Result<void>(Unexpected{Error{-ENODEV, QStringLiteral("ping failed")}}));
	}

private:
	std::function<bool()> m_fn;
};

} // namespace scopy::iio
