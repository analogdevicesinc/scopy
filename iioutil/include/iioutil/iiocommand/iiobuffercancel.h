#ifndef IIOBUFFERCANCEL_H
#define IIOBUFFERCANCEL_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioBufferCancel : public Command
{
	Q_OBJECT
public:
	explicit IioBufferCancel(struct iio_buffer *buffer, QObject *parent)
		: m_buffer(buffer)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		iio_buffer_cancel(m_buffer);
		m_cmdResult->errorCode = 0;
		Q_EMIT finished(this);
	}

private:
	struct iio_buffer *m_buffer;
};
} // namespace scopy

#endif // IIOBUFFERCANCEL_H
