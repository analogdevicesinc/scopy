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

#include "iiocpp/iiobuffer.h"
#include <QApplication>

using namespace scopy;

IIOBuffer *IIOBuffer::pinstance_{nullptr};

IIOBuffer::IIOBuffer(QObject *parent)
	: QObject(parent)
{
}

IIOBuffer::~IIOBuffer() = default;

IIOBuffer *IIOBuffer::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOBuffer(QApplication::instance());
	}
	return pinstance_;
}

const iio_device *IIOBuffer::get_device(const iio_buffer *buf) { return iio_buffer_get_device(buf); }

unsigned int IIOBuffer::get_attrs_count(const iio_buffer *buf) { return iio_buffer_get_attrs_count(buf); }

IIOResult<const iio_attr *> IIOBuffer::get_attr(const iio_buffer *buf, unsigned int index)
{
	const iio_attr *attr = iio_buffer_get_attr(buf, index);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<const iio_attr *> IIOBuffer::find_attr(const iio_buffer *buf, const char *name)
{
	const iio_attr *attr = iio_buffer_find_attr(buf, name);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<iio_buffer *> IIOBuffer::create_buffer(const iio_device *dev, unsigned int idx, const iio_channels_mask *mask)
{
	iio_buffer *buf = iio_device_create_buffer(dev, idx, mask);
	int err = iio_err(buf);

	if(err) {
		return IIOResult<iio_buffer *>(err);
	}
	return IIOResult<iio_buffer *>(buf);
}

void IIOBuffer::set_data(iio_buffer *buf, void *data) { iio_buffer_set_data(buf, data); }

void *IIOBuffer::get_data(const iio_buffer *buf) { return iio_buffer_get_data(buf); }

void IIOBuffer::destroy(iio_buffer *buf) { iio_buffer_destroy(buf); }

void IIOBuffer::cancel(iio_buffer *buf) { iio_buffer_cancel(buf); }

int IIOBuffer::enable(iio_buffer *buf) { return iio_buffer_enable(buf); }

int IIOBuffer::disable(iio_buffer *buf) { return iio_buffer_disable(buf); }

const iio_channels_mask *IIOBuffer::get_channels_mask(const iio_buffer *buf)
{
	return iio_buffer_get_channels_mask(buf);
}
