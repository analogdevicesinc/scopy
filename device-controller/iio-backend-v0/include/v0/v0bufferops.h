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
