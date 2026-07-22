#pragma once

#include "iioutil/ibackend.h"
#include "v1contextops.h"
#include "v1deviceops.h"
#include "v1channelops.h"
#include "v1attrops.h"
#include "v1bufferops.h"
#include "v1scanops.h"

namespace scopy::iio {

class V1Backend : public IBackend
{
public:
	LibiioVersion version() const override { return LibiioVersion::V1; }

	IContextOps *contextOps() override { return &m_contextOps; }
	IDeviceOps *deviceOps() override { return &m_deviceOps; }
	IChannelOps *channelOps() override { return &m_channelOps; }
	IAttrOps *attrOps() override { return &m_attrOps; }
	IBufferOps *bufferOps() override { return &m_bufferOps; }
	IScanOps *scanOps() override { return &m_scanOps; }

private:
	V1ContextOps m_contextOps;
	V1DeviceOps m_deviceOps;
	V1ChannelOps m_channelOps;
	V1AttrOps m_attrOps;
	V1BufferOps m_bufferOps;
	V1ScanOps m_scanOps;
};

} // namespace scopy::iio
