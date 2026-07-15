#pragma once

#include "iioutil/handles.h"
#include "core/result.h"
#include <QString>
#include <cstdint>

namespace scopy::iio {

class IDeviceOps {
public:
	virtual ~IDeviceOps() = default;

	// Metadata — infallible, return directly
	virtual QString id(DeviceHandle dev) const = 0;
	virtual QString name(DeviceHandle dev) const = 0;
	virtual QString label(DeviceHandle dev) const = 0;
	virtual bool isTrigger(DeviceHandle dev) const = 0;
	virtual bool isHwmon(DeviceHandle dev) const = 0;
	virtual unsigned int channelsCount(DeviceHandle dev) const = 0;
	virtual ChannelHandle getChannel(DeviceHandle dev, unsigned int index) const = 0;
	virtual ChannelHandle findChannel(DeviceHandle dev, const QString &name, bool output) const = 0;
	virtual unsigned int attrsCount(DeviceHandle dev) const = 0;
	virtual QString attrName(DeviceHandle dev, unsigned int index) const = 0;
	virtual unsigned int debugAttrsCount(DeviceHandle dev) const = 0;
	virtual QString debugAttrName(DeviceHandle dev, unsigned int index) const = 0;
	// v0: always 1. v1: iio_device_get_buffers_count(dev).
	virtual unsigned int buffersCount(DeviceHandle dev) const = 0;
	// NOTE: Buffer attr location differs between libiio versions.
	// v0: buffer attrs belong to the device; bufferIdx is ignored.
	// v1: buffer attrs belong to the iio_buffer* object — enumerated per buffer index.
	virtual unsigned int bufferAttrsCount(DeviceHandle dev, unsigned int bufferIdx) const = 0;
	virtual QString bufferAttrName(DeviceHandle dev, unsigned int bufferIdx, unsigned int index) const = 0;
	virtual DeviceHandle getTrigger(DeviceHandle dev) const = 0;
	virtual ssize_t sampleSize(DeviceHandle dev, ChannelsMaskHandle mask) const = 0;

	// I/O — returns a typed Result; register read carries the value directly.
	virtual Result<uint32_t> regRead(DeviceHandle dev, uint32_t addr) = 0;
	virtual Result<void> regWrite(DeviceHandle dev, uint32_t addr, uint32_t val) = 0;
	virtual Result<void> setTrigger(DeviceHandle dev, DeviceHandle trigger) = 0;
};

} // namespace scopy::iio
