#pragma once

#include "core/resultcommand.h"
#include "iioutil/ideviceops.h"

#include <cerrno>
#include <cstdint>

namespace scopy::iio {

class RegReadCommand : public ResultCommand<uint32_t>
{
	Q_OBJECT
public:
	RegReadCommand(IDeviceOps *ops, DeviceHandle handle, uint32_t addr, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_addr(addr)
	{
	}

protected:
	void run() override { setResult(m_ops->regRead(m_handle, m_addr)); }

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	uint32_t m_addr;
};

} // namespace scopy::iio
