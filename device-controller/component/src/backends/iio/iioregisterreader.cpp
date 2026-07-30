#include "component/backends/iio/iioregisterreader.h"

#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/regreadcommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIORegisterReader::IIORegisterReader(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle,
				     scopy::ICmdExecutor *executor, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_executor(executor)
{
}

QCoro::Task<Result<uint32_t>> IIORegisterReader::readInternal(scopy::iio::RegReadCommand *cmd, uint32_t addr)
{
	return runCommand(
		m_executor, cmd, [this, addr](Result<uint32_t> &r) { Q_EMIT readSucceeded(addr, r.value()); },
		[this](const scopy::Error &error) { Q_EMIT readFailed(error); });
}

QUuid IIORegisterReader::readAsync(uint32_t addr)
{
	auto *cmd = new scopy::iio::RegReadCommand(m_ops, m_handle, addr, this);
	const QUuid id = cmd->id();
	readInternal(cmd, addr);
	return id;
}

Result<uint32_t> IIORegisterReader::read(uint32_t addr)
{
	return QCoro::waitFor(readInternal(new scopy::iio::RegReadCommand(m_ops, m_handle, addr), addr));
}
