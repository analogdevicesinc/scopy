#pragma once

#include "core/resultcommand.h"
#include "iioutil/ibufferops.h"

#include <cerrno>

namespace scopy::iio {

class BufferOpenCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	BufferOpenCommand(IBufferOps *ops, DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask,
			  size_t samplesCount, bool cyclic, unsigned int kernelBuffers = 4, QObject *parent = nullptr)
		: ResultCommand(dev.ptr, parent)
		, m_ops(ops)
		, m_devHandle(dev)
		, m_idx(idx)
		, m_mask(mask)
		, m_samples(samplesCount)
		, m_cyclic(cyclic)
		, m_kernelBuffers(kernelBuffers)
	{
	}

	BufferHandle openedHandle() const { return m_openedHandle; }

protected:
	void run() override
	{
		m_openedHandle = m_ops->openBuffer(m_devHandle, m_idx, m_mask, m_samples, m_cyclic, m_kernelBuffers);
        setResult(m_openedHandle.ptr
                  ? Result<void>()
                  : Result<void>(Unexpected{Error{-EIO, QStringLiteral("buffer open failed")}}));
	}

private:
	IBufferOps *m_ops;
	DeviceHandle m_devHandle;
	unsigned int m_idx;
	ChannelsMaskHandle m_mask;
	size_t m_samples;
	bool m_cyclic;
	unsigned int m_kernelBuffers;
	BufferHandle m_openedHandle;
};

} // namespace scopy::iio
