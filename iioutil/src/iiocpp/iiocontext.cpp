/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 */

#include "iiocpp/iiocontext.h"
#include "iiocpp/iioresult.h"
#include <QApplication>

using namespace scopy;

IIOContext *IIOContext::pinstance_{nullptr};

IIOContext::IIOContext(QObject *parent)
	: QObject(parent)
{
}

IIOContext::~IIOContext() {}

IIOContext *IIOContext::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOContext(QApplication::instance());
	}
	return pinstance_;
}

IIOResult<iio_context *> IIOContext::create_context(const iio_context_params *params, const char *uri)
{
	iio_context *ctx = iio_create_context(params, uri);
	int err = iio_err(ctx);

	if(err) {
		return IIOResult<iio_context *>(err);
	}
	return IIOResult<iio_context *>(ctx);
}

void IIOContext::destroy(iio_context *ctx) { iio_context_destroy(ctx); }

unsigned int IIOContext::get_version_major(const iio_context *ctx) { return iio_context_get_version_major(ctx); }

unsigned int IIOContext::get_version_minor(const iio_context *ctx) { return iio_context_get_version_minor(ctx); }

const char *IIOContext::get_version_tag(const iio_context *ctx) { return iio_context_get_version_tag(ctx); }

IIOResult<char *> IIOContext::get_xml(const iio_context *ctx)
{
	char *xml = iio_context_get_xml(ctx);
	int err = iio_err(xml);
	if(err) {
		return IIOResult<char *>(err);
	}
	return IIOResult<char *>(xml);
}

const char *IIOContext::get_name(const iio_context *ctx) { return iio_context_get_name(ctx); }

const char *IIOContext::get_description(const iio_context *ctx) { return iio_context_get_description(ctx); }

unsigned int IIOContext::get_attrs_count(const iio_context *ctx) { return iio_context_get_attrs_count(ctx); }

IIOResult<const iio_attr *> IIOContext::get_attr(const iio_context *ctx, unsigned int index)
{
	const iio_attr *attr = iio_context_get_attr(ctx, index);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<const iio_attr *> IIOContext::find_attr(const iio_context *ctx, const char *name)
{
	const iio_attr *attr = iio_context_find_attr(ctx, name);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

unsigned int IIOContext::get_devices_count(const iio_context *ctx) { return iio_context_get_devices_count(ctx); }

IIOResult<iio_device *> IIOContext::get_device(const iio_context *ctx, unsigned int index)
{
	iio_device *dev = iio_context_get_device(ctx, index);
	if(!dev) {
		return IIOResult<iio_device *>(-EINVAL);
	}
	return IIOResult<iio_device *>(dev);
}

IIOResult<iio_device *> IIOContext::find_device(const iio_context *ctx, const char *name)
{
	iio_device *dev = iio_context_find_device(ctx, name);
	if(!dev) {
		return IIOResult<iio_device *>(-EINVAL);
	}
	return IIOResult<iio_device *>(dev);
}

int IIOContext::set_timeout(iio_context *ctx, unsigned int timeout_ms)
{
	return iio_context_set_timeout(ctx, timeout_ms);
}

const iio_context_params *IIOContext::get_params(const iio_context *ctx) { return iio_context_get_params(ctx); }

void IIOContext::set_data(iio_context *ctx, void *data) { iio_context_set_data(ctx, data); }

void *IIOContext::get_data(const iio_context *ctx) { return iio_context_get_data(ctx); }
