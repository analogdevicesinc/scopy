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

#include "component/backends/iio/iiosamplecodec.h"

#include "iioutil/ichannelops.h"

using namespace scopy::component::iio;

IIOSampleCodec::IIOSampleCodec(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle, QObject *parent)
	: SampleCodec(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_format(ops->dataFormat(handle))
{}

void IIOSampleCodec::convert(void *dst, const void *src) const { m_ops->convert(m_handle, dst, src); }

void IIOSampleCodec::convertInverse(void *dst, const void *src) const { m_ops->convertInverse(m_handle, dst, src); }
