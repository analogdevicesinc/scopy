#ifndef IIODEVICEATTRIBUTEREAD_H
#define IIODEVICEATTRIBUTEREAD_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceAttributeRead : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceAttributeRead(struct iio_device *device, const char *attribute, QObject *parent,
					bool overwrite = false)
		: m_device(device)
		, m_attribute_name(std::string(attribute))
	{
		setOverwrite(overwrite);
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~IioDeviceAttributeRead()
	{
		if(m_cmdResult->results) {
			delete[](char *) m_cmdResult->results;
			m_cmdResult->results = nullptr;
		}
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		if(!m_cmdResult->results) {
			m_cmdResult->results = new char[m_maxAttrSize];
		}
		ssize_t ret = iio_device_attr_read(m_device, m_attribute_name.c_str(), (char *)m_cmdResult->results,
						   m_maxAttrSize);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

	char *getResult() { return static_cast<char *>(m_cmdResult->results); }

private:
	struct iio_device *m_device;
	std::string m_attribute_name;
	const ssize_t m_maxAttrSize = 1024;
};
} // namespace scopy

#endif // IIODEVICEATTRIBUTEREAD_H
