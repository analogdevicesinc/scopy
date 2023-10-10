#ifndef IIOREGISTERREAD_H
#define IIOREGISTERREAD_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioRegisterRead : public Command
{
	Q_OBJECT
public:
	explicit IioRegisterRead(struct iio_device *device, uint32_t regAddr, QObject *parent)
		: m_device(device)
		, m_registerAddress(regAddr)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~IioRegisterRead()
	{
		if(m_cmdResult->results) {
			delete[](uint32_t *) m_cmdResult->results;
			m_cmdResult->results = nullptr;
		}
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		if(!m_cmdResult->results) {
			m_cmdResult->results = (uint32_t *)malloc(sizeof(uint32_t));
		}
		ssize_t ret = iio_device_reg_read(m_device, m_registerAddress, (uint32_t *)m_cmdResult->results);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

	uint32_t getResult() { return *(uint32_t *)m_cmdResult->results; }

	uint32_t getRegisterAddress() { return m_registerAddress; }

private:
	struct iio_device *m_device;
	uint32_t m_registerAddress;
	const ssize_t m_maxAttrSize = 1024;
};
} // namespace scopy

#endif
