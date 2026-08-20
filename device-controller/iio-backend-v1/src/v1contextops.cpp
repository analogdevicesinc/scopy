#include "v1contextops.h"
#include <iio/iio.h>
#include <cerrno>

namespace scopy::iio {

static iio_context *ctx(ContextHandle h) { return static_cast<iio_context *>(h.ptr); }

ContextHandle V1ContextOps::createContext(const QString &uri, const ContextParams &params)
{
	iio_context_params iioParams = {};
	iioParams.timeout_ms = params.timeoutMs;
	QByteArray uriBytes = uri.toUtf8();
	iio_context *c = iio_create_context(&iioParams, uriBytes.constData());
    if(iio_err(c)) {
		return {};
	}
	return {c};
}

void V1ContextOps::destroyContext(ContextHandle h)
{
    if(h.ptr) {
        iio_context_destroy(ctx(h));
    }
}

QString V1ContextOps::name(ContextHandle h) const
{
    const char *n = iio_context_get_name(ctx(h));
    return n ? QString::fromUtf8(n) : QString();
}

QString V1ContextOps::description(ContextHandle h) const
{
    const char *d = iio_context_get_description(ctx(h));
    return d ? QString::fromUtf8(d) : QString();
}

void V1ContextOps::version(ContextHandle h, unsigned int &major, unsigned int &minor, QString &tag) const
{
    const char *s = iio_context_get_version_tag(ctx(h));
    major = iio_context_get_version_major(ctx(h));
    minor = iio_context_get_version_minor(ctx(h));
    tag = s ? QString::fromUtf8(s) : QString();
}

ContextVersion V1ContextOps::getVersion(ContextHandle h) const
{
    ContextVersion v;
    const char *s = iio_context_get_version_tag(ctx(h));
    v.major = iio_context_get_version_major(ctx(h));
    v.minor = iio_context_get_version_minor(ctx(h));
    v.gitTag = s ? QString::fromUtf8(s) : QString();
    return v;
}

unsigned int V1ContextOps::devicesCount(ContextHandle h) const { return iio_context_get_devices_count(ctx(h)); }

DeviceHandle V1ContextOps::getDevice(ContextHandle h, unsigned int index) const
{
    return {const_cast<iio_device *>(iio_context_get_device(ctx(h), index))};
}

DeviceHandle V1ContextOps::findDevice(ContextHandle h, const QString &name) const
{
    return {const_cast<iio_device *>(iio_context_find_device(ctx(h), name.toUtf8().constData()))};
}

unsigned int V1ContextOps::attrsCount(ContextHandle h) const { return iio_context_get_attrs_count(ctx(h)); }

QString V1ContextOps::attrName(ContextHandle h, unsigned int index) const
{
    const struct iio_attr *attr = iio_context_get_attr(ctx(h), index);
    const char *s = iio_attr_get_name(attr);
    return s ? QString::fromUtf8(s) : QString();
}

bool V1ContextOps::ping(ContextHandle h) const
{
    if(iio_context_get_devices_count(ctx(h)) == 0) {
        return true;
    }
    const iio_device *dev = iio_context_get_device(ctx(h), 0);
    if(!dev) {
        return false;
    }
    const iio_device *trig = iio_device_get_trigger(dev);
    int err = iio_err(trig);
    return err == 0 || err == -ENOENT;
}

} // namespace scopy::iio
