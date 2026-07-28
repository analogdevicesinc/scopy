#pragma once

#include "core/command.h"
#include "iioutil/ibufferops.h"

namespace scopy::iio {

class BufferCloseCommand : public Command
{
	Q_OBJECT
public:
	BufferCloseCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: Command(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{
	}

protected:
	void run() override { m_ops->closeBuffer(m_handle); }

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
