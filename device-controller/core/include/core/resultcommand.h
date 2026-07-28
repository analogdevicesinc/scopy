#pragma once

#include "core/command.h"
#include "core/result.h"

#include <cerrno>

namespace scopy {

template <typename T>
class ResultCommand : public Command
{
public:
	using ResultType = Result<T>;

	ResultCommand(void *resource, QObject *parent = nullptr)
		: Command(resource, parent)
	{
	}

	const ResultType &result() const { return m_result; }

protected:
	void setResult(ResultType result) { m_result = std::move(result); }

	ResultType m_result{Unexpected{Error{-ECANCELED, "command cancelled"}}};
};

} // namespace scopy
