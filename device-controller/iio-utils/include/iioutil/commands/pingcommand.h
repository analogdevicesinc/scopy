#pragma once

#include "core/resultcommand.h"
#include "iioutil/handles.h"

#include <cerrno>
#include <functional>

namespace scopy::iio {

class PingCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	PingCommand(ContextHandle handle, std::function<bool()> fn, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_fn(std::move(fn))
	{
	}

protected:
	void run() override
	{
        setResult(m_fn() ? Result<void>()
                 : Result<void>(Unexpected{Error{-ENODEV, QStringLiteral("ping failed")}}));
	}

private:
	std::function<bool()> m_fn;
};

} // namespace scopy::iio
