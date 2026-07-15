#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

namespace scopy::iio {

class BufferCancelCommand : public Command
{
	Q_OBJECT
public:
	BufferCancelCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: Command(BufferCancel, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
    {
    }

    void execute() override
	{
        Q_EMIT started(this);
		if(!m_cancelled) {
			m_ops->cancelBuffer(m_handle);
		}
		Q_EMIT finished(this);
	}

	QString toString() const override
	{
		return QStringLiteral("BufferCancel(handle=%1)").arg(quintptr(m_handle.ptr));
	}

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
