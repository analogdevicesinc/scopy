#ifndef IIODEVICEGETTRIGGER_H
#define IIODEVICEGETTRIGGER_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceGetTrigger : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceGetTrigger(struct iio_device *device, QObject *parent)
		: m_device(device)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		const iio_device *test_dev;
		ssize_t ret = iio_device_get_trigger(m_device, &test_dev);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

	iio_device *getResult() { return static_cast<iio_device *>(m_cmdResult->results); }

private:
	struct iio_device *m_device;
};
} // namespace scopy

#endif // IIODEVICEGETTRIGGER_H
