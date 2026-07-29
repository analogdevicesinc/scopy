#pragma once

#include <QVector>
#include <cstddef>
#include <cstdint>

namespace scopy::component {

enum class DataType : uint8_t
{
	S8,
	U8,
	S16,
	U16,
	S32,
	U32,
	S64,
	U64,
	F32,
	F64
};

enum class SamplesLayout : uint8_t
{
	Interleaved,
	Planar
};

enum class ByteOrder : uint8_t
{
	LittleEndian,
	BigEndian
};

// Per-channel view into the stream-owned buffer: where the samples are, how to
// decode raw bytes to an integer, and how to scale that to physical units.
struct ChannelFormat
{
	ptrdiff_t offset = 0; // bytes from StreamFormat::data to this channel's first sample
	ptrdiff_t stride = 0; // bytes between consecutive samples of this channel
	DataType type = DataType::S16;
	uint8_t validBits = 16;	 // meaningful bits inside the container
	uint8_t shift = 0;	 // right-shift before masking to validBits
	double scale = 1.0;	 // raw integer -> physical units
	double offsetPhys = 0.0; // additive physical offset (v1; 0.0 on v0)
};

// Zero-copy descriptor of a refilled (RX) or writable (TX) buffer. The stream
// always owns `data`; the descriptor only views it.
struct StreamFormat
{
	void *data = nullptr;
	size_t sampleCount = 0; // per channel
	SamplesLayout layout = SamplesLayout::Interleaved;
	ByteOrder order = ByteOrder::LittleEndian;
	QVector<ChannelFormat> channels;
};

inline DataType dataTypeOf(unsigned int bits, bool isSigned)
{
	if(bits <= 8) {
		return isSigned ? DataType::S8 : DataType::U8;
	}
	if(bits <= 16) {
		return isSigned ? DataType::S16 : DataType::U16;
	}
	if(bits <= 32) {
		return isSigned ? DataType::S32 : DataType::U32;
	}
	return isSigned ? DataType::S64 : DataType::U64;
}

} // namespace scopy::component
