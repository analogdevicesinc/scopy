#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

namespace scopy::iio {

class BufferCloseCommand : public Command
{
	Q_OBJECT
public:
	BufferCloseCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: Command(BufferDestroy, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
    {
    }

    void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_ops->closeBuffer(m_handle);
        }
		Q_EMIT finished(this);
	}

	QString toString() const override
	{
		return QStringLiteral("BufferClose(handle=%1)").arg(quintptr(m_handle.ptr));
	}

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
