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

#include "v0contextops.h"
#include <iio.h>

namespace scopy::iio {

static iio_context *ctx(ContextHandle h) { return static_cast<iio_context *>(h.ptr); }

ContextHandle V0ContextOps::createContext(const QString &uri, const ContextParams &params)
{
	iio_context *c = iio_create_context_from_uri(uri.toUtf8().constData());
	if(!c) {
		return {};
	}
	if(params.timeoutMs != 0) {
		iio_context_set_timeout(c, static_cast<unsigned int>(params.timeoutMs));
	}
	return {c};
}

void V0ContextOps::destroyContext(ContextHandle h)
{
	if(h.ptr) {
		iio_context_destroy(ctx(h));
	}
}

QString V0ContextOps::name(ContextHandle h) const
{
	const char *n = iio_context_get_name(ctx(h));
	return n ? QString::fromUtf8(n) : QString();
}

QString V0ContextOps::description(ContextHandle h) const
{
	const char *d = iio_context_get_description(ctx(h));
	return d ? QString::fromUtf8(d) : QString();
}

void V0ContextOps::version(ContextHandle h, unsigned int &major, unsigned int &minor, QString &tag) const
{
	char tagBuf[64] = {};
	iio_context_get_version(ctx(h), &major, &minor, tagBuf);
	tag = QString::fromUtf8(tagBuf);
}

ContextVersion V0ContextOps::getVersion(ContextHandle h) const
{
	ContextVersion v;
	char tagBuf[64] = {};
	iio_context_get_version(ctx(h), &v.major, &v.minor, tagBuf);
	v.gitTag = QString::fromUtf8(tagBuf);
	return v;
}

unsigned int V0ContextOps::devicesCount(ContextHandle h) const { return iio_context_get_devices_count(ctx(h)); }

DeviceHandle V0ContextOps::getDevice(ContextHandle h, unsigned int index) const
{
	return {const_cast<iio_device *>(iio_context_get_device(ctx(h), index))};
}

DeviceHandle V0ContextOps::findDevice(ContextHandle h, const QString &name) const
{
	return {const_cast<iio_device *>(iio_context_find_device(ctx(h), name.toUtf8().constData()))};
}

unsigned int V0ContextOps::attrsCount(ContextHandle h) const { return iio_context_get_attrs_count(ctx(h)); }

QString V0ContextOps::attrName(ContextHandle h, unsigned int index) const
{
	const char *name = nullptr;
	iio_context_get_attr(ctx(h), index, &name, nullptr);
	return name ? QString::fromUtf8(name) : QString();
}

bool V0ContextOps::ping(ContextHandle h) const
{
	if(iio_context_get_devices_count(ctx(h)) == 0) {
		return true;
	}
	const iio_device *dev = iio_context_get_device(ctx(h), 0);
	if(!dev) {
		return false;
	}
	const iio_device *trig = nullptr;
	int ret = iio_device_get_trigger(dev, &trig);
	return ret >= 0 || ret == -ENOENT;
}

} // namespace scopy::iio
