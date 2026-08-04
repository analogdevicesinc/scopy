#pragma once

#include "core/resultcommand.h"
#include "iioutil/ibufferops.h"

namespace scopy::iio {

class BufferCloseCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	BufferCloseCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{
	}

protected:
    void run() override
	{
		m_ops->closeBuffer(m_handle);
		setResult(Result<void>());
	}

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
