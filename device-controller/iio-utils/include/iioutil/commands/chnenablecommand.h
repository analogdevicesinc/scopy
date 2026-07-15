#pragma once

#include "core/command.h"
#include "iioutil/ichannelops.h"

namespace scopy::iio {

class ChnEnableCommand : public Command {
	Q_OBJECT
public:
	ChnEnableCommand(IChannelOps *ops, ChannelHandle handle, ChannelsMaskHandle mask,
			 QObject *parent = nullptr)
		: Command(ChnEnable, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_mask(mask)
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_ops->enable(m_handle, m_mask);
		}
		Q_EMIT finished(this);
	}

	QString toString() const override
	{
		return QStringLiteral("ChnEnable(handle=%1)").arg(quintptr(m_handle.ptr));
	}

private:
	IChannelOps *m_ops;
	ChannelHandle m_handle;
	ChannelsMaskHandle m_mask;
};

} // namespace scopy::iio
