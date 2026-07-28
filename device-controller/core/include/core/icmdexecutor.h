#pragma once

#include <QFuture>
#include <QUuid>

namespace scopy {

class Command;

class ICmdExecutor
{
public:
	virtual ~ICmdExecutor() = default;

	virtual QFuture<void> execute(Command *cmd) = 0;

	virtual void cancelById(const QUuid &id) = 0;
	virtual void cancelByResource(void *resource) = 0;
	virtual void cancelAll() = 0;
	virtual int pendingCount() const = 0;
};

} // namespace scopy
