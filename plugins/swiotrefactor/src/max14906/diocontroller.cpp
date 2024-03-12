/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "max14906/diocontroller.h"
#include "max14906/max14906.h"
#include "swiot_logging_categories.h"

using namespace scopy::swiotrefactor;

DioController::DioController(struct iio_context *context_, QString deviceName)
	: m_deviceName(std::move(deviceName))
	, m_context(context_)
{
	struct iio_device *dev = iio_context_find_device(this->m_context, MAX_NAME);
	this->m_device = dev;
}

DioController::~DioController() {}

int DioController::getChannelCount()
{
	unsigned int dev_count = iio_device_get_channels_count(this->m_device);
	return (int)(dev_count);
}

QString DioController::getChannelName(unsigned int index)
{
	iio_channel *channel = iio_device_get_channel(this->m_device, index);
	if(channel == nullptr) {
		qCritical(CAT_SWIOT_MAX14906)
			<< "Error when selecting channel with index" << index << ", returning empty string.";
		return "";
	}
	QString name = iio_channel_get_id(channel);

	return name;
}

QString DioController::getChannelType(unsigned int index)
{
	iio_channel *channel = iio_device_get_channel(this->m_device, index);
	if(channel == nullptr) {
		qCritical(CAT_SWIOT_MAX14906)
			<< "Error when selecting channel with index" << index << ", returning empty string.";
		return "";
	}
	bool output = iio_channel_is_output(channel);

	return output ? "OUTPUT" : "INPUT";
}

iio_device *DioController::getDevice() const { return DioController::m_device; }
