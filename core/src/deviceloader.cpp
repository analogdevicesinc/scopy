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

#include "deviceloader.h"

#include <QThread>

using namespace scopy;

DeviceLoader::DeviceLoader(DeviceImpl *d, QObject *parent)
	: d(d)
	, QObject(parent)
{}

DeviceLoader::~DeviceLoader() {}

void DeviceLoader::init(bool async)
{
	if(async) {
		asyncInit();
	} else {
		syncInit();
	}
}

void DeviceLoader::asyncInit()
{
	QThread *th = QThread::create([=] {
		// initializer thread
		d->init();
	});
	oldParent = d->parent();
	d->setParent(nullptr);
	d->moveToThread(th);

	connect(
		th, &QThread::destroyed, this,
		[=]() {
			;
			// back to main thread
			d->moveToThread(QThread::currentThread());
			d->setParent(oldParent);
			Q_EMIT initialized();
		},
		Qt::QueuedConnection);
	connect(th, &QThread::finished, th, &QThread::deleteLater);

	th->start();
}

void DeviceLoader::syncInit()
{
	d->init();
	Q_EMIT initialized();
}

#include "moc_deviceloader.cpp"
