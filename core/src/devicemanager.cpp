#include "devicemanager.h"
#include "deviceimpl.h"
#include "iiodeviceimpl.h"
#include "QCoreApplication"
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_DEVICEMANAGER, "DeviceManager")
using namespace adiscope;
DeviceManager::DeviceManager(PluginManager *pm, QObject *parent)
	: QObject{parent}, pm(pm)
{

	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),this,SLOT(disconnectAll()));
}

DeviceManager::~DeviceManager()
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
	Q_EMIT deviceAddStarted(uri);

	Device *d = nullptr;
	if(map.contains(uri)) {
		qWarning(CAT_DEVICEMANAGER)<<"Duplicate" << uri <<" in device manager, removing old value";
		removeDevice(uri);
	}

	d = new DeviceImpl(uri, pm, this);
	//DeviceFactory::newDevice(uri);
	//d = new IIODeviceImpl(uri, pm, this);

	d->loadPlugins();

	connect(dynamic_cast<DeviceImpl*>(d),SIGNAL(connected()),this,SLOT(connectDevice()));
	connect(dynamic_cast<DeviceImpl*>(d),SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	connect(dynamic_cast<DeviceImpl*>(d),SIGNAL(requestedRestart()), this,SLOT(restartDevice()));
	connect(dynamic_cast<DeviceImpl*>(d),SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));
	connect(dynamic_cast<DeviceImpl*>(d),SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));

	map[uri] = d;

	if(d)
		Q_EMIT deviceAdded(uri, d);
}


void DeviceManager::removeDevice(QString uri)
{
	Device *d = nullptr;

	if(connectedDev.contains(uri)) {
		getDevice(uri)->disconnectDev();
	}

	if(!map.contains(uri)) {
		qWarning(CAT_DEVICEMANAGER) << uri <<"Device does not exist";
		return;
	}
	d = map.take(uri);
	Q_EMIT deviceRemoveStarted(uri, d);
	disconnect(dynamic_cast<QObject*>(d),SIGNAL(connected()),this,SLOT(connectDevice()));
	disconnect(dynamic_cast<QObject*>(d),SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	disconnect(dynamic_cast<QObject*>(d),SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));

	d->unloadPlugins();
	delete(d);

	qInfo(CAT_DEVICEMANAGER) << "device" << uri << "removed";

	Q_EMIT deviceRemoved(uri);
}

void DeviceManager::restartDevice(QString uri)
{
	removeDevice(uri);
	addDevice(uri);
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
	Q_EMIT requestTool("home");
	Q_EMIT deviceDisconnected(uri);
}

void DeviceManager::setExclusive(bool val) {
	exclusive = val;
}
bool DeviceManager::getExclusive() const {
	return exclusive;
}

void DeviceManager::restartDevice() {
	QString uri = dynamic_cast<Device*>(QObject::sender())->uri();
	qDebug(CAT_DEVICEMANAGER)<<"restarting "<< uri << "...";
	restartDevice(uri);
	Q_EMIT requestDevice(uri);
}

