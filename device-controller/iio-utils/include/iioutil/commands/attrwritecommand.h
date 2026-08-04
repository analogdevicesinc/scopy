#pragma once

#include "core/resultcommand.h"
#include "iioutil/iattrops.h"

#include <cerrno>

namespace scopy::iio {

class AttrWriteCommand : public ResultCommand<void>
{
    Q_OBJECT
public:
	AttrWriteCommand(IAttrOps *ops, AttrHandle handle, const QString &value, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_value(value)
	{
	}

protected:
    void run() override { setResult(m_ops->write(m_handle, m_value)); }

private:
	IAttrOps *m_ops;
	AttrHandle m_handle;
	QString m_value;
};

} // namespace scopy::iio
