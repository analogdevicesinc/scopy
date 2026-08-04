#include "component/backends/iio/iioping.h"

#include "component/capabilityexecutor.h"
#include "iioutil/icontextops.h"
#include "iioutil/commands/pingcommand.h"

using namespace scopy;
using namespace scopy::component::iio;

QCoro::Task<CommandResponse<void>> IIOPing::checkReachableAsync()
{
	auto *ops = m_ops;
	auto handle = m_handle;
	auto *cmd = new scopy::iio::PingCommand(handle, [ops, handle]() { return ops->ping(handle); });
	return runCommand(
		m_executor, cmd, [this](const Result<void> &) { Q_EMIT reachabilityChecked(true); },
		[this](const Error &) { Q_EMIT reachabilityChecked(false); });
}
