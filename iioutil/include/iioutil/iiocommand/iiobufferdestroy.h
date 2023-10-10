#ifndef IIOBUFFERDESTROY_H
#define IIOBUFFERDESTROY_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioBufferDestroy : public Command
{
	Q_OBJECT
public:
	explicit IioBufferDestroy(struct iio_buffer *buffer, QObject *parent)
		: m_buffer(buffer)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		m_cmdResult->errorCode = 0;
		iio_buffer_destroy(m_buffer);
		if(!m_buffer) {
			m_cmdResult->errorCode = -errno;
		}
		Q_EMIT finished(this);
	}

private:
	struct iio_buffer *m_buffer;
};
} // namespace scopy

#endif // IIOBUFFERDESTROY_H
