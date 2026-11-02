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

#include "component/streamformat.h"

#include <QtEndian>

#include <algorithm>
#include <cmath>
#include <cstring>

#include "component/component_export.h"

namespace scopy::component {

// Write-side counterpart of StreamView: encodes samples into a writable StreamFormat (an
// OutputStream's writeFormat()), so callers never write switch(type) + pointer math.
class COMPONENT_EXPORT StreamWriter
{
public:
	explicit StreamWriter(StreamFormat &format)
		: m_format(format)
	{}

	int channelCount() const { return m_format.channels.size(); }
	size_t sampleCount() const { return m_format.sampleCount; }

	// Stores the integer `raw` (masked to validBits, shifted into place) as sample `sample`.
	void setRaw(int ch, size_t sample, int64_t raw)
	{
		const ChannelFormat &cf = m_format.channels.at(ch);
		char *p = static_cast<char *>(m_format.data) + cf.offset + cf.stride * ptrdiff_t(sample);
		switch(cf.type) {
		case DataType::F32: {
			const float v = float(raw);
			std::memcpy(p, &v, sizeof(v));
			return;
		}
		case DataType::F64: {
			const double v = double(raw);
			std::memcpy(p, &v, sizeof(v));
			return;
		}
		case DataType::S8:
		case DataType::U8:
			store<uint8_t>(p, container(cf, raw));
			return;
		case DataType::S16:
		case DataType::U16:
			store<uint16_t>(p, container(cf, raw));
			return;
		case DataType::S32:
		case DataType::U32:
			store<uint32_t>(p, container(cf, raw));
			return;
		case DataType::S64:
		case DataType::U64:
			store<uint64_t>(p, container(cf, raw));
			return;
		}
	}

	// `value` in full-scale units, clamped to [-1, 1]; see normalizedToRaw().
	void setNormalized(int ch, size_t sample, double value)
	{
		setRaw(ch, sample, normalizedToRaw(m_format.channels.at(ch), value));
	}

	// Full-scale value -> code: signed maps [-1, 1] onto [-(2^(b-1) - 1), 2^(b-1) - 1] (0 -> 0);
	// unsigned maps it onto [0, 2^b - 1] (offset binary, 0 -> mid-scale).
	static int64_t normalizedToRaw(const ChannelFormat &cf, double value)
	{
		value = std::clamp(value, -1.0, 1.0);
		const int bits = std::clamp(int(cf.validBits), 1, 63);
		if(isSigned(cf.type)) {
			const double top = std::ldexp(1.0, bits - 1) - 1;
			return int64_t(std::llround(value * top));
		}
		const double top = std::ldexp(1.0, bits) - 1;
		return int64_t(std::llround((value + 1) / 2 * top));
	}

private:
	static bool isSigned(DataType t)
	{
		return t == DataType::S8 || t == DataType::S16 || t == DataType::S32 || t == DataType::S64 ||
			t == DataType::F32 || t == DataType::F64;
	}

	static uint64_t container(const ChannelFormat &cf, int64_t raw)
	{
		const uint64_t mask = (cf.validBits >= 64) ? ~uint64_t(0) : ((uint64_t(1) << cf.validBits) - 1);
		return (uint64_t(raw) & mask) << cf.shift;
	}

	template <typename T>
	void store(char *p, uint64_t v) const
	{
		T t = T(v);
		t = m_format.order == ByteOrder::BigEndian ? qToBigEndian(t) : qToLittleEndian(t);
		std::memcpy(p, &t, sizeof(T));
	}

	StreamFormat &m_format;
};

} // namespace scopy::component
