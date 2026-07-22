#include "v1deviceops.h"
#include <iio/iio.h>
#include <cerrno>

namespace scopy::iio {

static iio_device *dev(DeviceHandle h) { return static_cast<iio_device *>(h.ptr); }

QString V1DeviceOps::id(DeviceHandle h) const
{
    const char *s = iio_device_get_id(dev(h));
    return s ? QString::fromUtf8(s) : QString();
}

QString V1DeviceOps::name(DeviceHandle h) const
{
    const char *s = iio_device_get_name(dev(h));
    return s ? QString::fromUtf8(s) : QString();
}

QString V1DeviceOps::label(DeviceHandle h) const
{
    const char *s = iio_device_get_label(dev(h));
    return s ? QString::fromUtf8(s) : QString();
}

bool V1DeviceOps::isTrigger(DeviceHandle h) const { return iio_device_is_trigger(dev(h)); }

bool V1DeviceOps::isHwmon(DeviceHandle h) const { return iio_device_is_hwmon(dev(h)); }

unsigned int V1DeviceOps::channelsCount(DeviceHandle h) const { return iio_device_get_channels_count(dev(h)); }

ChannelHandle V1DeviceOps::getChannel(DeviceHandle h, unsigned int index) const
{
    return {const_cast<iio_channel *>(iio_device_get_channel(dev(h), index))};
}

ChannelHandle V1DeviceOps::findChannel(DeviceHandle h, const QString &name, bool output) const
{
    return {const_cast<iio_channel *>(iio_device_find_channel(dev(h), name.toUtf8().constData(), output))};
}

unsigned int V1DeviceOps::attrsCount(DeviceHandle h) const { return iio_device_get_attrs_count(dev(h)); }

QString V1DeviceOps::attrName(DeviceHandle h, unsigned int index) const
{
    const struct iio_attr *attr = iio_device_get_attr(dev(h), index);
    const char *s = iio_attr_get_name(attr);
    return s ? QString::fromUtf8(s) : QString();
}

unsigned int V1DeviceOps::debugAttrsCount(DeviceHandle h) const { return iio_device_get_debug_attrs_count(dev(h)); }

QString V1DeviceOps::debugAttrName(DeviceHandle h, unsigned int index) const
{
    const struct iio_attr *attr = iio_device_get_debug_attr(dev(h), index);
    const char *s = iio_attr_get_name(attr);
    return s ? QString::fromUtf8(s) : QString();
}

unsigned int V1DeviceOps::buffersCount(DeviceHandle h) const { return iio_device_get_buffers_count(dev(h)); }

unsigned int V1DeviceOps::bufferAttrsCount(DeviceHandle h, unsigned int bufferIdx) const
{
    const iio_buffer *buf = iio_device_get_buffer(dev(h), bufferIdx);
    if(!buf) {
        return 0;
    }
    return iio_buffer_get_attrs_count(buf);
}

QString V1DeviceOps::bufferAttrName(DeviceHandle h, unsigned int bufferIdx, unsigned int index) const
{
    const iio_buffer *buf = iio_device_get_buffer(dev(h), bufferIdx);
    if(!buf) {
        return {};
    }
    const iio_attr *attr = iio_buffer_get_attr(buf, index);
    if(!attr) {
        return {};
    }
    return QString::fromUtf8(iio_attr_get_name(attr));
}

DeviceHandle V1DeviceOps::getTrigger(DeviceHandle h) const
{
    const iio_device *trigger = iio_device_get_trigger(dev(h));
    if(iio_err(trigger)) {
        return {};
    }
    return {const_cast<iio_device *>(trigger)};
}

ssize_t V1DeviceOps::sampleSize(DeviceHandle h, ChannelsMaskHandle mask) const
{
    return iio_device_get_sample_size(dev(h), static_cast<iio_channels_mask *>(h.ptr));
}

Result<uint32_t> V1DeviceOps::regRead(DeviceHandle h, uint32_t addr)
{
    uint32_t val = 0;
    int ret = iio_device_reg_read(dev(h), addr, &val);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_device_reg_read failed @0x%1").arg(addr, 0, 16)}};
    }
    return val;
}

Result<void> V1DeviceOps::regWrite(DeviceHandle h, uint32_t addr, uint32_t val)
{
    int ret = iio_device_reg_write(dev(h), addr, val);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_device_reg_write failed @0x%1").arg(addr, 0, 16)}};
    }
    return {};
}

Result<void> V1DeviceOps::setTrigger(DeviceHandle h, DeviceHandle trigger)
{
    int ret = iio_device_set_trigger(dev(h), trigger.ptr ? dev(trigger) : nullptr);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_device_set_trigger failed")}};
    }
    return {};
}

} // namespace scopy::iio
