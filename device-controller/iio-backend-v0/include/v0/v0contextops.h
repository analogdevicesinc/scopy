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

#include "iioutil/icontextops.h"

namespace scopy::iio {

class V0ContextOps : public IContextOps
{
public:
	ContextHandle createContext(const QString &uri, const ContextParams &params = {}) override;
	void destroyContext(ContextHandle ctx) override;
	QString name(ContextHandle ctx) const override;
	QString description(ContextHandle ctx) const override;
	void version(ContextHandle ctx, unsigned int &major, unsigned int &minor, QString &tag) const override;
	ContextVersion getVersion(ContextHandle ctx) const override;
	unsigned int devicesCount(ContextHandle ctx) const override;
	DeviceHandle getDevice(ContextHandle ctx, unsigned int index) const override;
	DeviceHandle findDevice(ContextHandle ctx, const QString &name) const override;
	unsigned int attrsCount(ContextHandle ctx) const override;
	QString attrName(ContextHandle ctx, unsigned int index) const override;

	bool ping(ContextHandle ctx) const override;
};

} // namespace scopy::iio
