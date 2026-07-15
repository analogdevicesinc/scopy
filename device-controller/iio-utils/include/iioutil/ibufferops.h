#pragma once

#include "iioutil/handles.h"
#include "core/result.h"
#include <cstddef>

namespace scopy::iio {

class IBufferOps {
public:
	virtual ~IBufferOps() = default;

	// Lifecycle
	// v0: iio_device_create_buffer(dev, samplesCount, cyclic)
	// v1: iio_device_get_buffer(dev, idx) -> iio_buffer_open(buf, mask)
	//     -> iio_buffer_stream_start + create kernelBuffers blocks
	virtual BufferHandle openBuffer(DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask,
					size_t samplesCount, bool cyclic, unsigned int kernelBuffers = 4) = 0;

	// v0: iio_buffer_destroy(buf)
	// v1: destroy blocks -> iio_buffer_stream_stop -> iio_buffer_close(stream)
	virtual void closeBuffer(BufferHandle buf) = 0;

	// v0: iio_buffer_cancel(buf)
	// v1: iio_buffer_stream_cancel(stream)
	// Called DIRECTLY — never through IExecution (avoids serial-queue deadlock)
	virtual void cancelBuffer(BufferHandle buf) = 0;

	// I/O — returns Result<void>; called from Command::execute() on worker thread
	virtual Result<void> refill(BufferHandle buf) = 0;
	virtual Result<void> push(BufferHandle buf) = 0;

	// Pointer access — no I/O, direct pointer arithmetic
	virtual void *bufferStart(BufferHandle buf) const = 0;
	virtual void *bufferEnd(BufferHandle buf) const = 0;
	virtual ptrdiff_t bufferStep(BufferHandle buf) const = 0;
	virtual void *bufferFirst(BufferHandle buf, ChannelHandle ch) const = 0;

	// Mask management
	virtual ChannelsMaskHandle createChannelsMask(unsigned int nbChannels) = 0;
	virtual void destroyChannelsMask(ChannelsMaskHandle mask) = 0;

};

} // namespace scopy::iio
