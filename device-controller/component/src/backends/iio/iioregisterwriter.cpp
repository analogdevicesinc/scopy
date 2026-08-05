#include "component/backends/iio/iioregisterwriter.h"

#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/regwritecommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIORegisterWriter::IIORegisterWriter(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle,
				     scopy::ICmdExecutor *executor, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_executor(executor)
{
}

QCoro::Task<CommandResponse<void>> IIORegisterWriter::writeAsync(uint32_t addr, uint32_t value)
{
    auto *cmd = new scopy::iio::RegWriteCommand(m_ops, m_handle, addr, value);
	return runCommand(
		m_executor, cmd, [this, addr](Result<void> &) { Q_EMIT writeSucceeded(addr); },
		[this](const scopy::Error &error) { Q_EMIT writeFailed(error); });
}
