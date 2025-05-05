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

#include "datamonitor/dmmdatamonitormodel.hpp"

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

DmmDataMonitorModel::DmmDataMonitorModel(QObject *parent) {}

DmmDataMonitorModel::DmmDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure,
					 DMMReadStrategy *readStrategy, QObject *parent)
	: ReadableDataMonitorModel(parent)
{
	setName(name);
	setColor(color);

	if(unitOfMeasure) {
		setUnitOfMeasure(unitOfMeasure);
	} else {
		setUnitOfMeasure(new UnitOfMeasurement("Volt", "V"));
	}

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("datamonitor")) {
			setDataStorageSize();
		}
	});

	setDataStorageSize();
	setReadStrategy(readStrategy);
}

iio_channel *DmmDataMonitorModel::iioChannel() const { return m_iioChannel; }

void DmmDataMonitorModel::setIioChannel(iio_channel *newIioChannel) { m_iioChannel = newIioChannel; }

QString DmmDataMonitorModel::getDeviceName() { return QString::fromStdString(iio_device_get_name(m_iioDevice)); }

iio_device *DmmDataMonitorModel::iioDevice() const { return m_iioDevice; }

void DmmDataMonitorModel::setIioDevice(iio_device *newIioDevice) { m_iioDevice = newIioDevice; }

#include "moc_dmmdatamonitormodel.cpp"
