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

#include "iioutil/ibackend.h"
#include "v0contextops.h"
#include "v0deviceops.h"
#include "v0channelops.h"
#include "v0attrops.h"
#include "v0bufferops.h"
#include "v0scanops.h"

namespace scopy::iio {

class V0Backend : public IBackend
{
public:
	LibiioVersion version() const override { return LibiioVersion::V0; }

	IContextOps *contextOps() override { return &m_contextOps; }
	IDeviceOps *deviceOps() override { return &m_deviceOps; }
	IChannelOps *channelOps() override { return &m_channelOps; }
	IAttrOps *attrOps() override { return &m_attrOps; }
	IBufferOps *bufferOps() override { return &m_bufferOps; }
	IScanOps *scanOps() override { return &m_scanOps; }

private:
	V0ContextOps m_contextOps;
	V0DeviceOps m_deviceOps;
	V0ChannelOps m_channelOps;
	V0AttrOps m_attrOps;
	V0BufferOps m_bufferOps;
	V0ScanOps m_scanOps;
};

} // namespace scopy::iio
