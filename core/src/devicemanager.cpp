#include "devicemanager.h"
#include "deviceimpl.h"
#include "iiodeviceimpl.h"
#include "QApplication"
#include <QLoggingCategory>
#include <QDebug>
#include <QThread>

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

Device* DeviceManager::getDevice(QString id) {

	Device *d = nullptr;
	if(map.contains(id)) {
		d = map.value(id, nullptr);
	}
	return d;
}

QString DeviceManager::addDevice(QString category, QString param)
{	
	static bool threaded = true;

	qInfo(CAT_DEVICEMANAGER) <<category<< "device with params" << param << "added";
	Q_EMIT deviceAddStarted(param);

	DeviceImpl *d = nullptr;

	d = new DeviceImpl(param, pm, category);
	QString id = d->id();
	map[id] = d;

	if(threaded) {
		QThread *th = QThread::create([=]{
			d->loadCompatiblePlugins();
			d->compatiblePreload();
		});
		dynamic_cast<QObject*>(d)->moveToThread(th);

		connect(th,&QThread::destroyed, this,[=]() {
			d->moveToThread(QThread::currentThread());
			d->setParent(this);
			d->loadPlugins();
			connectDeviceToManager(d);
			Q_EMIT deviceAdded(d->id(), d);
		}, Qt::QueuedConnection);
		connect(th,&QThread::finished, th, &QThread::deleteLater);
		th->start();
	} else {
		d->loadCompatiblePlugins();
		d->compatiblePreload();
		d->setParent(this);
		d->loadPlugins();
		connectDeviceToManager(d);
		Q_EMIT deviceAdded(d->id(), d);
	}
	return d->id();
}


// This is only used by scan context collector - should I rethink this ?
// Map all devices to uris in scan context collector
void DeviceManager::removeDevice(QString category, QString param) {

	for(Device* d : qAsConst(map)) {
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
		qWarning(CAT_DEVICEMANAGER) << id <<"Device does not exist";
		return;
	}
	d = map.take(id);
	Q_EMIT deviceRemoveStarted(id, d);

	disconnectDeviceFromManager(dynamic_cast<DeviceImpl*>(d));
	d->unloadPlugins();
	delete(d);

	qInfo(CAT_DEVICEMANAGER) << "device" << id << "removed";

	Q_EMIT deviceRemoved(id);
}

void DeviceManager::connectDeviceToManager(DeviceImpl *d) {
	connect(d,SIGNAL(connected()),this,SLOT(connectDevice()));
	connect(d,SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	connect(d,SIGNAL(requestedRestart()), this,SLOT(restartDevice()));
	connect(d,SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));
	connect(d,SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));

}
void DeviceManager::disconnectDeviceFromManager(DeviceImpl *d) {
	disconnect(d,SIGNAL(connected()),this,SLOT(connectDevice()));
	disconnect(d,SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	disconnect(d,SIGNAL(requestedRestart()), this,SLOT(restartDevice()));
	disconnect(d,SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));
	disconnect(d,SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
}

QString DeviceManager::restartDevice(QString id)
{
	QString cat = map[id]->category();
	QString params = map[id]->param();
	removeDeviceById(id);
	QString newId = addDevice(cat, params);
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

void DeviceManager::changeToolListDevice() {
	QString id = dynamic_cast<Device*>(QObject::sender())->id();
	Q_EMIT deviceChangedToolList(id, map[id]->toolList());
}

void DeviceManager::connectDevice() {
	QString id = dynamic_cast<Device*>(QObject::sender())->id();
	qDebug(CAT_DEVICEMANAGER)<<"connecting " << id << "...";
	if(connectedDev.contains(id)) {
		qDebug(CAT_DEVICEMANAGER)<<"connecting to the same device, disconnecting first .. ";
		map[id]->disconnectDev();
	}
	if(exclusive) {
		if(connectedDev.size() > 0) {
			qDebug(CAT_DEVICEMANAGER)<<"exclusive mode, disconnecting all connected devices .. ";
			for(int i = 0;i<connectedDev.count();i++)
				map[connectedDev[i]]->disconnectDev();
		}
	}

	connectedDev.append(id);
	Q_EMIT deviceConnected(id, map[id]);
}

void DeviceManager::disconnectDevice() {
	QString id = dynamic_cast<Device*>(QObject::sender())->id();
	qDebug(CAT_DEVICEMANAGER)<<"disconnecting "<< id << "...";
	connectedDev.removeOne(id);
	Q_EMIT requestTool("home");
	Q_EMIT deviceDisconnected(id, map[id]);
}

void DeviceManager::setExclusive(bool val) {
	exclusive = val;
}
bool DeviceManager::getExclusive() const {
	return exclusive;
}

void DeviceManager::restartDevice() {
	QString id = dynamic_cast<Device*>(QObject::sender())->id();
	qDebug(CAT_DEVICEMANAGER)<<"restarting "<< id << "...";
	QString newId = restartDevice(id);
//	connect(this,SIGNAL(deviceAdded(QString,Device*)),this,SIGNAL(requestDevice(QString)));
//	Q_EMIT requestDevice(newId);
}

