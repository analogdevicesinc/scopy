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

#include <QFuture>
#include <QUuid>

#include <memory>

namespace scopy {

class Command;
class TraceConfig;

class ICmdExecutor
{
public:
	virtual ~ICmdExecutor() = default;

	virtual QFuture<void> execute(Command *cmd) = 0;

	virtual void cancelById(const QUuid &id) = 0;
	virtual void cancelByResource(void *resource) = 0;
	virtual void cancelAll() = 0;
	virtual int pendingCount() const = 0;

	// Call trace of the commands this executor runs (core/calltrace.h); nullptr = never record.
	// Executors that do not trace keep the defaults.
	virtual void setTraceConfig(std::shared_ptr<TraceConfig> config) { (void)config; }
	virtual std::shared_ptr<TraceConfig> traceConfig() const { return {}; }
};

} // namespace scopy
