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
#include "core/result.h"
#include <cstddef>

namespace scopy::iio {

class IBufferOps
{
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
