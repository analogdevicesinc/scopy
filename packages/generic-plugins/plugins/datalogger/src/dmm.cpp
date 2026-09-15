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

#include <qcoro/qcorotask.h>
#include <component/attributereader.h>
#include <component/backends/iio/iiochannel.h>

using namespace scopy;
using namespace datamonitor;

namespace {
// Read a scalar attribute value once, synchronously (discovery-time only).
double readAttrOnce(component::Attribute *attr, double fallback)
{
	if(!attr || !attr->readCapability()) {
		return fallback;
	}
	auto response = QCoro::waitFor(attr->readCapability()->readAsync());
	if(!response) {
		return fallback;
	}
	return QString::fromUtf8(response.value()).toDouble();
}
} // namespace

DMM::DMM(QObject *parent)
	: QObject{parent}
{
	generateDictionaries();
}

QList<DmmDataMonitorModel *> DMM::getDmmMonitors(component::Context *ctx)
{
	QList<DmmDataMonitorModel *> result;

	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIODevice *dev : devices) {
		const QList<component::Channel *> channels =
			dev->findChildren<component::Channel *>(Qt::FindDirectChildrenOnly);
		for(component::Channel *chn : channels) {
			// if the channel is DMM or hwmon we create a monitor for it
			if(isDMMCompatible(chn) || isHwmon(dev, chn)) {
				QString name = dev->name() + ":" + chn->id();

				UnitOfMeasurement *unitOfMeasurement = new UnitOfMeasurement("", "");

				// DMM reads "raw", hwmon reads "input".
				component::Attribute *readAttr = channelAttribute(chn, "raw");
				if(!readAttr) {
					readAttr = channelAttribute(chn, "input");
				}
				DMMReadStrategy *dmmReadStrategy = new DMMReadStrategy(readAttr);

				component::Attribute *offsetAttr = channelAttribute(chn, "offset");
				bool hasOffset = offsetAttr != nullptr;
				double offset = readAttrOnce(offsetAttr, 0);

				component::Attribute *scaleAttr = channelAttribute(chn, "scale");
				bool hasScale = scaleAttr != nullptr;
				double scale = readAttrOnce(scaleAttr, 1);

				component::iio::IIOChannel *iioChnl = dynamic_cast<component::iio::IIOChannel *>(chn);
				int type = iioChnl ? iioChnl->chanType() : iio_chan_type::IIO_CHAN_TYPE_UNKNOWN;
				if(type != iio_chan_type::IIO_CHAN_TYPE_UNKNOWN) {
					IIOUnit dmmInfo = m_iioDevices.value(static_cast<iio_chan_type>(type));

					if(isHwmon(dev, chn)) {
						dmmInfo = m_hwmonDevices.value(static_cast<hwmon_chan_type>(type));
					}
					unitOfMeasurement = new UnitOfMeasurement(dmmInfo.name, dmmInfo.symbol);
					scale = scale * dmmInfo.scale;
				}

				DmmDataMonitorModel *channelModel =
					new DmmDataMonitorModel(name, StyleHelper::getChannelColor(result.size()),
								unitOfMeasurement, scale, offset, dmmReadStrategy);
				channelModel->setChannel(chn);
				channelModel->setDeviceName(dev->name());
				channelModel->setHasOffset(hasOffset);
				channelModel->setHasScale(hasScale);

				if(!chn->name().isEmpty()) {
					channelModel->setDisplayName(chn->name());
				}

				result.push_back(channelModel);
			}
		}
	}

	return result;
}

bool DMM::isDMMCompatible(component::Channel *chn)
{
	// DMM channels have raw and be input (scale is optional, defaults to 1)
	if(!chn->isOutput() && channelAttribute(chn, "raw")) {
		return true;
	}
	return false;
}

bool DMM::isHwmon(component::iio::IIODevice *dev, component::Channel *chn)
{
	if(dev->isHwmon() && channelAttribute(chn, "input")) {
		if(!chn->id().isEmpty() && !dev->name().isEmpty()) {
			return true;
		}
	}

	return false;
}

component::Attribute *DMM::channelAttribute(component::Channel *chn, const QString &attr)
{
	const QList<component::Attribute *> attrs =
		chn->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
	for(component::Attribute *a : attrs) {
		if(a->name().contains(attr)) {
			return a;
		}
	}
	return nullptr;
}

void DMM::generateDictionaries()
{
	m_hwmonDevices = IIOUnitsManager::hwmonChannelTypes();
	m_iioDevices = IIOUnitsManager::iioChannelTypes();
}

QMap<iio_chan_type, IIOUnit> DMM::iioDevices() const { return m_iioDevices; }

QMap<hwmon_chan_type, IIOUnit> DMM::hwmonDevices() const { return m_hwmonDevices; }

#include "moc_dmm.cpp"
