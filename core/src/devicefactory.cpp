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

#include <component/controller.h>
#include <pluginbase/preferences.h>

using namespace scopy;

component::ConnectionKind DeviceFactory::kindFor(const QString &category)
{
	const QList<component::ConnectionKind> kinds = component::Controller::GetInstance()->providers();
	return kinds.contains(category) ? category : component::ConnectionKind();
}

DeviceImpl *DeviceFactory::build(QString param, QString category, QObject *parent)
{
	const component::ConnectionKind kind = kindFor(category);
	if(kind.isEmpty()) {
		return new DeviceImpl(param, category, {}, 1, parent);
	}
	int maxThreads = Preferences::get("dc_cmd_execution_parallel").toBool() ? QThread::idealThreadCount() : 1;
	return new DeviceImpl(param, category, kind, maxThreads, parent);
}

#include "moc_devicefactory.cpp"