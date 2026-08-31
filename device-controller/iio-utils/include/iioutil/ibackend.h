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

namespace scopy::iio {

class IContextOps;
class IDeviceOps;
class IChannelOps;
class IAttrOps;
class IBufferOps;
class IScanOps;

class IBackend
{
public:
	virtual ~IBackend() = default;

	virtual LibiioVersion version() const = 0;

	virtual IContextOps *contextOps() = 0;
	virtual IDeviceOps *deviceOps() = 0;
	virtual IChannelOps *channelOps() = 0;
	virtual IAttrOps *attrOps() = 0;
	virtual IBufferOps *bufferOps() = 0;
	virtual IScanOps *scanOps() = 0;
};

} // namespace scopy::iio
