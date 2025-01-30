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

const struct iio_device *IIOChannel::get_device(const struct iio_channel *chn) { return iio_channel_get_device(chn); }

const char *IIOChannel::get_id(const struct iio_channel *chn) { return iio_channel_get_id(chn); }

const char *IIOChannel::get_name(const struct iio_channel *chn) { return iio_channel_get_name(chn); }

const char *IIOChannel::get_label(const struct iio_channel *chn) { return iio_channel_get_label(chn); }

bool IIOChannel::is_output(const struct iio_channel *chn) { return iio_channel_is_output(chn); }

bool IIOChannel::is_scan_element(const struct iio_channel *chn) { return iio_channel_is_scan_element(chn); }

unsigned int IIOChannel::get_attrs_count(const struct iio_channel *chn) { return iio_channel_get_attrs_count(chn); }

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

void IIOChannel::enable(const struct iio_channel *chn, struct iio_channels_mask *mask)
{
	iio_channel_enable(chn, mask);
}

void IIOChannel::disable(const struct iio_channel *chn, struct iio_channels_mask *mask)
{
	iio_channel_disable(chn, mask);
}

bool IIOChannel::is_enabled(const struct iio_channel *chn, const struct iio_channels_mask *mask)
{
	return iio_channel_is_enabled(chn, mask);
}

size_t IIOChannel::read(const struct iio_channel *chn, const struct iio_block *block, void *dst, size_t len, bool raw)
{
	return iio_channel_read(chn, block, dst, len, raw);
}

size_t IIOChannel::write(const struct iio_channel *chn, struct iio_block *block, const void *src, size_t len, bool raw)
{
	return iio_channel_write(chn, block, src, len, raw);
}

void IIOChannel::set_data(struct iio_channel *chn, void *data) { iio_channel_set_data(chn, data); }

void *IIOChannel::get_data(const struct iio_channel *chn) { return iio_channel_get_data(chn); }

enum iio_chan_type IIOChannel::get_type(const struct iio_channel *chn) { return iio_channel_get_type(chn); }

enum iio_modifier IIOChannel::get_modifier(const struct iio_channel *chn) { return iio_channel_get_modifier(chn); }
