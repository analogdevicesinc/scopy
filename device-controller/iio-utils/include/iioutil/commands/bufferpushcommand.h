#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

#include <cerrno>

namespace scopy::iio {

class BufferPushCommand : public Command {
	Q_OBJECT
public:
	BufferPushCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: Command(BufferPush, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->push(m_handle);
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("BufferPush(handle=%1)").arg(quintptr(m_handle.ptr));
	}

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
