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

#include "component/samplecodec.h"
#include "iioutil/dataformat.h"
#include "iioutil/handles.h"

namespace scopy::iio {
class IChannelOps;
}

namespace scopy::component::iio {

class IIOSampleCodec : public SampleCodec
{
	Q_OBJECT
public:
	IIOSampleCodec(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle, QObject *parent = nullptr);

	const scopy::iio::DataFormat &dataFormat() const { return m_format; }

	void convert(void *dst, const void *src) const override;
	void convertInverse(void *dst, const void *src) const override;

private:
	scopy::iio::IChannelOps *m_ops;
	scopy::iio::ChannelHandle m_handle;
	scopy::iio::DataFormat m_format;
};

} // namespace scopy::component::iio
