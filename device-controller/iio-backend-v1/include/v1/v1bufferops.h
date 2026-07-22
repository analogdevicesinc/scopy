#pragma once

#include "iioutil/ibufferops.h"
#include <iio/iio.h>

namespace scopy::iio {

// v1 buffer lifecycle:
//   openBuffer  : iio_device_get_buffer -> iio_buffer_open -> iio_buffer_stream_create_block x N
//   closeBuffer : iio_buffer_stream_cancel -> iio_block_destroy x N -> iio_buffer_close
//   cancelBuffer: iio_buffer_stream_cancel — called directly, never through IExecution
//   refill      : iio_block_dequeue (RX: waits for hardware to fill current block)
//   push        : iio_block_enqueue + iio_buffer_stream_start (lazy) + iio_block_dequeue on wrap

struct V1BufferState
{
    iio_buffer *staticBuf;
    iio_buffer_stream *bufStream;
    QVector<iio_block *> blocks;
    size_t nbBlocks;
    unsigned int crtIndex;
    ptrdiff_t sampleSize;
    bool cyclic;
    bool bufEnabled;
};

class V1BufferOps : public IBufferOps
{
public:
    BufferHandle openBuffer(DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask, size_t samplesCount,
                bool cyclic = false, unsigned int kernelBuffers = 4) override;
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
    static V1BufferState *state(BufferHandle h) { return static_cast<V1BufferState *>(h.ptr); }
    static Result<void> startRefill(V1BufferState *s);
    static Result<void> continueRefill(V1BufferState *s);
};

} // namespace scopy::iio
