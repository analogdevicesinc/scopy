#pragma once

#include "core/command.h"
#include "iioutil/iattrops.h"

#include <cerrno>

namespace scopy::iio {

class AttrReadCommand : public Command
{
	Q_OBJECT
public:
	AttrReadCommand(IAttrOps *ops, AttrHandle handle, QObject *parent = nullptr)
		: Command(AttrRead, handle.ptr, parent)
		, m_ops(ops)
        , m_handle(handle)
    {
    }

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->read(m_handle);
		}
		Q_EMIT finished(this);
	}

	Result<QByteArray> result() const { return m_result; }

    QString toString() const override { return QStringLiteral("AttrRead(handle=%1)").arg(quintptr(m_handle.ptr)); }

private:
	IAttrOps *m_ops;
	AttrHandle m_handle;
	Result<QByteArray> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
