/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "v1attrops.h"
#include "core/calltrace.h"
#include <iio/iio.h>
#include <cerrno>

namespace scopy::iio {

static const struct iio_attr *attr(AttrHandle h) { return static_cast<const iio_attr *>(h.ptr); }

static QString attrName(AttrHandle h)
{
	const char *s = h.ptr ? iio_attr_get_name(attr(h)) : nullptr;
	return s ? QString::fromUtf8(s) : QString();
}

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

Result<QByteArray> V1AttrOps::read(AttrHandle h, size_t bytes)
{
	QByteArray buf(static_cast<int>(bytes), Qt::Uninitialized);
	ssize_t ret =
		DC_TRACE_OUT("libiio", "iio_attr_read_raw", h.ptr, iio_attr_read_raw(attr(h), buf.data(), buf.size()),
			     dcRc > 0 ? CallTrace::text(buf.constData(), dcRc) : CallOut(),
			     CallArg::string("attr", attrName(h)), CallArg::outLen("len", buf.size()));
	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_attr_read_raw failed")}};
	}
	// libiio reports the byte count including the trailing NUL terminator; strip it so the
	// value is not polluted with an embedded '\0' (breaks QString::toDouble(), comparisons, etc.)
	QByteArray result(buf.constData(), static_cast<int>(ret));
	while(result.endsWith('\0')) {
		result.chop(1);
	}
	return result;
}

Result<void> V1AttrOps::write(AttrHandle h, const QString &value)
{
	QByteArray b = value.toUtf8();
	ssize_t ret = DC_TRACE("libiio", "iio_attr_write_raw", h.ptr,
			       iio_attr_write_raw(attr(h), b.constData(), static_cast<size_t>(b.size())),
			       CallArg::string("attr", attrName(h)), CallArg::string("src", value),
			       CallArg::integer("len", b.size()));
	if(ret < 0) {
		return Unexpected{Error{static_cast<int>(ret), QStringLiteral("iio_attr_write_raw failed")}};
	}
	return {};
}

Result<void> V1AttrOps::getRange(AttrHandle h, double &min, double &step, double &max) const
{
	int ret = DC_TRACE_OUT("libiio", "iio_attr_get_range", h.ptr, iio_attr_get_range(attr(h), &min, &step, &max),
			       QStringLiteral("[%1 %2 %3]").arg(min).arg(step).arg(max),
			       CallArg::string("attr", attrName(h)));
	if(ret < 0) {
		return Unexpected{Error{ret, QStringLiteral("iio_attr_get_range failed")}};
	}
	return {};
}

Result<void> V1AttrOps::getAvailable(AttrHandle h, QStringList &values) const
{
	char **list = nullptr;
	size_t count = 0;
	int ret =
		DC_TRACE_OUT("libiio", "iio_attr_get_available", h.ptr, iio_attr_get_available(attr(h), &list, &count),
			     QVariant::fromValue(qint64(count)), CallArg::string("attr", attrName(h)));
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
