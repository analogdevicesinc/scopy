#include "core/concurrentcmdexecutor.h"
#include "core/command.h"

namespace scopy {

ConcurrentCmdExecutor::ConcurrentCmdExecutor(int maxThreads, QObject *parent)
	: QObject(parent)
{
	m_pool.setMaxThreadCount(maxThreads);
}

ConcurrentCmdExecutor::~ConcurrentCmdExecutor()
{
	ConcurrentCmdExecutor::cancelAll();
	m_pool.waitForDone();
	QMutexLocker lock(&m_mutex);
	qDeleteAll(m_pending);
}

QFuture<void> ConcurrentCmdExecutor::execute(Command *cmd)
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

void ConcurrentCmdExecutor::cancelByResource(void *resource)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		if(cmd->resource() == resource) {
			cmd->cancel();
		}
	}
}

void ConcurrentCmdExecutor::cancelAll()
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		cmd->cancel();
	}
}

int ConcurrentCmdExecutor::pendingCount() const
{
	QMutexLocker lock(&m_mutex);
	return m_pending.size();
}

} // namespace scopy
