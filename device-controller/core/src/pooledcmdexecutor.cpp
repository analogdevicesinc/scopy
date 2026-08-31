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

#include "core/pooledcmdexecutor.h"
#include "core/command.h"
#include <QtConcurrent>

namespace scopy {

PooledCmdExecutor::PooledCmdExecutor(int maxThreads, QObject *parent)
	: QObject(parent)
{
	m_pool.setMaxThreadCount(maxThreads);
}

PooledCmdExecutor::~PooledCmdExecutor()
{
	PooledCmdExecutor::cancelAll();
	m_pool.waitForDone();
	QMutexLocker lock(&m_mutex);
	qDeleteAll(m_pending);
}

QFuture<void> PooledCmdExecutor::execute(Command *cmd)
{
	if(!cmd) {
		return {};
	}
	{
		QMutexLocker lock(&m_mutex);
		m_pending.append(cmd);
	}
	auto promise = std::make_shared<QPromise<void>>();
	promise->start();

	m_pool.start([this, cmd, promise]() {
		{
			QMutexLocker lock(&m_mutex);
			if(cmd) {
				m_pending.removeOne(cmd);
			}
		}
		if(!cmd->isCancelled()) {
			cmd->execute();
		}
		promise->finish();
	});
	return promise->future();
}

void PooledCmdExecutor::cancelById(const QUuid &id)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		if(cmd->id() == id) {
			cmd->cancel();
			break;
		}
	}
}

void PooledCmdExecutor::cancelByResource(void *resource)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		if(cmd->resource() == resource) {
			cmd->cancel();
		}
	}
}

void PooledCmdExecutor::cancelAll()
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		cmd->cancel();
	}
}

int PooledCmdExecutor::pendingCount() const
{
	QMutexLocker lock(&m_mutex);
	return m_pending.size();
}

} // namespace scopy
