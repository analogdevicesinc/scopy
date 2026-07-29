#pragma once

#include "component/inputstream.h"
#include "iioutil/handles.h"

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IBufferOps;
class IChannelOps;
class BufferRefillCommand;
}
} // namespace scopy

namespace scopy::component::iio {

class IIOInputStream : public InputStream
{
	Q_OBJECT
	Q_PROPERTY(unsigned kernelBuffers READ kernelBuffers CONSTANT)
public:
	IIOInputStream(scopy::iio::IBufferOps *bufOps, scopy::iio::IChannelOps *chOps, scopy::iio::DeviceHandle dev,
		       int nbChannels, scopy::ICmdExecutor *executor, QObject *parent = nullptr);
	~IIOInputStream() override;

	scopy::iio::ChannelsMaskHandle mask() const { return m_mask; }
	unsigned kernelBuffers() const { return m_kernelBuffers; }
	Result<void> setKernelBuffers(unsigned n);

	Result<void> open(const StreamConfig &cfg) override;
	void close() override;
	Result<void> refill() override;
	QUuid refillAsync() override;
	const StreamFormat &readFormat() const override { return m_format; }

private:
	QCoro::Task<Result<void>> openInternal(StreamConfig cfg);
	QCoro::Task<Result<void>> refillInternal(scopy::iio::BufferRefillCommand *cmd);
	QCoro::Task<void> closeInternal();
	void applyEnabledChannels(const QList<int> &channels);
	void buildStreamFormat();

	scopy::iio::IBufferOps *m_bufOps;
	scopy::iio::IChannelOps *m_chOps;
	scopy::iio::DeviceHandle m_dev;
	scopy::iio::ChannelsMaskHandle m_mask;
	scopy::iio::BufferHandle m_buffer;
	scopy::ICmdExecutor *m_executor;
	unsigned m_kernelBuffers = 4;
	StreamFormat m_format;
};

} // namespace scopy::component::iio
