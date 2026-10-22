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

#include "devicefactory.h"

#include <QThread>

#include <component/backends/iio/iioctxbackend.h>
#include <pluginbase/preferences.h>

using namespace scopy;

DeviceImpl *DeviceFactory::build(QString param, QString category, QObject *parent)
{
	int maxThreads = Preferences::get("dc_cmd_execution_parallel").toBool() ? QThread::idealThreadCount() : 1;
	if(category.compare("iio", Qt::CaseInsensitive) == 0) {
		return new DeviceImpl(param, category, component::iio::IIOCtxBackend::ID, maxThreads, parent);
	}
	return new DeviceImpl(param, category, {}, 1, parent);
}

#include "moc_devicefactory.cpp"