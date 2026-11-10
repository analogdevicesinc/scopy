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

#include "v1bufferops.h"
#include "core/calltrace.h"
#include <cerrno>
#include <qdebug.h>

namespace scopy::iio {

// libiio v1 reports pointer errors in the pointer itself.
#define V1_TRACE_PTR(fn, handle, expr, ...)                                                                            \
	DC_TRACE_EX("libiio", fn, handle, expr, iio_err(dcRc), CallTrace::outOf(dcRc), __VA_ARGS__)

static qint64 blockSize(iio_block *b)
{
	return static_cast<const char *>(iio_block_end(b)) - static_cast<const char *>(iio_block_start(b));
}

static CallOut blockPayload(iio_block *b) { return CallTrace::payload(iio_block_start(b), blockSize(b)); }

BufferHandle V1BufferOps::openBuffer(DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask, size_t samplesCount,
				     bool cyclic, unsigned int kernelBuffers)
{
	iio_device *d = static_cast<iio_device *>(dev.ptr);
	iio_channels_mask *m = static_cast<iio_channels_mask *>(mask.ptr);

	iio_buffer *staticBuf =
		V1_TRACE_PTR("iio_device_get_buffer", d, iio_device_get_buffer(d, idx), CallArg::integer("index", idx));
	if(iio_err(staticBuf)) {
		return {};
	}

	iio_buffer_stream *bufStream =
		V1_TRACE_PTR("iio_buffer_open", staticBuf, iio_buffer_open(staticBuf, m), CallArg::handle("mask", m));
	if(iio_err(bufStream)) {
		return {};
	}

	ssize_t sampleSize = DC_TRACE("libiio", "iio_device_get_sample_size", d, iio_device_get_sample_size(d, m),
				      CallArg::handle("mask", m));
	if(sampleSize <= 0) {
		DC_TRACE("libiio", "iio_buffer_close", bufStream, iio_buffer_close(bufStream));
		return {};
	}

	size_t nbBlocks = cyclic ? 1 : kernelBuffers;
	size_t blockSize = samplesCount * (size_t)sampleSize;

	QVector<iio_block *> blocks;
	for(size_t i = 0; i < nbBlocks; ++i) {
		iio_block *b = V1_TRACE_PTR("iio_buffer_stream_create_block", bufStream,
					    iio_buffer_stream_create_block(bufStream, blockSize),
					    CallArg::integer("size", qint64(blockSize)));
		if(iio_err(b)) {
			for(auto *blk : std::as_const(blocks)) {
				DC_TRACE("libiio", "iio_block_destroy", blk, iio_block_destroy(blk));
			}
			DC_TRACE("libiio", "iio_buffer_close", bufStream, iio_buffer_close(bufStream));
			return {};
		}
		blocks.push_back(b);
	}

	auto *s = new V1BufferState;
	s->staticBuf = staticBuf;
	s->bufStream = bufStream;
	s->blocks = blocks;
	s->nbBlocks = nbBlocks;
	s->crtIndex = 0;
	s->sampleSize = sampleSize;
	s->cyclic = cyclic;
	s->bufEnabled = false;

	return {s};
}

void V1BufferOps::closeBuffer(BufferHandle h)
{
	V1BufferState *s = state(h);
	DC_TRACE("libiio", "iio_buffer_stream_cancel", s->bufStream, iio_buffer_stream_cancel(s->bufStream));
	if(s->bufEnabled) {
		DC_TRACE("libiio", "iio_buffer_stream_stop", s->bufStream, iio_buffer_stream_stop(s->bufStream));
	}
	for(auto *b : std::as_const(s->blocks)) {
		DC_TRACE("libiio", "iio_block_dequeue", b, iio_block_dequeue(b, false),
			 CallArg::boolean("nonblock", false));
		DC_TRACE("libiio", "iio_block_destroy", b, iio_block_destroy(b));
	}
	DC_TRACE("libiio", "iio_buffer_close", s->bufStream, iio_buffer_close(s->bufStream));
	delete s;
}

void V1BufferOps::cancelBuffer(BufferHandle h)
{
	// Called directly (not from a command) to unblock a refill: traced only inside a command.
	DC_TRACE("libiio", "iio_buffer_stream_cancel", state(h)->bufStream,
		 iio_buffer_stream_cancel(state(h)->bufStream));
}

Result<void> V1BufferOps::startRefill(V1BufferState *s)
{
	for(auto *b : std::as_const(s->blocks)) {
		int ret = DC_TRACE("libiio", "iio_block_enqueue", b, iio_block_enqueue(b, 0, false),
				   CallArg::integer("bytes_used", 0), CallArg::boolean("cyclic", false));
		if(ret < 0) {
			return Unexpected{Error{ret, QStringLiteral("iio_block_enqueue failed (startRefill)")}};
		}
	}
	int ret = DC_TRACE("libiio", "iio_buffer_stream_start", s->bufStream, iio_buffer_stream_start(s->bufStream));
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_buffer_stream_start failed")}};
	}
	s->bufEnabled = true;
	return {};
}

Result<void> V1BufferOps::continueRefill(V1BufferState *s)
{
	unsigned int doneIdx = s->crtIndex % s->nbBlocks;
	iio_block *b = s->blocks[doneIdx];
	int ret = DC_TRACE("libiio", "iio_block_enqueue", b, iio_block_enqueue(b, 0, false),
			   CallArg::integer("bytes_used", 0), CallArg::boolean("cyclic", false));
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_block_enqueue failed (continueRefill)")}};
	}
	s->crtIndex++;
	return {};
}

Result<void> V1BufferOps::refill(BufferHandle h)
{
	V1BufferState *s = state(h);

	Result<void> r = s->bufEnabled ? continueRefill(s) : startRefill(s);
	if(!r) {
		return r;
	}

	unsigned int nextIdx = s->crtIndex % s->nbBlocks;
	iio_block *next = s->blocks[nextIdx];
	int ret = DC_TRACE_OUT("libiio", "iio_block_dequeue", next, iio_block_dequeue(next, false),
			       dcRc < 0 ? CallOut() : blockPayload(next), CallArg::boolean("nonblock", false));
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_block_dequeue failed (refill)")}};
	}

	return {};
}

Result<void> V1BufferOps::push(BufferHandle h)
{
	V1BufferState *s = state(h);
	unsigned int blockIdx = s->crtIndex % s->nbBlocks;
	iio_block *crtBlock = s->blocks[blockIdx];

	if(s->cyclic && s->crtIndex > 0) {
		return {};
	}

	if(s->crtIndex >= s->nbBlocks) {
		int ret = DC_TRACE("libiio", "iio_block_dequeue", crtBlock, iio_block_dequeue(crtBlock, false),
				   CallArg::boolean("nonblock", false));
		if(ret < 0) {
			return Unexpected{Error{ret, QStringLiteral("iio_block_dequeue failed (push)")}};
		}
	}

	// The block content goes out with the call: keep its preview as an argument.
	int ret = DC_TRACE("libiio", "iio_block_enqueue", crtBlock, iio_block_enqueue(crtBlock, 0, s->cyclic),
			   CallArg::integer("bytes_used", 0), CallArg::boolean("cyclic", s->cyclic),
			   CallArg::bytes("data", blockPayload(crtBlock).value.toByteArray()),
			   CallArg::integer("size", blockSize(crtBlock)));
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_block_enqueue failed (push)")}};
	}

	if(!s->bufEnabled) {
		ret = DC_TRACE("libiio", "iio_buffer_stream_start", s->bufStream,
			       iio_buffer_stream_start(s->bufStream));
		if(ret < 0) {
			return Unexpected{Error{ret, QStringLiteral("iio_buffer_stream_start failed (push)")}};
		}
		s->bufEnabled = true;
	}

	s->crtIndex++;
	return {};
}

void *V1BufferOps::bufferStart(BufferHandle h) const
{
	V1BufferState *s = state(h);
	unsigned int blockIdx = s->crtIndex % s->nbBlocks;
	iio_block *crtBlock = s->blocks[blockIdx];
	return iio_block_start(crtBlock);
}

void *V1BufferOps::bufferEnd(BufferHandle h) const
{
	V1BufferState *s = state(h);
	unsigned int blockIdx = s->crtIndex % s->nbBlocks;
	iio_block *crtBlock = s->blocks[blockIdx];
	return iio_block_end(crtBlock);
}

ptrdiff_t V1BufferOps::bufferStep(BufferHandle h) const { return state(h)->sampleSize; }

void *V1BufferOps::bufferFirst(BufferHandle h, ChannelHandle ch) const
{
	V1BufferState *s = state(h);
	unsigned int blockIdx = s->crtIndex % s->nbBlocks;
	iio_block *crtBlock = s->blocks[blockIdx];
	return iio_block_first(crtBlock, static_cast<const iio_channel *>(ch.ptr));
}

ChannelsMaskHandle V1BufferOps::createChannelsMask(unsigned int nbChannels)
{
	return {DC_TRACE("libiio", "iio_create_channels_mask", nullptr, iio_create_channels_mask(nbChannels),
			 CallArg::integer("nb_channels", nbChannels))};
}

void V1BufferOps::destroyChannelsMask(ChannelsMaskHandle mask)
{
	DC_TRACE("libiio", "iio_channels_mask_destroy", mask.ptr,
		 iio_channels_mask_destroy(static_cast<iio_channels_mask *>(mask.ptr)));
}

} // namespace scopy::iio
