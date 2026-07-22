#include "v1attrops.h"
#include <iio/iio.h>
#include <cerrno>

namespace scopy::iio {

static const struct iio_attr *attr(AttrHandle h) { return static_cast<const iio_attr *>(h.ptr); }

AttrHandle V1AttrOps::contextAttr(ContextHandle ctx, const QString &name)
{
    return {const_cast<iio_attr *>(
        iio_context_find_attr(static_cast<const iio_context *>(ctx.ptr), qUtf8Printable(name)))};
}

AttrHandle V1AttrOps::deviceAttr(DeviceHandle dev, const QString &name)
{
    return {const_cast<iio_attr *>(
        iio_device_find_attr(static_cast<const iio_device *>(dev.ptr), qUtf8Printable(name)))};
}

AttrHandle V1AttrOps::debugAttr(DeviceHandle dev, const QString &name)
{
    return {const_cast<iio_attr *>(
        iio_device_find_debug_attr(static_cast<const iio_device *>(dev.ptr), qUtf8Printable(name)))};
}

AttrHandle V1AttrOps::bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name)
{
    const iio_buffer *buf = iio_device_get_buffer(static_cast<const iio_device *>(dev.ptr), bufferIdx);
    if(!buf) {
        return {};
    }
    return {const_cast<iio_attr *>(iio_buffer_find_attr(buf, qUtf8Printable(name)))};
}

AttrHandle V1AttrOps::channelAttr(ChannelHandle ch, const QString &name)
{
    return {const_cast<iio_attr *>(
        iio_channel_find_attr(static_cast<const iio_channel *>(ch.ptr), qUtf8Printable(name)))};
}

void V1AttrOps::releaseAttr(AttrHandle attr) {}

Result<QByteArray> V1AttrOps::read(AttrHandle h)
{
    char buf[4096];
    ssize_t ret = iio_attr_read_raw(attr(h), buf, sizeof(buf));
    if(ret < 0) {
        return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_attr_read_raw failed")}};
    }
    return QByteArray(buf, static_cast<int>(ret));
}

Result<void> V1AttrOps::write(AttrHandle h, const QString &value)
{
    QByteArray b = value.toUtf8();
    ssize_t ret = iio_attr_write_raw(attr(h), b.constData(), static_cast<size_t>(b.size()));
    if(ret < 0) {
        return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_attr_write_raw failed")}};
    }
    return {};
}

Result<void> V1AttrOps::getRange(AttrHandle h, double &min, double &step, double &max) const
{
    int ret = iio_attr_get_range(attr(h), &min, &step, &max);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_attr_get_range failed")}};
    }
    return {};
}

Result<void> V1AttrOps::getAvailable(AttrHandle h, QStringList &values) const
{
    char **list = nullptr;
    size_t count = 0;
    int ret = iio_attr_get_available(attr(h), &list, &count);
    if(ret < 0) {
        return Unexpected{Error{ret, QStringLiteral("iio_attr_get_available failed")}};
    }
    for(size_t i = 0; i < count; ++i) {
        values << QString::fromUtf8(list[i]);
    }
    iio_available_list_free(list, count);
    return {};
}

} // namespace scopy::iio
