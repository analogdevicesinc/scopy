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

#include "deviceautoconnect.h"

#include <pluginbase/preferences.h>
#include <QList>

using namespace scopy;

void DeviceAutoConnect::initPreferences() { Preferences::init("autoconnect_devices", QMap<QString, QVariant>()); }

void DeviceAutoConnect::addDevice(QString uri, QStringList plugins)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	if(!devicesMap.contains(uri) && !plugins.isEmpty()) {
		devicesMap[uri] = plugins.join(";");
		Preferences::set("autoconnect_devices", devicesMap);
	}
}

void DeviceAutoConnect::removeDevice(QString uri)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	QString prefId = uri + "_sticky";
	if(devicesMap.contains(uri) && !Preferences::get(prefId).toBool()) {
		devicesMap.remove(uri);
		Preferences::set("autoconnect_devices", devicesMap);
	}
}

void DeviceAutoConnect::clear()
{
	const QStringList uris = Preferences::get("autoconnect_devices").toMap().keys();
	for(const QString &key : uris) {
		removeDevice(key);
	}
}

bool DeviceAutoConnect::isAutoConnectEnabled(QString uri)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	return devicesMap.contains(uri);
}
