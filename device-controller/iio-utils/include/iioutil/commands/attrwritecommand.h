#pragma once

#include "core/command.h"
#include "iioutil/iattrops.h"

#include <cerrno>

namespace scopy::iio {

class AttrWriteCommand : public Command
{
	Q_OBJECT
public:
	AttrWriteCommand(IAttrOps *ops, AttrHandle handle, const QString &value, QObject *parent = nullptr)
		: Command(AttrWrite, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_value(value)
    {
    }

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->write(m_handle, m_value);
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("AttrWrite(handle=%1, value=\"%2\")").arg(quintptr(m_handle.ptr)).arg(m_value);
	}

private:
	IAttrOps *m_ops;
	AttrHandle m_handle;
	QString m_value;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
