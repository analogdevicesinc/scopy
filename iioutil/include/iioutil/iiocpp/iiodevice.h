/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef IIODEVICE_H
#define IIODEVICE_H

#include "iiocpp/iioresult.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIODevice : public QObject
{
	Q_OBJECT
protected:
	IIODevice(QObject *parent = nullptr);
	~IIODevice();

public:
	IIODevice(const IIODevice &) = delete;
	IIODevice &operator=(const IIODevice &) = delete;

	static IIODevice *GetInstance();

	static const iio_context *get_context(const iio_device *dev);
	static const char *get_id(const iio_device *dev);
	static const char *get_name(const iio_device *dev);
	static const char *get_label(const iio_device *dev);
	static unsigned int get_channels_count(const iio_device *dev);
	static unsigned int get_attrs_count(const iio_device *dev);
	static IIOResult<const iio_channel *> get_channel(const iio_device *dev, unsigned int index);
	static IIOResult<const iio_attr *> get_attr(const iio_device *dev, unsigned int index);
	static IIOResult<iio_channel *> find_channel(const iio_device *dev, const char *name, bool output);
	static IIOResult<const iio_attr *> find_attr(const iio_device *dev, const char *name);
	static void set_data(iio_device *dev, void *data);
	static void *get_data(const iio_device *dev);
	static IIOResult<const iio_device *> get_trigger(const iio_device *dev);
	static int set_trigger(iio_device *dev, const iio_device *trigger);
	static bool is_trigger(const iio_device *dev);

private:
	static IIODevice *pinstance_;
};
} // namespace scopy

#endif // IIODEVICE_H
