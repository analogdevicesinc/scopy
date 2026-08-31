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
#include <QString>

namespace scopy::iio {

class IContextOps
{
public:
	virtual ~IContextOps() = default;

	virtual ContextHandle createContext(const QString &uri, const ContextParams &params = {}) = 0;
	virtual void destroyContext(ContextHandle ctx) = 0;
	virtual QString name(ContextHandle ctx) const = 0;
	virtual QString description(ContextHandle ctx) const = 0;
	virtual void version(ContextHandle ctx, unsigned int &major, unsigned int &minor, QString &tag) const = 0;
	virtual ContextVersion getVersion(ContextHandle ctx) const = 0;
	virtual unsigned int devicesCount(ContextHandle ctx) const = 0;
	virtual DeviceHandle getDevice(ContextHandle ctx, unsigned int index) const = 0;
	virtual DeviceHandle findDevice(ContextHandle ctx, const QString &name) const = 0;
	virtual unsigned int attrsCount(ContextHandle ctx) const = 0;
	virtual QString attrName(ContextHandle ctx, unsigned int index) const = 0;

	// Returns true if the context is still reachable, false if the connection is lost.
	virtual bool ping(ContextHandle ctx) const = 0;
};

} // namespace scopy::iio
