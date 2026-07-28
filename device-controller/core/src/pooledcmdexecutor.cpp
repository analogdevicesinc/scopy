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
