#pragma once

#include "core/resultcommand.h"
#include "iioutil/ibufferops.h"

#include <cerrno>

namespace scopy::iio {

class BufferPushCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	BufferPushCommand(IBufferOps *ops, BufferHandle handle, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{
	}

protected:
	void run() override { setResult(m_ops->push(m_handle)); }

private:
	IBufferOps *m_ops;
	BufferHandle m_handle;
};

} // namespace scopy::iio
