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
