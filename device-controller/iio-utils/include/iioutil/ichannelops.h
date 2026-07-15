#pragma once

#include "iioutil/dataformat.h"
#include "iioutil/handles.h"
#include <QString>
#include <cstddef>

namespace scopy::iio {

class IChannelOps {
public:
	virtual ~IChannelOps() = default;

	// Metadata — infallible, return directly
	virtual QString id(ChannelHandle ch) const = 0;
	virtual QString name(ChannelHandle ch) const = 0;
	virtual QString label(ChannelHandle ch) const = 0; // v0: returns ""
	virtual bool isOutput(ChannelHandle ch) const = 0;
	virtual bool isScanElement(ChannelHandle ch) const = 0;
	virtual long index(ChannelHandle ch) const = 0;
	virtual int channelType(ChannelHandle ch) const = 0;
	virtual int modifier(ChannelHandle ch) const = 0;
	virtual unsigned int attrsCount(ChannelHandle ch) const = 0;
	virtual QString attrName(ChannelHandle ch, unsigned int index) const = 0;
	virtual DataFormat dataFormat(ChannelHandle ch) const = 0;

	// Enable/disable — mask required for v1; v0 impl ignores it
	virtual void enable(ChannelHandle ch, ChannelsMaskHandle mask) = 0;
	virtual void disable(ChannelHandle ch, ChannelsMaskHandle mask) = 0;
	virtual bool isEnabled(ChannelHandle ch, ChannelsMaskHandle mask) const = 0;

	// Demux [+ convert] from an already-refilled block/buffer — no kernel call, local memcpy only.
	// raw=true  -> demux only (hardware format preserved)
	// raw=false -> demux + convert to host format
	// v0: BlockHandle.ptr is iio_buffer* — maps to iio_channel_read_raw / iio_channel_read
	// v1: BlockHandle.ptr is iio_block*  — maps to iio_channel_read(chn, block, dst, len, raw)
	virtual size_t read(ChannelHandle ch, BlockHandle block, void *dst, size_t len, bool raw) const = 0;
	virtual size_t write(ChannelHandle ch, BlockHandle block, const void *src, size_t len, bool raw) = 0;

	// Single-sample conversion — pure computation, no block/buffer needed.
	// Maps to iio_channel_convert / iio_channel_convert_inverse (both v0 and v1).
	virtual void convert(ChannelHandle ch, void *dst, const void *src) const = 0;
	virtual void convertInverse(ChannelHandle ch, void *dst, const void *src) const = 0;
};

} // namespace scopy::iio
