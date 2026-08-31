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
#include "core/result.h"
#include <QStringList>

namespace scopy::iio {

class IAttrOps
{
public:
	virtual ~IAttrOps() = default;

	// Handle lifecycle — called during buildTree()
	virtual AttrHandle contextAttr(ContextHandle ctx, const QString &name) = 0;
	virtual AttrHandle deviceAttr(DeviceHandle dev, const QString &name) = 0;
	virtual AttrHandle debugAttr(DeviceHandle dev, const QString &name) = 0;
	virtual AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) = 0;
	virtual AttrHandle channelAttr(ChannelHandle ch, const QString &name) = 0;
	virtual void releaseAttr(AttrHandle attr) = 0;

	// I/O — returns a typed Result; called from Command::execute() on worker thread.
	// The read payload is the raw attr bytes; errors travel in the Unexpected<Error> channel.
	virtual Result<QByteArray> read(AttrHandle attr) = 0;
	virtual Result<void> write(AttrHandle attr, const QString &value) = 0;

	// Range / available — convenience parsers for _available attributes.
	// v1: thin wrappers around iio_attr_get_range / iio_attr_get_available.
	// v0: backend reads the attr string and parses it manually.
	virtual Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const = 0;
	virtual Result<void> getAvailable(AttrHandle attr, QStringList &values) const = 0;
};

} // namespace scopy::iio
