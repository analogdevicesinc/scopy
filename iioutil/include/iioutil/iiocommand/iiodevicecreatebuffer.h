#ifndef IIODEVICECREATEBUFFER_H
#define IIODEVICECREATEBUFFER_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceCreateBuffer : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceCreateBuffer(struct iio_device *device, size_t samples_count, bool cyclic, QObject *parent)
		: m_device(device)
		, m_samplesCount(samples_count)
		, m_cyclic(cyclic)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		struct iio_buffer *buffer = iio_device_create_buffer(m_device, m_samplesCount, m_cyclic);
		if(!buffer) {
			m_cmdResult->errorCode = -errno;
		} else {
			m_cmdResult->errorCode = 0;
		}
		m_cmdResult->results = buffer;
		Q_EMIT finished(this);
	}

	iio_buffer *getResult() { return static_cast<iio_buffer *>(m_cmdResult->results); }

private:
	struct iio_device *m_device;
	ssize_t m_samplesCount;
	bool m_cyclic;
};
} // namespace scopy

#endif // IIODEVICECREATEBUFFER_H
