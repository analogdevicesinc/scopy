#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

#include <cerrno>

namespace scopy::iio {

class BufferOpenCommand : public Command
{
	Q_OBJECT
public:
	BufferOpenCommand(IBufferOps *ops, DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask,
              size_t samplesCount, bool cyclic, unsigned int kernelBuffers = 4, QObject *parent = nullptr)
		: Command(BufferCreate, dev.ptr, parent)
		, m_ops(ops)
		, m_devHandle(dev)
		, m_idx(idx)
		, m_mask(mask)
		, m_samples(samplesCount)
		, m_cyclic(cyclic)
		, m_kernelBuffers(kernelBuffers)
    {
    }

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
            m_openedHandle =
                m_ops->openBuffer(m_devHandle, m_idx, m_mask, m_samples, m_cyclic, m_kernelBuffers);
			m_result = m_openedHandle.ptr
                ? Result<void>()
                : Result<void>(Unexpected{Error{-EIO, QStringLiteral("buffer open failed")}});
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }
	BufferHandle openedHandle() const { return m_openedHandle; }

	QString toString() const override
	{
		return QStringLiteral("BufferOpen(dev=%1, idx=%2, samples=%3, cyclic=%4)")
			.arg(quintptr(m_devHandle.ptr))
			.arg(m_idx)
			.arg(m_samples)
			.arg(m_cyclic);
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
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
