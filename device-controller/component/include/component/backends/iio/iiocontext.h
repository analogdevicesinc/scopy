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

#include "component/backends/iio/iioconnection.h"
#include "component/context.h"

#include "component/component_export.h"

namespace scopy::component::iio {

// Root of the generic IIO tree. Handle, backend and version live on the IIOConnection; the
// accessors here forward to it once the view is attached.
class COMPONENT_EXPORT IIOContext : public Context
{
	Q_OBJECT
public:
	using Version = IIOConnection::Version;

	explicit IIOContext(QObject *parent = nullptr)
		: Context(parent)
	{}
	~IIOContext() override = default;

	IIOConnection *iioConnection() const { return connection() ? connection()->as<IIOConnection>() : nullptr; }
	scopy::iio::ContextHandle handle() const
	{
		return iioConnection() ? iioConnection()->handle() : scopy::iio::ContextHandle{};
	}
	scopy::iio::IBackend *iioBackend() const { return iioConnection() ? iioConnection()->backend() : nullptr; }
	Version version() const { return iioConnection() ? iioConnection()->version() : Version{}; }
};

} // namespace scopy::component::iio
