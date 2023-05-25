#ifndef IIODEVICEATTRIBUTEWRITE_H
#define IIODEVICEATTRIBUTEWRITE_H

#include "command.h"
#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceAttributeWrite : public Command {
	Q_OBJECT
public:
	explicit IioDeviceAttributeWrite(struct iio_device *device,
					 const char *attribute,
					 const char *value,
					 QObject *parent)
		: m_device(device)
		, m_attribute_name(attribute)
		, m_value(value) {
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override {
		Q_EMIT started(this);
		ssize_t ret = iio_device_attr_write(m_device, m_attribute_name, m_value);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}


private:
	struct iio_device *m_device;
	const char *m_attribute_name;
	const char *m_value;
};
}

#endif // IIODEVICEATTRIBUTEWRITE_H
