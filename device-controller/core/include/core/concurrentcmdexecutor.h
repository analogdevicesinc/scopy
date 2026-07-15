#pragma once

#include "core/icmdexecutor.h"
#include <QMutex>
#include <QObject>
#include <QThreadPool>

namespace scopy {

// Concurrent command executor: N-thread pool for backends that tolerate parallel
// access. Same interface as SerialCmdExecutor, no FIFO guarantee.
class ConcurrentCmdExecutor : public QObject, public ICmdExecutor
{
    Q_OBJECT
public:
	explicit ConcurrentCmdExecutor(int maxThreads = 4, QObject *parent = nullptr);
	~ConcurrentCmdExecutor() override;

	QFuture<void> execute(Command *cmd) override;
	void cancelByResource(void *resource) override;
	void cancelAll() override;
	int pendingCount() const override;

private:
	QThreadPool m_pool;
	QList<Command *> m_pending;
	mutable QMutex m_mutex;
};

} // namespace scopy
