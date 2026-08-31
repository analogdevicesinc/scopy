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

#include "component/backends/iio/iioping.h"

#include "component/capabilityexecutor.h"
#include "iioutil/icontextops.h"
#include "iioutil/commands/pingcommand.h"

using namespace scopy;
using namespace scopy::component::iio;

QCoro::Task<CommandResponse<void>> IIOPing::checkReachableAsync()
{
	auto *ops = m_ops;
	auto handle = m_handle;
	auto *cmd = new scopy::iio::PingCommand(handle, [ops, handle]() { return ops->ping(handle); });
	return runCommand(
		m_executor, cmd, [this](const Result<void> &) { Q_EMIT reachabilityChecked(true); },
		[this](const Error &) { Q_EMIT reachabilityChecked(false); });
}
