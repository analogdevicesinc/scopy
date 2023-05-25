#ifndef IIOCHANNELATTRIBUTEWRITE_H
#define IIOCHANNELATTRIBUTEWRITE_H

#include "command.h"
#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioChannelAttributeWrite : public Command {
	Q_OBJECT
public:
	explicit IioChannelAttributeWrite(struct iio_channel *channel,
					  const char *attribute,
					  const char *value,
					  QObject *parent)
		: m_channel(channel)
		, m_attribute_name(attribute)
		, m_value(value) {
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override {
		Q_EMIT started(this);
		ssize_t ret = iio_channel_attr_write(m_channel, m_attribute_name, m_value);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

private:
	struct iio_channel *m_channel;
	const char *m_attribute_name;
	const char *m_value;
};
}

#endif // IIOCHANNELATTRIBUTEWRITE_H
