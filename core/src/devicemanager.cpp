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

Device* DeviceManager::getDevice(QString uri) {

	Device *d = nullptr;
	if(map.contains(uri)) {
		d = map.value(uri, nullptr);
	}
	return d;
}

void DeviceManager::addDevice(QString uri)
{	
	static bool threaded = false;

	qInfo(CAT_DEVICEMANAGER) << "device" << uri << "added";
	Q_EMIT deviceAddStarted(uri);

	DeviceImpl *d = nullptr;
	if(map.contains(uri)) {
		qWarning(CAT_DEVICEMANAGER)<<"Duplicate" << uri <<" in device manager, removing old value";
		removeDevice(uri);
	}

	d = new DeviceImpl(uri, pm);
	map[uri] = d;

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
		}, Qt::QueuedConnection);
		connect(th,&QThread::finished, th, &QThread::deleteLater);
		th->start();
	} else {
		d->loadCompatiblePlugins();
		d->compatiblePreload();
		d->setParent(this);
		d->loadPlugins();
		connectDeviceToManager(d);
	}
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

	disconnectDeviceFromManager(dynamic_cast<DeviceImpl*>(d));
	d->unloadPlugins();
	delete(d);

	qInfo(CAT_DEVICEMANAGER) << "device" << uri << "removed";

	Q_EMIT deviceRemoved(uri);
}

void DeviceManager::connectDeviceToManager(DeviceImpl *d) {
	if(!d)
		return;

	connect(d,SIGNAL(connected()),this,SLOT(connectDevice()));
	connect(d,SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	connect(d,SIGNAL(requestedRestart()), this,SLOT(restartDevice()));
	connect(d,SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));
	connect(d,SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
	Q_EMIT deviceAdded(d->uri(), d);
}
void DeviceManager::disconnectDeviceFromManager(DeviceImpl *d) {
	disconnect(d,SIGNAL(connected()),this,SLOT(connectDevice()));
	disconnect(d,SIGNAL(disconnected()),this,SLOT(disconnectDevice()));
	disconnect(d,SIGNAL(requestedRestart()), this,SLOT(restartDevice()));
	disconnect(d,SIGNAL(toolListChanged()),this,SLOT(changeToolListDevice()));
	disconnect(d,SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
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

