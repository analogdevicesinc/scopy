#include "component/backends/iio/iioscanelement.h"

#include "core/icmdexecutor.h"
#include "iioutil/commands/chndisablecommand.h"
#include "iioutil/commands/chnenablecommand.h"
#include "iioutil/ichannelops.h"

#include <cerrno>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOScanElement::IIOScanElement(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle,
			       scopy::iio::ChannelsMaskHandle mask, scopy::ICmdExecutor *executor, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_mask(mask)
	, m_executor(executor)
{
}

Command *IIOScanElement::makeCommand(bool en)
{
	return en ? static_cast<Command *>(new scopy::iio::ChnEnableCommand(m_ops, m_handle, m_mask))
		  : static_cast<Command *>(new scopy::iio::ChnDisableCommand(m_ops, m_handle, m_mask));
}

QCoro::Task<Result<void>> IIOScanElement::enableInternal(Command *cmd, bool en)
{
	const bool was = m_ops->isEnabled(m_handle, m_mask);
	co_await m_executor->execute(cmd);
	cmd->deleteLater();

	const bool now = m_ops->isEnabled(m_handle, m_mask);
	if(now != was) {
		Q_EMIT enabledChanged(now);
	}
	if(now == en) {
		co_return Result<void>();
	}
	Result<void> err{Unexpected{Error{-EIO, QStringLiteral("channel mask did not reach requested state")}}};
	Q_EMIT enableFailed(err.error());
	co_return err;
}

QUuid IIOScanElement::enableAsync(bool en)
{
	auto *cmd = makeCommand(en);
	const QUuid id = cmd->id();
	enableInternal(cmd, en);
	return id;
}

Result<void> IIOScanElement::enable(bool en) { return QCoro::waitFor(enableInternal(makeCommand(en), en)); }

bool IIOScanElement::isEnabled() const { return m_ops->isEnabled(m_handle, m_mask); }
