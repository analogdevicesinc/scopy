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

#include "iiocpp/iiodevice.h"
#include <QApplication>

using namespace scopy;

IIODevice *IIODevice::pinstance_{nullptr};

IIODevice::IIODevice(QObject *parent)
	: QObject(parent)
{
}

IIODevice::~IIODevice() = default;

IIODevice *IIODevice::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIODevice(QApplication::instance());
	}
	return pinstance_;
}

const iio_context *IIODevice::get_context(const iio_device *dev) { return iio_device_get_context(dev); }

const char *IIODevice::get_id(const iio_device *dev) { return iio_device_get_id(dev); }

const char *IIODevice::get_name(const iio_device *dev) { return iio_device_get_name(dev); }

const char *IIODevice::get_label(const iio_device *dev) { return iio_device_get_label(dev); }

unsigned int IIODevice::get_channels_count(const iio_device *dev) { return iio_device_get_channels_count(dev); }

unsigned int IIODevice::get_attrs_count(const iio_device *dev) { return iio_device_get_attrs_count(dev); }

IIOResult<const iio_channel *> IIODevice::get_channel(const iio_device *dev, unsigned int index)
{
	iio_channel *channel = iio_device_get_channel(dev, index);
	if(!channel) {
		return IIOResult<const iio_channel *>(-EINVAL);
	}
	return IIOResult<const iio_channel *>(channel);
}

IIOResult<const iio_attr *> IIODevice::get_attr(const iio_device *dev, unsigned int index)
{
	const iio_attr *attr = iio_device_get_attr(dev, index);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<iio_channel *> IIODevice::find_channel(const iio_device *dev, const char *name, bool output)
{
	iio_channel *channel = iio_device_find_channel(dev, name, output);
	if(!channel) {
		return IIOResult<iio_channel *>(-EINVAL);
	}
	return IIOResult<iio_channel *>(channel);
}

IIOResult<const iio_attr *> IIODevice::find_attr(const iio_device *dev, const char *name)
{
	const iio_attr *attr = iio_device_find_attr(dev, name);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

void IIODevice::set_data(iio_device *dev, void *data) { iio_device_set_data(dev, data); }

void *IIODevice::get_data(const iio_device *dev) { return iio_device_get_data(dev); }

IIOResult<const iio_device *> IIODevice::get_trigger(const iio_device *dev)
{
	const iio_device *trigger = iio_device_get_trigger(dev);
	int err = iio_err(trigger);

	if(err) {
		return IIOResult<const iio_device *>(err);
	}
	return IIOResult<const iio_device *>(trigger);
}

int IIODevice::set_trigger(iio_device *dev, const iio_device *trigger) { return iio_device_set_trigger(dev, trigger); }

bool IIODevice::is_trigger(const iio_device *dev) { return iio_device_is_trigger(dev); }

ssize_t IIODevice::get_sample_size(const iio_device *dev, const iio_channels_mask *mask)
{
	return iio_device_get_sample_size(dev, mask);
}

unsigned int IIODevice::get_debug_attrs_count(const iio_device *dev) { return iio_device_get_debug_attrs_count(dev); }

IIOResult<const iio_attr *> IIODevice::get_debug_attr(const iio_device *dev, unsigned int index)
{
	const iio_attr *attr = iio_device_get_debug_attr(dev, index);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

IIOResult<const iio_attr *> IIODevice::find_debug_attr(const iio_device *dev, const char *name)
{
	const iio_attr *attr = iio_device_find_debug_attr(dev, name);
	if(!attr) {
		return IIOResult<const iio_attr *>(-EINVAL);
	}
	return IIOResult<const iio_attr *>(attr);
}

int IIODevice::reg_write(iio_device *dev, uint32_t address, uint32_t value)
{
	return iio_device_reg_write(dev, address, value);
}

int IIODevice::reg_read(iio_device *dev, uint32_t address, uint32_t *value)
{
	return iio_device_reg_read(dev, address, value);
}
