#pragma once

#include "component/inputstream.h"
#include "iioutil/handles.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IBufferOps;
class IChannelOps;
class BufferRefillCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOInputStream : public InputStream
{
	Q_OBJECT
	Q_PROPERTY(unsigned kernelBuffers READ kernelBuffers CONSTANT)
public:
	IIOInputStream(scopy::iio::IBufferOps *bufOps, scopy::iio::IChannelOps *chOps, scopy::iio::DeviceHandle dev,
		       int nbChannels, unsigned bufferIndex, scopy::ICmdExecutor *executor, QObject *parent = nullptr);
	~IIOInputStream() override;

	scopy::iio::ChannelsMaskHandle mask() const { return m_mask; }
	unsigned kernelBuffers() const { return m_kernelBuffers; }
	Result<void> setKernelBuffers(unsigned n);

	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	QCoro::Task<CommandResponse<void>> refillAsync() override;
	const StreamFormat &readFormat() const override { return m_format; }

private:
	QCoro::Task<void> applyEnabledChannels(const QList<int> &channels);
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
