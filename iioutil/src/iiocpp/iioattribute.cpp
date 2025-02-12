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

#include "iiocpp/iioattribute.h"
#include <QApplication>
#include <iio/iio.h>

using namespace scopy;

IIOAttribute *IIOAttribute::pinstance_{nullptr};

IIOAttribute::IIOAttribute(QObject *parent)
	: QObject(parent)
{
}

IIOAttribute::~IIOAttribute() = default;

IIOAttribute *IIOAttribute::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOAttribute(QApplication::instance());
	}
	return pinstance_;
}

ssize_t IIOAttribute::read_any(const iio_attr *attr, char *dst, size_t len)
{
	const char *value = iio_attr_get_static_value(attr);
	ssize_t ret = 0;
	if(value) {
		dst = const_cast<char *>(value);
		return ret;
	}

	ret = iio_attr_read_raw(attr, dst, len);
	return ret;
}

ssize_t IIOAttribute::read_raw(const iio_attr *attr, char *dst, size_t len)
{
	return iio_attr_read_raw(attr, dst, len);
}

ssize_t read(const iio_attr *attr, bool *ptr) { return iio_attr_read_bool(attr, ptr); }

ssize_t read(const iio_attr *attr, long long *ptr) { return iio_attr_read_longlong(attr, ptr); }

ssize_t read(const iio_attr *attr, double *ptr) { return iio_attr_read_double(attr, ptr); }

ssize_t IIOAttribute::write_raw(const iio_attr *attr, const void *src, size_t len)
{
	return iio_attr_write_raw(attr, src, len);
}

ssize_t IIOAttribute::write(const iio_attr *attr, const char *val) { return iio_attr_write_string(attr, val); }

ssize_t IIOAttribute::write(const iio_attr *attr, char *val) { return iio_attr_write_string(attr, val); }

ssize_t IIOAttribute::write(const iio_attr *attr, bool val) { return iio_attr_write_bool(attr, val); }

ssize_t IIOAttribute::write(const iio_attr *attr, long long val) { return iio_attr_write_longlong(attr, val); }

ssize_t IIOAttribute::write(const iio_attr *attr, double val) { return iio_attr_write_double(attr, val); }

const char *IIOAttribute::get_name(const iio_attr *attr) { return iio_attr_get_name(attr); }

const char *IIOAttribute::get_filename(const iio_attr *attr) { return iio_attr_get_filename(attr); }

const char *IIOAttribute::get_static_value(const iio_attr *attr) { return iio_attr_get_static_value(attr); }
