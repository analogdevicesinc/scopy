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
		d = map.value(uri,nullptr);
		qWarning(CAT_DEVICEMANAGER)<<"Duplicate" << uri <<" in device manager, removing old value";
		removeDevice(uri);
	}
	else
	{

		// what to do here
	}

	d = new DeviceImpl(uri, this);//DeviceFactory::newDevice(uri);
	map[uri] = d;

	if(d)
		Q_EMIT deviceAdded(uri, d);
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
