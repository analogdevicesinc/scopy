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

QCoro::Task<CommandResponse<uint32_t>> IIORegisterReader::readAsync(uint32_t addr)
{
	auto *cmd = new scopy::iio::RegReadCommand(m_ops, m_handle, addr, this);
	return runCommand(
		m_executor, cmd, [this, addr](Result<uint32_t> &r) { Q_EMIT readSucceeded(addr, r.value()); },
		[this](const scopy::Error &error) { Q_EMIT readFailed(error); });
}
