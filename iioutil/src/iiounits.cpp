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

#include <iiounits.h>
#include <QApplication>

using namespace scopy;

IIOUnitsManager *IIOUnitsManager::pinstance_{nullptr};

IIOUnitsManager::IIOUnitsManager(QObject *parent)
	: QObject(parent)
{

	m_iioChannelTypes = QMap<iio::ChannelType, IIOUnit>({
		{iio::CHAN_VOLTAGE, {"Voltage", "V", 0.001}},
		{iio::CHAN_TEMP, {"Degree Celsius", "°C", 0.001}},
		{iio::CHAN_CURRENT, {"Ampere", "A", 0.001}},
		{iio::CHAN_PRESSURE, {"Pascal", "Pa", 1000}},
		{iio::CHAN_ACCEL, {"Metre per second squared", "m/s2", 1}},
		{iio::CHAN_ANGL_VEL, {"Radian per second", "rad/s", 1}},
		{iio::CHAN_MAGN, {"Gauss", "Gs", 1}},
		{iio::CHAN_POWER, {"Watt", "W", 0.000001}}
		/// and others
	});

	m_hwmonChannelTypes = {
		{iio::HWMON_CHAN_VOLTAGE, {"Voltage", "V", 0.001}},
		{iio::HWMON_CHAN_TEMP, {"Degree Celsius", "°C", 0.001}},
		{iio::HWMON_CHAN_CURRENT, {"Ampere", "A", 0.001}},
		{iio::HWMON_CHAN_POWER, {"Watt", "W", 0.000001}},
		{iio::HWMON_CHAN_ENERGY, {"Joule", "J", 0.000001}},
		{iio::HWMON_CHAN_FAN, {"Revolution/Min", "RPM", 1}},
		{iio::HWMON_CHAN_HUMIDITY, {"percent", "%", 0.001}}
		/// and others
	};
}

IIOUnitsManager *IIOUnitsManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new IIOUnitsManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

QMap<iio::ChannelType, IIOUnit> IIOUnitsManager::iioChannelTypes()
{
	return IIOUnitsManager::GetInstance()->_iioChannelTypes();
}

QMap<iio::HwmonChannelType, IIOUnit> IIOUnitsManager::hwmonChannelTypes()
{
	return IIOUnitsManager::GetInstance()->_hwmonChannelTypes();
}

IIOUnitsManager::~IIOUnitsManager() {}

QMap<iio::ChannelType, IIOUnit> IIOUnitsManager::_iioChannelTypes() { return m_iioChannelTypes; }

QMap<iio::HwmonChannelType, IIOUnit> IIOUnitsManager::_hwmonChannelTypes() { return m_hwmonChannelTypes; }

#include "moc_iiounits.cpp"
