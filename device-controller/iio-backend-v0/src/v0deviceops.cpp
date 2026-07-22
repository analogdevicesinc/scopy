#include "v0deviceops.h"
#include <iio.h>

namespace scopy::iio {

static iio_device *dev(DeviceHandle h) { return static_cast<iio_device *>(h.ptr); }

QString V0DeviceOps::id(DeviceHandle h) const
{
	const char *s = iio_device_get_id(dev(h));
	return s ? QString::fromUtf8(s) : QString();
}

QString V0DeviceOps::name(DeviceHandle h) const
{
	const char *s = iio_device_get_name(dev(h));
	return s ? QString::fromUtf8(s) : QString();
}

QString V0DeviceOps::label(DeviceHandle h) const
{
	const char *s = iio_device_get_label(dev(h));
	return s ? QString::fromUtf8(s) : QString();
}

bool V0DeviceOps::isTrigger(DeviceHandle h) const { return iio_device_is_trigger(dev(h)); }

bool V0DeviceOps::isHwmon(DeviceHandle h) const { return iio_device_is_hwmon(dev(h)); }

unsigned int V0DeviceOps::channelsCount(DeviceHandle h) const { return iio_device_get_channels_count(dev(h)); }

ChannelHandle V0DeviceOps::getChannel(DeviceHandle h, unsigned int index) const
{
	return {const_cast<iio_channel *>(iio_device_get_channel(dev(h), index))};
}

ChannelHandle V0DeviceOps::findChannel(DeviceHandle h, const QString &name, bool output) const
{
	return {const_cast<iio_channel *>(iio_device_find_channel(dev(h), name.toUtf8().constData(), output))};
}

unsigned int V0DeviceOps::attrsCount(DeviceHandle h) const { return iio_device_get_attrs_count(dev(h)); }

QString V0DeviceOps::attrName(DeviceHandle h, unsigned int index) const
{
	const char *s = iio_device_get_attr(dev(h), index);
	return s ? QString::fromUtf8(s) : QString();
}

unsigned int V0DeviceOps::debugAttrsCount(DeviceHandle h) const { return iio_device_get_debug_attrs_count(dev(h)); }

QString V0DeviceOps::debugAttrName(DeviceHandle h, unsigned int index) const
{
	const char *s = iio_device_get_debug_attr(dev(h), index);
	return s ? QString::fromUtf8(s) : QString();
}

unsigned int V0DeviceOps::buffersCount(DeviceHandle /*h*/) const { return 1; }

unsigned int V0DeviceOps::bufferAttrsCount(DeviceHandle h, unsigned int /*bufferIdx*/) const
{
	return iio_device_get_buffer_attrs_count(dev(h));
}

QString V0DeviceOps::bufferAttrName(DeviceHandle h, unsigned int /*bufferIdx*/, unsigned int index) const
{
	const char *s = iio_device_get_buffer_attr(dev(h), index);
	return s ? QString::fromUtf8(s) : QString();
}

DeviceHandle V0DeviceOps::getTrigger(DeviceHandle h) const
{
	const iio_device *trigger = nullptr;
	int ret = iio_device_get_trigger(dev(h), &trigger);
	if(ret < 0) {
		return {};
	}
	return {const_cast<iio_device *>(trigger)};
}

ssize_t V0DeviceOps::sampleSize(DeviceHandle h, ChannelsMaskHandle /*mask*/) const
{
	return iio_device_get_sample_size(dev(h));
}

Result<uint32_t> V0DeviceOps::regRead(DeviceHandle h, uint32_t addr)
{
	uint32_t val = 0;
	int ret = iio_device_reg_read(dev(h), addr, &val);
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_device_reg_read failed @0x%1").arg(addr, 0, 16)}};
	}
	return val;
}

Result<void> V0DeviceOps::regWrite(DeviceHandle h, uint32_t addr, uint32_t val)
{
	int ret = iio_device_reg_write(dev(h), addr, val);
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_device_reg_write failed @0x%1").arg(addr, 0, 16)}};
	}
	return {};
}

Result<void> V0DeviceOps::setTrigger(DeviceHandle h, DeviceHandle trigger)
{
	int ret = iio_device_set_trigger(dev(h), trigger.ptr ? dev(trigger) : nullptr);
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_device_set_trigger failed")}};
	}
	return {};
}

} // namespace scopy::iio
