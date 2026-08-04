#include "component/backends/iio/iioattributewriter.h"

#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/attrwritecommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOAttributeWriter::IIOAttributeWriter(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle,
				       scopy::ICmdExecutor *executor, QObject *parent)
	: AttributeWriter(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_executor(executor)
{
}

QCoro::Task<CommandResponse<void>> IIOAttributeWriter::writeAsync(const QString &value)
{
    auto *cmd = new scopy::iio::AttrWriteCommand(m_ops, m_handle, value, this);
    return runCommand(
        m_executor, cmd, [this](const Result<void> &) { Q_EMIT writeSucceeded(); },
        [this](const scopy::Error &error) { Q_EMIT writeFailed(error); });
}
