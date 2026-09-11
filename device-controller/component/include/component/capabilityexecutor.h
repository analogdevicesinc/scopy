/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
