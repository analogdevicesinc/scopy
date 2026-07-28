#pragma once

#include "core/resultcommand.h"
#include "iioutil/iattrops.h"

#include <cerrno>

namespace scopy::iio {

class AttrReadCommand : public ResultCommand<QByteArray>
{
	Q_OBJECT
public:
	AttrReadCommand(IAttrOps *ops, AttrHandle handle, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
	{
	}

protected:
	void run() override { setResult(m_ops->read(m_handle)); }

private:
	IAttrOps *m_ops;
	AttrHandle m_handle;
};

} // namespace scopy::iio
