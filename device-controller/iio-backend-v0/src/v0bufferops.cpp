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

#include "v0bufferops.h"
#include <iio.h>

namespace scopy::iio {

static iio_device *dev(DeviceHandle h) { return static_cast<iio_device *>(h.ptr); }
static iio_channel *chn(ChannelHandle h) { return static_cast<iio_channel *>(h.ptr); }

BufferHandle V0BufferOps::openBuffer(DeviceHandle d, unsigned int /*idx*/, ChannelsMaskHandle /*mask*/,
				     size_t samplesCount, bool cyclic, unsigned int kernelBuffers)
{
	iio_device_set_kernel_buffers_count(dev(d), kernelBuffers);
	iio_buffer *b = iio_device_create_buffer(dev(d), samplesCount, cyclic);
	if(!b) {
		return {};
	}
	auto *s = new V0BufferState{b, cyclic, false};
	return {s};
}

void V0BufferOps::closeBuffer(BufferHandle h)
{
	V0BufferState *s = state(h);
	if(s) {
		iio_buffer_destroy(s->buf);
		delete s;
	}
}

void V0BufferOps::cancelBuffer(BufferHandle h)
{
	V0BufferState *s = state(h);
	if(s) {
		iio_buffer_cancel(s->buf);
	}
}

Result<void> V0BufferOps::refill(BufferHandle h)
{
	ssize_t ret = iio_buffer_refill(state(h)->buf);
	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_buffer_refill failed")}};
	}
	return {};
}

Result<void> V0BufferOps::push(BufferHandle h)
{
	V0BufferState *s = state(h);
	if(s->cyclic && s->started) {
		return {};
	}
	ssize_t ret = iio_buffer_push(s->buf);
	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_buffer_push failed")}};
	}
	s->started = true;
	return {};
}

void *V0BufferOps::bufferStart(BufferHandle h) const { return iio_buffer_start(state(h)->buf); }

void *V0BufferOps::bufferEnd(BufferHandle h) const { return iio_buffer_end(state(h)->buf); }

ptrdiff_t V0BufferOps::bufferStep(BufferHandle h) const { return iio_buffer_step(state(h)->buf); }

void *V0BufferOps::bufferFirst(BufferHandle h, ChannelHandle ch) const
{
	return iio_buffer_first(state(h)->buf, chn(ch));
}

ChannelsMaskHandle V0BufferOps::createChannelsMask(unsigned int /*nbChannels*/) { return {}; }

void V0BufferOps::destroyChannelsMask(ChannelsMaskHandle /*mask*/) {}

} // namespace scopy::iio
