#pragma once

#include "core/icmdexecutor.h"

#include <qcoro/qcorofuture.h>
#include <qcoro/qcorotask.h>

#include <type_traits>

namespace scopy::component {

// Dispatch `cmd` through `executor`, await it, invoke the success/failure
// callback, delete the command, and return its Result.
template <typename Command, typename OnSuccess, typename OnFailure>
auto runCommand(ICmdExecutor *executor, Command *cmd, OnSuccess onSuccess, OnFailure onFailure)
	-> QCoro::Task<std::remove_cvref_t<decltype(cmd->result())>>
{
	co_await executor->execute(cmd);
	auto result = cmd->result();
	cmd->deleteLater();

	if(result) {
		onSuccess(result);
	} else {
		onFailure(result.error());
	}

	co_return result;
}

} // namespace scopy::component
