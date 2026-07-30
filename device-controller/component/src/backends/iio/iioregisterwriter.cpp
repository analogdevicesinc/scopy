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

QCoro::Task<Result<void>> IIORegisterWriter::writeInternal(scopy::iio::RegWriteCommand *cmd, uint32_t addr)
{
	return runCommand(
		m_executor, cmd, [this, addr](Result<void> &) { Q_EMIT writeSucceeded(addr); },
		[this](const scopy::Error &error) { Q_EMIT writeFailed(error); });
}

QUuid IIORegisterWriter::writeAsync(uint32_t addr, uint32_t value)
{
	auto *cmd = new scopy::iio::RegWriteCommand(m_ops, m_handle, addr, value, this);
	const QUuid id = cmd->id();
	writeInternal(cmd, addr);
	return id;
}

Result<void> IIORegisterWriter::write(uint32_t addr, uint32_t value)
{
	return QCoro::waitFor(writeInternal(new scopy::iio::RegWriteCommand(m_ops, m_handle, addr, value), addr));
}
