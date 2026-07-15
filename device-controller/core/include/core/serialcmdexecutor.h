#pragma once

#include "core/icmdexecutor.h"
#include <QMutex>
#include <QObject>
#include <QThreadPool>

namespace scopy {

// Serial command executor: single-thread pool, strict FIFO. The default executor
// — serializes all hardware operations on one connection.
class SerialCmdExecutor : public QObject, public ICmdExecutor
{
	Q_OBJECT
public:
	explicit SerialCmdExecutor(QObject *parent = nullptr);
	~SerialCmdExecutor() override;

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
