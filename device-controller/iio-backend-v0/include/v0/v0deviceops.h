#pragma once

#include "iioutil/ideviceops.h"

namespace scopy::iio {

class V0DeviceOps : public IDeviceOps
{
public:
	QString id(DeviceHandle dev) const override;
	QString name(DeviceHandle dev) const override;
	QString label(DeviceHandle dev) const override;
	bool isTrigger(DeviceHandle dev) const override;
    bool isHwmon(DeviceHandle dev) const override;
	unsigned int channelsCount(DeviceHandle dev) const override;
	ChannelHandle getChannel(DeviceHandle dev, unsigned int index) const override;
	ChannelHandle findChannel(DeviceHandle dev, const QString &name, bool output) const override;
	unsigned int attrsCount(DeviceHandle dev) const override;
	QString attrName(DeviceHandle dev, unsigned int index) const override;
	unsigned int debugAttrsCount(DeviceHandle dev) const override;
	QString debugAttrName(DeviceHandle dev, unsigned int index) const override;
	unsigned int buffersCount(DeviceHandle dev) const override;
	unsigned int bufferAttrsCount(DeviceHandle dev, unsigned int bufferIdx) const override;
	QString bufferAttrName(DeviceHandle dev, unsigned int bufferIdx, unsigned int index) const override;
	DeviceHandle getTrigger(DeviceHandle dev) const override;
	ssize_t sampleSize(DeviceHandle dev, ChannelsMaskHandle mask) const override;

	Result<uint32_t> regRead(DeviceHandle dev, uint32_t addr) override;
	Result<void> regWrite(DeviceHandle dev, uint32_t addr, uint32_t val) override;
	Result<void> setTrigger(DeviceHandle dev, DeviceHandle trigger) override;
};

} // namespace scopy::iio
