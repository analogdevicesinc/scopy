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
#include <QList>
#include <QMutex>
#include <QObject>
#include <QThreadPool>

namespace scopy {

class Command;

// Pool-based command executor. maxThreads == 1 gives strict FIFO (the default,
// serializing all hardware operations on one connection); maxThreads > 1 allows
// parallel access for backends that tolerate it (no ordering guarantee).
class PooledCmdExecutor : public QObject, public ICmdExecutor
{
	Q_OBJECT
public:
	explicit PooledCmdExecutor(int maxThreads = 1, QObject *parent = nullptr);
	~PooledCmdExecutor() override;

	QFuture<void> execute(Command *cmd) override;

	void cancelById(const QUuid &id) override;
	void cancelByResource(void *resource) override;
	void cancelAll() override;
	int pendingCount() const override;

private:
	QThreadPool m_pool;
	QList<Command *> m_pending;
	mutable QMutex m_mutex;
};

} // namespace scopy
