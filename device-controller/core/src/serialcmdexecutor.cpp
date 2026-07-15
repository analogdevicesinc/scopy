#include "core/serialcmdexecutor.h"
#include "core/command.h"
#include <QtConcurrent>

namespace scopy {

SerialCmdExecutor::SerialCmdExecutor(QObject *parent)
	: QObject(parent)
{
	m_pool.setMaxThreadCount(1);
}

SerialCmdExecutor::~SerialCmdExecutor()
{
	SerialCmdExecutor::cancelAll();
	m_pool.waitForDone();
	QMutexLocker lock(&m_mutex);
	qDeleteAll(m_pending);
}

QFuture<void> SerialCmdExecutor::execute(Command *cmd)
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

void SerialCmdExecutor::cancelByResource(void *resource)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		if(cmd->resource() == resource) {
			cmd->cancel();
		}
	}
}

void SerialCmdExecutor::cancelAll()
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_pending)) {
		cmd->cancel();
	}
}

int SerialCmdExecutor::pendingCount() const
{
	QMutexLocker lock(&m_mutex);
	return m_pending.size();
}

} // namespace scopy
