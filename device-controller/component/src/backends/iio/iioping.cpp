#include "component/backends/iio/iioping.h"

#include "core/icmdexecutor.h"
#include "iioutil/icontextops.h"
#include "iioutil/commands/pingcommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

QCoro::Task<bool> IIOPing::checkInternal()
{
	auto *ops = m_ops;
	auto handle = m_handle;
	auto *cmd = new scopy::iio::PingCommand(handle, [ops, handle]() { return ops->ping(handle); });
	co_await m_executor->execute(cmd);
	Result<void> r = cmd->result();
	cmd->deleteLater();
	const bool reachable = bool(r);
	Q_EMIT reachabilityChecked(reachable);
	co_return reachable;
}

bool IIOPing::checkReachable() { return QCoro::waitFor(checkInternal()); }

void IIOPing::checkReachableAsync() { checkInternal(); }
