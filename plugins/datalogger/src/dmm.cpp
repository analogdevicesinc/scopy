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

#include "dmm.hpp"

#include <datamonitor/readstrategy/dmmreadstrategy.hpp>
#include <stylehelper.h>

using namespace scopy;
using namespace datamonitor;

DMM::DMM(QObject *parent)
	: QObject{parent}
{
	generateDictionaries();
}

QList<DmmDataMonitorModel *> DMM::getDmmMonitors(iio_context *ctx)
{
	QList<DmmDataMonitorModel *> result;

	auto deviceCount = iio_context_get_devices_count(ctx);
	for(int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		auto chnCout = iio_device_get_channels_count(dev);
		for(int j = 0; j < chnCout; j++) {
			// check if dmm
			iio_channel *chn = iio_device_get_channel(dev, j);
			// if the channel is DMM or hwmon we create a monitor for it
			if(isDMMCompatible(chn) || isHwmon(dev, chn)) {
				QString name = QString::fromStdString(iio_device_get_name(dev)) + ":" +
					QString::fromStdString(iio_channel_get_id(chn));

				UnitOfMeasurement *unitOfMeasurement = new UnitOfMeasurement("", "");
				DMMReadStrategy *dmmReadStrategy = new DMMReadStrategy(dev, chn);

				if(iioChannelHasAttribute(chn, "offset")) {
					double offset = 0;
					iio_channel_attr_read_double(chn, "offset", &offset);
					dmmReadStrategy->setOffset(offset);
				}

				int type = iio_channel_get_type(chn);
				if(type != iio_chan_type::IIO_CHAN_TYPE_UNKNOWN) {
					IIOUnit dmmInfo = m_iioDevices.value(static_cast<iio_chan_type>(type));

					if(isHwmon(dev, chn)) {
						dmmInfo = m_hwmonDevices.value(static_cast<hwmon_chan_type>(type));
					}
					unitOfMeasurement = new UnitOfMeasurement(dmmInfo.name, dmmInfo.symbol);
					dmmReadStrategy->setUmScale(dmmInfo.scale);
				}

				DmmDataMonitorModel *channelModel =
					new DmmDataMonitorModel(name, StyleHelper::getChannelColor(j),
								unitOfMeasurement, dmmReadStrategy);

				channelModel->setIioChannel(chn);
				channelModel->setIioDevice(dev);

				result.push_back(channelModel);
			}
		}
	}

	return result;
}

bool DMM::isDMMCompatible(iio_channel *chn)
{
	// DMM channels have raw and scale and be input
	if(!iio_channel_is_output(chn) && iioChannelHasAttribute(chn, "raw") && iioChannelHasAttribute(chn, "scale")) {
		return true;
	}
	return false;
}

bool DMM::isHwmon(iio_device *dev, iio_channel *chn)
{
	if(iio_device_is_hwmon(dev) && iioChannelHasAttribute(chn, "input")) {
		auto d_name = iio_device_get_name(dev);
		auto c_name = std::string(iio_channel_get_id(chn));
		if(!c_name.empty() && d_name) {
			return true;
		}
	}

	return false;
}

bool DMM::iioChannelHasAttribute(iio_channel *chn, const std::string &attr)
{
	unsigned int nb_attr = iio_channel_get_attrs_count(chn);
	const char *attr_name;
	for(unsigned int i = 0; i < nb_attr; i++) {
		attr_name = iio_channel_get_attr(chn, i);
		std::size_t found = std::string(attr_name).find(attr);
		if(found != std::string::npos) {
			return true;
		}
	}
	return false;
}

void DMM::generateDictionaries()
{
	m_hwmonDevices = IIOUnitsManager::hwmonChannelTypes();
	m_iioDevices = IIOUnitsManager::iioChannelTypes();
}

QMap<iio_chan_type, IIOUnit> DMM::iioDevices() const { return m_iioDevices; }

QMap<hwmon_chan_type, IIOUnit> DMM::hwmonDevices() const { return m_hwmonDevices; }

#include "moc_dmm.cpp"
