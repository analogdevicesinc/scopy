#ifndef IIOCHANNELATTRIBUTEWRITE_H
#define IIOCHANNELATTRIBUTEWRITE_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioChannelAttributeWrite : public Command
{
	Q_OBJECT
public:
	explicit IioChannelAttributeWrite(struct iio_channel *channel, const char *attribute, const char *value,
					  QObject *parent, bool overwrite = false)
		: m_channel(channel)
		, m_attribute_name(std::string(attribute))
		, m_value(std::string(value))
	{
		setOverwrite(overwrite);
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		ssize_t ret = iio_channel_attr_write(m_channel, m_attribute_name.c_str(), m_value.c_str());
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

private:
	struct iio_channel *m_channel;
	std::string m_attribute_name;
	std::string m_value;
};
} // namespace scopy

#endif // IIOCHANNELATTRIBUTEWRITE_H
