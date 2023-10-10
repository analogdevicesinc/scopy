#ifndef IIODEVICESETTRIGGER_H
#define IIODEVICESETTRIGGER_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceSetTrigger : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceSetTrigger(struct iio_device *device, struct iio_device *trigger_device, QObject *parent)
		: m_device(device)
		, m_trigger_device(trigger_device)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		ssize_t ret = iio_device_set_trigger(m_device, m_trigger_device);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

private:
	struct iio_device *m_device;
	struct iio_device *m_trigger_device;
};
} // namespace scopy

#endif // IIODEVICESETTRIGGER_H
