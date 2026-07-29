#include "component/backends/iio/iioattributereader.h"

#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/attrreadcommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOAttributeReader::IIOAttributeReader(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle,
				       scopy::ICmdExecutor *executor, QObject *parent)
	: AttributeReader(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_executor(executor)
{
}

QCoro::Task<Result<QByteArray>> IIOAttributeReader::readInternal(scopy::iio::AttrReadCommand *cmd)
{
	return runCommand(
		m_executor, cmd, [this](Result<QByteArray> &r) { Q_EMIT readSucceeded(r); },
		[this](const scopy::Error &error) { Q_EMIT readFailed(error); });
}

QUuid IIOAttributeReader::readAsync()
{
	auto *cmd = new scopy::iio::AttrReadCommand(m_ops, m_handle, this);
	const QUuid id = cmd->id();
	readInternal(cmd);
	return id;
}

Result<QByteArray> IIOAttributeReader::read()
{
	return QCoro::waitFor(readInternal(new scopy::iio::AttrReadCommand(m_ops, m_handle)));
}
