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

#include "devicemanager.h"

#include "QApplication"
#include "devicefactory.h"
#include "deviceimpl.h"
#include "deviceloader.h"
#include "pluginbase/statusbarmanager.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <QtConcurrent>

Q_LOGGING_CATEGORY(CAT_DEVICEMANAGER, "DeviceManager")
using namespace scopy;
DeviceManager::DeviceManager(PluginManager *pm, QObject *parent)
	: QObject{parent}
	, pm(pm)
{

	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(disconnectAll()));
}

DeviceManager::~DeviceManager() {}

Device *DeviceManager::getDevice(QString id)
{

	Device *d = nullptr;
	if(map.contains(id)) {
		d = map.value(id, nullptr);
	}
	return d;
}

void DeviceManager::addDevice(Device *d)
{

	DeviceImpl *di = dynamic_cast<DeviceImpl *>(d);
	QString id = d->id();
	map[id] = d;
	di->setParent(this);
	di->loadPlugins();
	connectDeviceToManager(di);
	Q_EMIT deviceAdded(id, d);
}

QString DeviceManager::createDevice(QString category, QString param, bool async, QList<QString> plugins)
{
	qInfo(CAT_DEVICEMANAGER) << category << "device with params" << param << "added";
	Q_EMIT deviceAddStarted(param);

	DeviceImpl *d = DeviceFactory::build(param, pm, category);
	DeviceLoader *dl = new DeviceLoader(d, this);

	connect(dl, &DeviceLoader::initialized, this, [=]() {
		QList<Plugin *> availablePlugins = d->plugins();
		if(!plugins.isEmpty()) {
			for(Plugin *p : qAsConst(availablePlugins)) {
				if(!plugins.contains(p->name())) {
					p->setEnabled(false);
				}
			}
		}
		addDevice(d);
	}); // add device to manager once it is initialized
	connect(dl, &DeviceLoader::initialized, dl,
		&QObject::deleteLater); // don't forget to delete loader once we're done
	dl->init(async);

	return d->id();
}

// This is only used by scan context collector - should I rethink this ?
// Map all devices to uris in scan context collector
void DeviceManager::removeDevice(QString category, QString param)
{

	for(Device *d : qAsConst(map)) {
		if(d->category() == category && d->param() == param) {
			removeDeviceById(d->id());
			return;
		}
	}
}

void DeviceManager::removeDeviceById(QString id)
{
	Device *d = nullptr;

	if(connectedDev.contains(id)) {
		getDevice(id)->disconnectDev();
	}

	if(!map.contains(id)) {
		qWarning(CAT_DEVICEMANAGER) << id << "Device does not exist";
		return;
	}
	d = map.take(id);
	Q_EMIT deviceRemoveStarted(id, d);

	disconnectDeviceFromManager(dynamic_cast<DeviceImpl *>(d));
	d->unloadPlugins();
	delete(d);

	qInfo(CAT_DEVICEMANAGER) << "device" << id << "removed";

	Q_EMIT deviceRemoved(id);
}

void DeviceManager::connectDeviceToManager(DeviceImpl *d)
{
	connect(d, &DeviceImpl::connecting, this, [=]() { connectingDevice(d->id()); });
	connect(d, &DeviceImpl::connected, this, [=]() { connectDevice(d->id()); });
	connect(d, &DeviceImpl::disconnected, this, [=]() { disconnectDevice(d->id()); });
	connect(d, &DeviceImpl::forget, this, [=]() { removeDeviceById(d->id()); });
	connect(d, SIGNAL(requestedRestart()), this, SLOT(restartDevice()));
	connect(d, SIGNAL(toolListChanged()), this, SLOT(changeToolListDevice()));
	connect(d, SIGNAL(requestTool(QString)), this, SIGNAL(requestTool(QString)));
}

void DeviceManager::disconnectDeviceFromManager(DeviceImpl *d)
{
	disconnect(d, SIGNAL(connecting()));
	disconnect(d, SIGNAL(connected()));
	disconnect(d, SIGNAL(disconnected()));
	disconnect(d, SIGNAL(forget()));
	disconnect(d, SIGNAL(requestedRestart()), this, SLOT(restartDevice()));
	disconnect(d, SIGNAL(toolListChanged()), this, SLOT(changeToolListDevice()));
	disconnect(d, SIGNAL(requestTool(QString)), this, SIGNAL(requestTool(QString)));
}

QString DeviceManager::restartDevice(QString id)
{
	QString cat = map[id]->category();
	QString params = map[id]->param();
	removeDeviceById(id);
	QString newId = createDevice(cat, params);
	return newId;
}

void DeviceManager::disconnectAll()
{
	for(const QString &d : qAsConst(connectedDev)) {
		map[d]->disconnectDev();
	}
}

void DeviceManager::save(QSettings &s)
{
	for(const QString &d : qAsConst(connectedDev)) {
		map[d]->save(s);
	}
}

void DeviceManager::load(QSettings &s)
{
	for(const QString &d : qAsConst(connectedDev)) {
		map[d]->load(s);
	}
}

void DeviceManager::changeToolListDevice()
{
	QString id = dynamic_cast<Device *>(QObject::sender())->id();
	Q_EMIT deviceChangedToolList(id, map[id]->toolList());
}

void DeviceManager::connectDevice()
{
	QString id = dynamic_cast<Device *>(QObject::sender())->id();
	connectDevice(id);
}

void DeviceManager::connectingDevice()
{
	QString id = dynamic_cast<Device *>(QObject::sender())->id();
	connectingDevice(id);
}

void DeviceManager::connectingDevice(QString id)
{
	qDebug(CAT_DEVICEMANAGER) << "connecting " << id << "...";
	if(connectedDev.contains(id)) {
		qDebug(CAT_DEVICEMANAGER) << "connecting to the same device, disconnecting first .. ";
		map[id]->disconnectDev();
	}
	if(exclusive) {
		if(connectedDev.size() > 0) {
			qDebug(CAT_DEVICEMANAGER) << "exclusive mode, disconnecting all connected devices .. ";
			for(int i = 0; i < connectedDev.count(); i++)
				map[connectedDev[i]]->disconnectDev();
		}
	}
	Q_EMIT deviceConnecting(id);
}

void DeviceManager::connectDevice(QString id)
{
	connectedDev.append(id);
	StatusBarManager::pushMessage("Connected to " + map[id]->id(), 3000);
	Q_EMIT deviceConnected(id, map[id]);
}

void DeviceManager::disconnectDevice()
{
	QString id = dynamic_cast<Device *>(QObject::sender())->id();
	disconnectDevice(id);
}

void DeviceManager::disconnectDevice(QString id)
{
	qDebug(CAT_DEVICEMANAGER) << "disconnecting " << id << "...";
	connectedDev.removeOne(id);
	StatusBarManager::pushMessage("Disconnected from " + map[id]->id(), 3000);
	Q_EMIT requestTool("home");
	Q_EMIT deviceDisconnected(id, map[id]);
}

void DeviceManager::setExclusive(bool val)
{
	exclusive = val;
	if(val == true && connectedDev.size() > 1) {
		disconnectAll();
	}
}
bool DeviceManager::getExclusive() const { return exclusive; }

void DeviceManager::restartDevice()
{
	QString id = dynamic_cast<Device *>(QObject::sender())->id();
	qDebug(CAT_DEVICEMANAGER) << "restarting " << id << "...";
	QString newId = restartDevice(id);
	//	connect(this,SIGNAL(deviceAdded(QString,Device*)),this,SIGNAL(requestDevice(QString)));
	//	Q_EMIT requestDevice(newId);
}

#include "moc_devicemanager.cpp"
