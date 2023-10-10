#ifndef IIOCHANNELENABLE_H
#define IIOCHANNELENABLE_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioChannelEnable : public Command
{
	Q_OBJECT
public:
	explicit IioChannelEnable(struct iio_channel *channel, bool enable, QObject *parent)
		: m_channel(channel)
		, m_enable(enable)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		if(m_enable) {
			iio_channel_enable(m_channel);
		} else {
			iio_channel_disable(m_channel);
		}
		m_cmdResult->errorCode = 0;
		Q_EMIT finished(this);
	}

private:
	struct iio_channel *m_channel;
	const char *m_attribute_name;
	bool m_enable;
};
} // namespace scopy

#endif // IIOCHANNELENABLE_H
