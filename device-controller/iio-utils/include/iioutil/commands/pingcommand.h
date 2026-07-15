#pragma once

#include "core/command.h"
#include "iioutil/handles.h"

#include <cerrno>
#include <functional>

namespace scopy::iio {

class PingCommand : public Command {
	Q_OBJECT
public:
	PingCommand(ContextHandle handle, std::function<bool()> fn, QObject *parent = nullptr)
		: Command(Ping, handle.ptr, parent)
		, m_fn(std::move(fn))
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_fn() ? Result<void>()
					  : Result<void>(Unexpected{Error{-ENODEV, QStringLiteral("ping failed")}});
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("Ping(ctx=%1)").arg(quintptr(m_resource));
	}

private:
	std::function<bool()> m_fn;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
