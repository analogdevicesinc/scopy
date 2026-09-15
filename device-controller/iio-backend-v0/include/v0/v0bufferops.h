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

#include "iioutil/ibufferops.h"
#include <iio.h>

namespace scopy::iio {

struct V0BufferState
{
	iio_buffer *buf;
	bool cyclic;
	bool started;
};

class V0BufferOps : public IBufferOps
{
public:
	BufferHandle openBuffer(DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask, size_t samplesCount,
				bool cyclic, unsigned int kernelBuffers = 4) override;
	void closeBuffer(BufferHandle buf) override;
	void cancelBuffer(BufferHandle buf) override;

	Result<void> refill(BufferHandle buf) override;
	Result<void> push(BufferHandle buf) override;

	void *bufferStart(BufferHandle buf) const override;
	void *bufferEnd(BufferHandle buf) const override;
	ptrdiff_t bufferStep(BufferHandle buf) const override;
	void *bufferFirst(BufferHandle buf, ChannelHandle ch) const override;

	ChannelsMaskHandle createChannelsMask(unsigned int nbChannels) override;
	void destroyChannelsMask(ChannelsMaskHandle mask) override;

private:
	static V0BufferState *state(BufferHandle h) { return static_cast<V0BufferState *>(h.ptr); }
};

} // namespace scopy::iio
