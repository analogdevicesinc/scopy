#pragma once

#include "component/outputstream.h"
#include "iioutil/handles.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IBufferOps;
class IChannelOps;
class BufferPushCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOOutputStream : public OutputStream
{
	Q_OBJECT
	Q_PROPERTY(unsigned kernelBuffers READ kernelBuffers CONSTANT)
	Q_PROPERTY(bool cyclic READ cyclic CONSTANT)
public:
	IIOOutputStream(scopy::iio::IBufferOps *bufOps, scopy::iio::IChannelOps *chOps, scopy::iio::DeviceHandle dev,
			int nbChannels, unsigned bufferIndex, scopy::ICmdExecutor *executor, QObject *parent = nullptr);
	~IIOOutputStream() override;

	scopy::iio::ChannelsMaskHandle mask() const { return m_mask; }
	unsigned kernelBuffers() const { return m_kernelBuffers; }
	bool cyclic() const { return m_cyclic; }
	Result<void> setKernelBuffers(unsigned n);
	Result<void> setCyclic(bool cyclic);

	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	StreamFormat &writeFormat() override { return m_format; }
	QCoro::Task<CommandResponse<void>> pushAsync() override;

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
	bool m_cyclic = false;
	StreamFormat m_format;
};

} // namespace scopy::component::iio
