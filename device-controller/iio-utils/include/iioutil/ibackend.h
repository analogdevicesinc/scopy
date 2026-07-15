#pragma once

#include "iioutil/handles.h"

namespace scopy::iio {

class IContextOps;
class IDeviceOps;
class IChannelOps;
class IAttrOps;
class IBufferOps;
class IScanOps;

class IBackend {
public:
	virtual ~IBackend() = default;

	virtual LibiioVersion version() const = 0;

	virtual IContextOps *contextOps() = 0;
	virtual IDeviceOps *deviceOps() = 0;
	virtual IChannelOps *channelOps() = 0;
	virtual IAttrOps *attrOps() = 0;
	virtual IBufferOps *bufferOps() = 0;
	virtual IScanOps *scanOps() = 0;
};

} // namespace scopy::iio
