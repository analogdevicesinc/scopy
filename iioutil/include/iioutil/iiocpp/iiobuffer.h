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

#ifndef IIOBUFFER_H
#define IIOBUFFER_H

#include "iiocpp/iioresult.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOBuffer : public QObject
{
	Q_OBJECT

protected:
	IIOBuffer(QObject *parent = nullptr);
	~IIOBuffer();

public:
	IIOBuffer(const IIOBuffer &) = delete;
	IIOBuffer &operator=(const IIOBuffer &) = delete;

	static IIOBuffer *GetInstance();

	static const iio_device *get_device(const iio_buffer *buf);
	static unsigned int get_attrs_count(const iio_buffer *buf);
	static IIOResult<const iio_attr *> get_attr(const iio_buffer *buf, unsigned int index);
	static IIOResult<const iio_attr *> find_attr(const iio_buffer *buf, const char *name);
	static IIOResult<iio_buffer *> create_buffer(const iio_device *dev, unsigned int idx,
						     const iio_channels_mask *mask);
	static void set_data(iio_buffer *buf, void *data);
	static void *get_data(const iio_buffer *buf);
	static void destroy(iio_buffer *buf);
	static void cancel(iio_buffer *buf);
	static int enable(iio_buffer *buf);
	static int disable(iio_buffer *buf);
	static const iio_channels_mask *get_channels_mask(const iio_buffer *buf);

private:
	static IIOBuffer *pinstance_;
};
} // namespace scopy

#endif // IIOBUFFER_H
