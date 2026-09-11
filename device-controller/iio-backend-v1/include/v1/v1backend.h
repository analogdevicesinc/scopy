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
#include "v1contextops.h"
#include "v1deviceops.h"
#include "v1channelops.h"
#include "v1attrops.h"
#include "v1bufferops.h"
#include "v1scanops.h"

namespace scopy::iio {

class V1Backend : public IBackend
{
public:
	LibiioVersion version() const override { return LibiioVersion::V1; }

	IContextOps *contextOps() override { return &m_contextOps; }
	IDeviceOps *deviceOps() override { return &m_deviceOps; }
	IChannelOps *channelOps() override { return &m_channelOps; }
	IAttrOps *attrOps() override { return &m_attrOps; }
	IBufferOps *bufferOps() override { return &m_bufferOps; }
	IScanOps *scanOps() override { return &m_scanOps; }

private:
	V1ContextOps m_contextOps;
	V1DeviceOps m_deviceOps;
	V1ChannelOps m_channelOps;
	V1AttrOps m_attrOps;
	V1BufferOps m_bufferOps;
	V1ScanOps m_scanOps;
};

} // namespace scopy::iio
