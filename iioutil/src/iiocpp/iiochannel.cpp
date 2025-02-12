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

#include "iiocpp/iiochannel.h"
#include <QApplication>
#include <iio/iio.h>

using namespace scopy;

IIOChannel *IIOChannel::pinstance_{nullptr};

IIOChannel::IIOChannel(QObject *parent)
	: QObject(parent)
{
}

IIOChannel::~IIOChannel() = default;

IIOChannel *IIOChannel::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOChannel(QApplication::instance());
	}
	return pinstance_;
}

const iio_device *IIOChannel::get_device(const iio_channel *chn) { return iio_channel_get_device(chn); }

const char *IIOChannel::get_id(const iio_channel *chn) { return iio_channel_get_id(chn); }

const char *IIOChannel::get_name(const iio_channel *chn) { return iio_channel_get_name(chn); }

const char *IIOChannel::get_label(const iio_channel *chn) { return iio_channel_get_label(chn); }

bool IIOChannel::is_output(const iio_channel *chn) { return iio_channel_is_output(chn); }

bool IIOChannel::is_scan_element(const iio_channel *chn) { return iio_channel_is_scan_element(chn); }

unsigned int IIOChannel::get_attrs_count(const iio_channel *chn) { return iio_channel_get_attrs_count(chn); }

IIOResult<const iio_attr *> IIOChannel::get_attr(const iio_channel *chn, unsigned int index)
{
	const iio_attr *attr = iio_channel_get_attr(chn, index);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<const iio_attr *> IIOChannel::find_attr(const iio_channel *chn, const char *name)
{
	const iio_attr *attr = iio_channel_find_attr(chn, name);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

void IIOChannel::enable(const iio_channel *chn, iio_channels_mask *mask) { iio_channel_enable(chn, mask); }

void IIOChannel::disable(const iio_channel *chn, iio_channels_mask *mask) { iio_channel_disable(chn, mask); }

bool IIOChannel::is_enabled(const iio_channel *chn, const iio_channels_mask *mask)
{
	return iio_channel_is_enabled(chn, mask);
}

size_t IIOChannel::read(const iio_channel *chn, const iio_block *block, void *dst, size_t len, bool raw)
{
	return iio_channel_read(chn, block, dst, len, raw);
}

size_t IIOChannel::write(const iio_channel *chn, iio_block *block, const void *src, size_t len, bool raw)
{
	return iio_channel_write(chn, block, src, len, raw);
}

void IIOChannel::set_data(iio_channel *chn, void *data) { iio_channel_set_data(chn, data); }

void *IIOChannel::get_data(const iio_channel *chn) { return iio_channel_get_data(chn); }

enum iio_chan_type IIOChannel::get_type(const iio_channel *chn) { return iio_channel_get_type(chn); }

enum iio_modifier IIOChannel::get_modifier(const iio_channel *chn) { return iio_channel_get_modifier(chn); }

inline hwmon_chan_type IIOChannel::hwmon_get_type(const iio_channel *chn) { return hwmon_channel_get_type(chn); }

iio_channels_mask *IIOChannel::create_channels_mask(unsigned int nb_channels)
{
	return iio_create_channels_mask(nb_channels);
}

void IIOChannel::destroy_channels_mask(iio_channels_mask *mask) { iio_channels_mask_destroy(mask); }

long IIOChannel::get_index(const iio_channel *chn) { return iio_channel_get_index(chn); }

const iio_data_format *IIOChannel::get_data_format(const iio_channel *chn) { return iio_channel_get_data_format(chn); }

void IIOChannel::convert(const iio_channel *chn, void *dst, const void *src) { iio_channel_convert(chn, dst, src); }

void IIOChannel::convert_inverse(const iio_channel *chn, void *dst, const void *src)
{
	iio_channel_convert_inverse(chn, dst, src);
}
