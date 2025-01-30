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

#ifndef IIOATTRIBUTE_H
#define IIOATTRIBUTE_H

#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOAttribute : public QObject
{
	Q_OBJECT
protected:
	IIOAttribute(QObject *parent = nullptr);
	~IIOAttribute();

public:
	IIOAttribute(const IIOAttribute &) = delete;
	IIOAttribute &operator=(const IIOAttribute &) = delete;

	static IIOAttribute *GetInstance();

	static ssize_t read_raw(const iio_attr *attr, char *dst, size_t len);
	static ssize_t write_raw(const iio_attr *attr, const void *src, size_t len);
	static const char *get_name(const iio_attr *attr);
	static const char *get_filename(const iio_attr *attr);
	static const char *get_static_value(const iio_attr *attr);

private:
	static IIOAttribute *pinstance_;
};
} // namespace scopy

#endif // IIOATTRIBUTE_H
