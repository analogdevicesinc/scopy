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

#include "m2kdeviceimpl.h"

#include <QThread>

#include <component/backends/iio/iioctxbackend.h>
#include <component/context.h>
#include <component/controller.h>
#include <component/device.h>
#include <pluginbase/preferences.h>

using namespace scopy;

component::BackendId DeviceFactory::backendFor(const QString &category)
{
	const QList<component::BackendId> ids = component::Controller::GetInstance()->backends();
	return ids.contains(category) ? category : component::BackendId();
}

DeviceImpl *DeviceFactory::build(QString param, QString category, QObject *parent)
{
	const component::BackendId backend = backendFor(category);
	if(backend.isEmpty()) {
		return new DeviceImpl(param, category, {}, 1, parent);
	}
	int maxThreads = Preferences::get("dc_cmd_execution_parallel").toBool() ? QThread::idealThreadCount() : 1;
	if(backend == component::iio::IIOCtxBackend::ID) {
		component::ContextHandle ctx = component::Controller::connectCtx(param, backend, maxThreads);
		if(ctx && ctx->findChild<component::Device *>("m2k-adc", Qt::FindDirectChildrenOnly) &&
		   ctx->findChild<component::Device *>("m2k-dac-a", Qt::FindDirectChildrenOnly) &&
		   ctx->findChild<component::Device *>("m2k-dac-b", Qt::FindDirectChildrenOnly)) {
			return new M2kDeviceImpl(param, maxThreads, parent);
		}
	}
	return new DeviceImpl(param, category, backend, maxThreads, parent);
}

#include "moc_devicefactory.cpp"