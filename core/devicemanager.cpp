#include "devicemanager.h"
#include "deviceimpl.h"
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_DEVICEMANAGER, "DeviceManager")
using namespace adiscope;
DeviceManager::DeviceManager(QObject *parent)
	: QObject{parent}
{

}

Device* DeviceManager::getDevice(QString uri) {

	Device *d = nullptr;
	if(map.contains(uri)) {
		d = map.value(uri, nullptr);
	}
	return d;
}

void DeviceManager::addDevice(QString uri)
{	
	qInfo(CAT_DEVICEMANAGER) << "device" << uri << "added";
	Device *d = nullptr;
	if(map.contains(uri)) {
//		d = map.value(uri,nullptr);
		qWarning(CAT_DEVICEMANAGER)<<"Duplicate" << uri <<" in device manager, removing old value";
		removeDevice(uri);
	}

	d = new DeviceImpl(uri, this);//DeviceFactory::newDevice(uri);
	d->loadPlugins();

	connect(dynamic_cast<QObject*>(d),SIGNAL(connected()),this,SLOT(connectDevice()));
	connect(dynamic_cast<QObject*>(d),SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	connect(dynamic_cast<QObject*>(d),SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));

	map[uri] = d;

	if(d)
		Q_EMIT deviceAdded(uri, d);
}

void DeviceManager::changeToolListDevice() {
	QString uri = dynamic_cast<Device*>(QObject::sender())->uri();
	Q_EMIT deviceChangedToolList(uri, map[uri]->toolList());
}

void DeviceManager::connectDevice() {
	QString uri = dynamic_cast<Device*>(QObject::sender())->uri();
	qDebug(CAT_DEVICEMANAGER)<<"connecting " << uri << "...";
	if(connectedDev.contains(uri)) {
		qDebug(CAT_DEVICEMANAGER)<<"connecting to the same device, disconnecting first .. ";
		map[uri]->disconnectDev();
	}
	if(exclusive) {
		if(connectedDev.size() > 0) {
			qDebug(CAT_DEVICEMANAGER)<<"exclusive mode, disconnecting all connected devices .. ";
			for(int i = 0;i<connectedDev.count();i++)
				map[connectedDev[i]]->disconnectDev();
		}
	}

	connectedDev.append(uri);
	Q_EMIT deviceConnected(uri);
}

void DeviceManager::disconnectDevice() {
	QString uri = dynamic_cast<Device*>(QObject::sender())->uri();
	qDebug(CAT_DEVICEMANAGER)<<"disconnecting "<< uri << "...";
	connectedDev.removeOne(uri);
	Q_EMIT deviceDisconnected(uri);
}

void DeviceManager::setExclusive(bool val) {
	exclusive = val;
}
bool DeviceManager::getExclusive() const {
	return exclusive;
}

void DeviceManager::removeDevice(QString uri)
{	
	Device *d = nullptr;
	if(map.contains(uri)) {
		d = map.take(uri);
		delete(d);
	}
	qInfo(CAT_DEVICEMANAGER) << "device" << uri << "removed";
	Q_EMIT deviceRemoved(uri);
}
