#include "v1bufferops.h"
#include <cerrno>
#include <qdebug.h>

namespace scopy::iio {

BufferHandle V1BufferOps::openBuffer(DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask, size_t samplesCount,
                     bool cyclic, unsigned int kernelBuffers)
{
    iio_device *d = static_cast<iio_device *>(dev.ptr);
    iio_channels_mask *m = static_cast<iio_channels_mask *>(mask.ptr);

    iio_buffer *staticBuf = iio_device_get_buffer(d, idx);
    if(iio_err(staticBuf)) {
        return {};
    }

    iio_buffer_stream *bufStream = iio_buffer_open(staticBuf, m);
    if(iio_err(bufStream)) {
        return {};
    }

    ssize_t sampleSize = iio_device_get_sample_size(d, m);
    if(sampleSize <= 0) {
        iio_buffer_close(bufStream);
        return {};
    }

    size_t nbBlocks = cyclic ? 1 : kernelBuffers;
    size_t blockSize = samplesCount * (size_t)sampleSize;

    QVector<iio_block *> blocks;
    for(size_t i = 0; i < nbBlocks; ++i) {
        iio_block *b = iio_buffer_stream_create_block(bufStream, blockSize);
        if(iio_err(b)) {
            for(auto *blk : std::as_const(blocks)) {
                iio_block_destroy(blk);
            }
            iio_buffer_close(bufStream);
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
    iio_buffer_stream_cancel(s->bufStream);
    if(s->bufEnabled) {
        iio_buffer_stream_stop(s->bufStream);
    }
    for(auto *b : std::as_const(s->blocks)) {
        iio_block_dequeue(b, false);
        iio_block_destroy(b);
    }
    iio_buffer_close(s->bufStream);
    delete s;
}

void V1BufferOps::cancelBuffer(BufferHandle h) { iio_buffer_stream_cancel(state(h)->bufStream); }

Result<void> V1BufferOps::startRefill(V1BufferState *s)
{
    for(auto *b : std::as_const(s->blocks)) {
        int ret = iio_block_enqueue(b, 0, false);
        if(ret < 0) {
            return Unexpected{Error{ret, QStringLiteral("iio_block_enqueue failed (startRefill)")}};
        }
    }
    int ret = iio_buffer_stream_start(s->bufStream);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_buffer_stream_start failed")}};
    }
    s->bufEnabled = true;
    return {};
}

Result<void> V1BufferOps::continueRefill(V1BufferState *s)
{
    unsigned int doneIdx = s->crtIndex % s->nbBlocks;
    int ret = iio_block_enqueue(s->blocks[doneIdx], 0, false);
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
    int ret = iio_block_dequeue(s->blocks[nextIdx], false);
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
        int ret = iio_block_dequeue(crtBlock, false);
        if(ret < 0) {
            return Unexpected{Error{ret, QStringLiteral("iio_block_dequeue failed (push)")}};
        }
    }

    int ret = iio_block_enqueue(crtBlock, 0, s->cyclic);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_block_enqueue failed (push)")}};
    }

    if(!s->bufEnabled) {
        ret = iio_buffer_stream_start(s->bufStream);
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
    return {iio_create_channels_mask(nbChannels)};
}

void V1BufferOps::destroyChannelsMask(ChannelsMaskHandle mask)
{
    iio_channels_mask_destroy(static_cast<iio_channels_mask *>(mask.ptr));
}

} // namespace scopy::iio
