#include "v0attrops.h"
#include <iio.h>
#include <cerrno>
#include <QRegularExpression>

namespace scopy::iio {

static V0AttrInfo *info(AttrHandle h) { return static_cast<V0AttrInfo *>(h.ptr); }

AttrHandle V0AttrOps::makeHandle(V0AttrInfo::Scope scope, void *parent, const QString &name)
{
	auto *ai = new V0AttrInfo{scope, parent, name.toUtf8()};
	return {ai};
}

AttrHandle V0AttrOps::contextAttr(ContextHandle ctx, const QString &name)
{
	return makeHandle(V0AttrInfo::Context, ctx.ptr, name);
}

AttrHandle V0AttrOps::deviceAttr(DeviceHandle dev, const QString &name)
{
	return makeHandle(V0AttrInfo::Device, dev.ptr, name);
}

AttrHandle V0AttrOps::debugAttr(DeviceHandle dev, const QString &name)
{
	return makeHandle(V0AttrInfo::Debug, dev.ptr, name);
}

AttrHandle V0AttrOps::bufferAttr(DeviceHandle dev, unsigned int /*bufferIdx*/, const QString &name)
{
	return makeHandle(V0AttrInfo::Buffer, dev.ptr, name);
}

AttrHandle V0AttrOps::channelAttr(ChannelHandle ch, const QString &name)
{
	return makeHandle(V0AttrInfo::Channel, ch.ptr, name);
}

void V0AttrOps::releaseAttr(AttrHandle attr) { delete info(attr); }

Result<QByteArray> V0AttrOps::read(AttrHandle attr)
{
	auto *ai = info(attr);
	char buf[4096];
	ssize_t ret = 0;

	switch(ai->scope) {
	case V0AttrInfo::Context: {
		const char *val =
			iio_context_get_attr_value(static_cast<iio_context *>(ai->parent), ai->name.constData());
		if(val) {
			return QByteArray(val);
		}
        return Unexpected{Error{
            -ENOENT, QStringLiteral("context attr read failed: %1").arg(QString::fromUtf8(ai->name))}};
	}
	case V0AttrInfo::Device:
		ret = iio_device_attr_read(static_cast<const iio_device *>(ai->parent), ai->name.constData(), buf,
					   sizeof(buf));
		break;
	case V0AttrInfo::Channel:
		ret = iio_channel_attr_read(static_cast<const iio_channel *>(ai->parent), ai->name.constData(), buf,
					    sizeof(buf));
		break;
	case V0AttrInfo::Debug:
        ret = iio_device_debug_attr_read(static_cast<const iio_device *>(ai->parent), ai->name.constData(), buf,
                         sizeof(buf));
		break;
	case V0AttrInfo::Buffer:
		ret = iio_device_buffer_attr_read(static_cast<const iio_device *>(ai->parent), ai->name.constData(),
						  buf, sizeof(buf));
		break;
	}

	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret),
					QStringLiteral("attr read failed: %1").arg(QString::fromUtf8(ai->name))}};
	}
	return QByteArray(buf, ret);
}

Result<void> V0AttrOps::write(AttrHandle attr, const QString &value)
{
	auto *ai = info(attr);
	QByteArray val = value.toUtf8();
	ssize_t ret = 0;

	switch(ai->scope) {
	case V0AttrInfo::Context:
		return Unexpected{Error{-ENOSYS, QStringLiteral("context attrs are read-only")}};
	case V0AttrInfo::Device:
		ret = iio_device_attr_write(static_cast<const iio_device *>(ai->parent), ai->name.constData(),
					    val.constData());
		break;
	case V0AttrInfo::Channel:
		ret = iio_channel_attr_write(static_cast<const iio_channel *>(ai->parent), ai->name.constData(),
					     val.constData());
		break;
	case V0AttrInfo::Debug:
		ret = iio_device_debug_attr_write(static_cast<const iio_device *>(ai->parent), ai->name.constData(),
						  val.constData());
		break;
	case V0AttrInfo::Buffer:
		ret = iio_device_buffer_attr_write(static_cast<const iio_device *>(ai->parent), ai->name.constData(),
						   val.constData());
		break;
	}

	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret),
					QStringLiteral("attr write failed: %1").arg(QString::fromUtf8(ai->name))}};
	}
	return {};
}

Result<QByteArray> V0AttrOps::readAvailableAttr(const V0AttrInfo *ai) const
{
	QByteArray availName = ai->name + "_available";
	char buf[4096];
	ssize_t ret = 0;

	switch(ai->scope) {
	case V0AttrInfo::Context:
		return Unexpected{Error{-ENOSYS, QStringLiteral("no _available for context attrs")}};
	case V0AttrInfo::Device:
		ret = iio_device_attr_read(static_cast<const iio_device *>(ai->parent), availName.constData(), buf,
					   sizeof(buf));
		break;
	case V0AttrInfo::Channel:
		ret = iio_channel_attr_read(static_cast<const iio_channel *>(ai->parent), availName.constData(), buf,
					    sizeof(buf));
		break;
	case V0AttrInfo::Debug:
		ret = iio_device_debug_attr_read(static_cast<const iio_device *>(ai->parent), availName.constData(),
						 buf, sizeof(buf));
		break;
	case V0AttrInfo::Buffer:
		ret = iio_device_buffer_attr_read(static_cast<const iio_device *>(ai->parent), availName.constData(),
						  buf, sizeof(buf));
		break;
	}

	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret),
					QStringLiteral("_available read failed: %1").arg(QString::fromUtf8(ai->name))}};
	}
	return QByteArray(buf, ret);
}

Result<void> V0AttrOps::getRange(AttrHandle attr, double &min, double &step, double &max) const
{
	auto *ai = info(attr);
	Result<QByteArray> r = readAvailableAttr(ai);
	if(!r) {
		return Unexpected{r.error()};
	}

	// Parse "[min step max]" format
	QString str = QString::fromUtf8(r.value()).trimmed();
	if(str.startsWith('[') && str.endsWith(']')) {
		str = str.mid(1, str.length() - 2).trimmed();
	}

	QStringList parts = str.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	if(parts.size() != 3) {
        return Unexpected{
            Error{-EINVAL, QStringLiteral("range: expected \"[min step max]\", got \"%1\"").arg(str)}};
	}

	bool okMin, okStep, okMax;
	min = parts[0].toDouble(&okMin);
	step = parts[1].toDouble(&okStep);
	max = parts[2].toDouble(&okMax);

	if(!okMin || !okStep || !okMax) {
		return Unexpected{Error{-EINVAL, QStringLiteral("range: non-numeric field in \"%1\"").arg(str)}};
	}
	return {};
}

Result<void> V0AttrOps::getAvailable(AttrHandle attr, QStringList &values) const
{
	auto *ai = info(attr);
	Result<QByteArray> r = readAvailableAttr(ai);
	if(!r) {
		return Unexpected{r.error()};
	}

	values = QString::fromUtf8(r.value()).trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	return {};
}

} // namespace scopy::iio
