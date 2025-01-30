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

#ifndef IIOCHANNEL_H
#define IIOCHANNEL_H

#include "iiocpp/iioresult.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOChannel : public QObject
{
	Q_OBJECT
protected:
	IIOChannel(QObject *parent = nullptr);
	~IIOChannel();

public:
	IIOChannel(const IIOChannel &) = delete;
	IIOChannel &operator=(const IIOChannel &) = delete;

	static IIOChannel *GetInstance();

	static const iio_device *get_device(const iio_channel *chn);
	static const char *get_id(const iio_channel *chn);
	static const char *get_name(const iio_channel *chn);
	static const char *get_label(const iio_channel *chn);
	static bool is_output(const iio_channel *chn);
	static bool is_scan_element(const iio_channel *chn);
	static unsigned int get_attrs_count(const iio_channel *chn);
	static IIOResult<const iio_attr *> get_attr(const iio_channel *chn, unsigned int index);
	static IIOResult<const iio_attr *> find_attr(const iio_channel *chn, const char *name);
	static void enable(const iio_channel *chn, iio_channels_mask *mask);
	static void disable(const iio_channel *chn, iio_channels_mask *mask);
	static bool is_enabled(const iio_channel *chn, const iio_channels_mask *mask);
	static size_t read(const iio_channel *chn, const iio_block *block, void *dst, size_t len, bool raw);
	static size_t write(const iio_channel *chn, iio_block *block, const void *src, size_t len, bool raw);
	static void set_data(iio_channel *chn, void *data);
	static void *get_data(const iio_channel *chn);
	static enum iio_chan_type get_type(const iio_channel *chn);
	static enum iio_modifier get_modifier(const iio_channel *chn);

private:
	static IIOChannel *pinstance_;
};
} // namespace scopy

#endif // IIOCHANNEL_H
