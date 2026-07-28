#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

namespace scopy::iio {

class BufferCancelCommand : public Command
{
	Q_OBJECT
public:
	BufferCancelCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: Command(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{
	}

protected:
	void run() override { m_ops->cancelBuffer(m_handle); }

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
