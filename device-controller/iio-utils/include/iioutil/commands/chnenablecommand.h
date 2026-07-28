#pragma once

#include "core/command.h"
#include "iioutil/ichannelops.h"

namespace scopy::iio {

class ChnEnableCommand : public Command
{
	Q_OBJECT
public:
	ChnEnableCommand(IChannelOps *ops, ChannelHandle handle, ChannelsMaskHandle mask, QObject *parent = nullptr)
		: Command(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_mask(mask)
	{
	}

protected:
	void run() override { m_ops->enable(m_handle, m_mask); }

private:
	IChannelOps *m_ops;
	ChannelHandle m_handle;
	ChannelsMaskHandle m_mask;
};

} // namespace scopy::iio
