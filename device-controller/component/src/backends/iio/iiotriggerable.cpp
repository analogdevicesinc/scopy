#include "component/backends/iio/iiotriggerable.h"

#include "component/backends/iio/iiotrigger.h"
#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/settriggercommand.h"
#include "iioutil/ideviceops.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOTriggerable::IIOTriggerable(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, scopy::ICmdExecutor *executor,
			       QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_dev(dev)
	, m_executor(executor)
{
}

bool IIOTriggerable::hasTrigger() const { return m_ops->getTrigger(m_dev).ptr != nullptr; }

QString IIOTriggerable::assignedTriggerName() const
{
	const auto t = m_ops->getTrigger(m_dev);
	return t.ptr ? m_ops->name(t) : QString();
}

QCoro::Task<CommandResponse<void>> IIOTriggerable::setInternal(scopy::iio::SetTriggerCommand *cmd)
{
	return runCommand(
		m_executor, cmd, [this](Result<void> &) { Q_EMIT triggerSucceeded(); },
		[this](const scopy::Error &error) { Q_EMIT triggerFailed(error); });
}

// Only an IIOTrigger (an is_trigger device) can back an assignment; a null source
// clears it.
static scopy::iio::DeviceHandle handleOf(IIOTrigger *source)
{
	return source ? source->handle() : scopy::iio::DeviceHandle{};
}

QCoro::Task<CommandResponse<void>> IIOTriggerable::setTriggerAsync(IIOTrigger *source)
{
    return setInternal(new scopy::iio::SetTriggerCommand(m_ops, m_dev, handleOf(source)));
}

QCoro::Task<CommandResponse<void>> IIOTriggerable::clearTriggerAsync()
{
    return setInternal(new scopy::iio::SetTriggerCommand(m_ops, m_dev, {}));
}
