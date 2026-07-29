#pragma once

#include "component/streamformat.h"

#include <QVector>
#include <cstring>

namespace scopy::component {

// Read-only decoder over a StreamFormat: raw byte access and normalized-double
// materialization, so callers never write switch(type) + pointer math.
class StreamView
{
public:
	explicit StreamView(const StreamFormat &format)
		: m_format(format)
	{
	}

	int channelCount() const { return m_format.channels.size(); }
	size_t sampleCount() const { return m_format.sampleCount; }

	template <typename T>
	T raw(int ch, size_t sample) const
	{
		const ChannelFormat &cf = m_format.channels.at(ch);
		const char *p = static_cast<const char *>(m_format.data) + cf.offset + cf.stride * ptrdiff_t(sample);
		T value;
		std::memcpy(&value, p, sizeof(T));
		return value;
	}

	double sampleAsDouble(int ch, size_t sample) const
	{
		const ChannelFormat &cf = m_format.channels.at(ch);
		const int64_t rawInt = rawInteger(cf, ch, sample);
		return double(rawInt) * cf.scale + cf.offsetPhys;
	}

	// One channel, materialized as owned doubles.
	QVector<double> toDoubles(int ch) const
	{
		QVector<double> out;
		out.reserve(int(m_format.sampleCount));
		for(size_t s = 0; s < m_format.sampleCount; ++s) {
			out.append(sampleAsDouble(ch, s));
		}
		return out;
	}

	// All channels de-interleaved: out[c] is the c-th channel's samples.
	QVector<QVector<double>> toDoubles() const
	{
		QVector<QVector<double>> out(channelCount());
		for(int c = 0; c < channelCount(); ++c) {
			out[c] = toDoubles(c);
		}
		return out;
	}

private:
	int64_t rawInteger(const ChannelFormat &cf, int ch, size_t sample) const
	{
		switch(cf.type) {
		case DataType::S8:
			return decodeInt(int8_t(raw<uint8_t>(ch, sample)), cf);
		case DataType::U8:
			return decodeInt(raw<uint8_t>(ch, sample), cf);
		case DataType::S16:
			return decodeInt(raw<uint16_t>(ch, sample), cf);
		case DataType::U16:
			return decodeInt(raw<uint16_t>(ch, sample), cf);
		case DataType::S32:
			return decodeInt(raw<uint32_t>(ch, sample), cf);
		case DataType::U32:
			return decodeInt(raw<uint32_t>(ch, sample), cf);
		case DataType::S64:
			return decodeInt(raw<uint64_t>(ch, sample), cf);
		case DataType::U64:
			return decodeInt(raw<uint64_t>(ch, sample), cf);
		case DataType::F32:
			return int64_t(raw<float>(ch, sample));
		case DataType::F64:
			return int64_t(raw<double>(ch, sample));
		}
		return 0;
	}

	static bool isSigned(DataType t)
	{
		return t == DataType::S8 || t == DataType::S16 || t == DataType::S32 || t == DataType::S64;
	}

	static int64_t decodeInt(uint64_t container, const ChannelFormat &cf)
	{
		uint64_t v = container >> cf.shift;
		const uint64_t mask = (cf.validBits >= 64) ? ~uint64_t(0) : ((uint64_t(1) << cf.validBits) - 1);
		v &= mask;
		if(isSigned(cf.type) && cf.validBits < 64) {
			const uint64_t signBit = uint64_t(1) << (cf.validBits - 1);
			if(v & signBit) {
				v |= ~mask; // sign-extend
			}
		}
		return int64_t(v);
	}

	const StreamFormat &m_format;
};

} // namespace scopy::component
