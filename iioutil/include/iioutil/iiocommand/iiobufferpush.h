#ifndef IIOBUFFERPUSH_H
#define IIOBUFFERPUSH_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioBufferPush : public Command
{
	Q_OBJECT
public:
	explicit IioBufferPush(struct iio_buffer *buffer, QObject *parent)
		: m_buffer(buffer)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		ssize_t ret = iio_buffer_push(m_buffer);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

private:
	struct iio_buffer *m_buffer;
};
} // namespace scopy

#endif // IIOBUFFERPUSH_H
