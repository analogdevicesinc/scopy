#include "deviceautoconnect.h"

#include <pluginbase/preferences.h>
#include <QList>

using namespace scopy;

void DeviceAutoConnect::initPreferences() { Preferences::init("autoconnect_devices", QMap<QString, QVariant>()); }

void DeviceAutoConnect::addDevice(QString uri, QStringList plugins)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	if(!devicesMap.contains(uri) && !plugins.isEmpty()) {
		devicesMap[uri] = plugins.join(";");
		Preferences::set("autoconnect_devices", devicesMap);
	}
}

void DeviceAutoConnect::removeDevice(QString uri)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	QString prefId = uri + "_sticky";
	if(devicesMap.contains(uri) && !Preferences::get(prefId).toBool()) {
		devicesMap.remove(uri);
		Preferences::set("autoconnect_devices", devicesMap);
	}
}

void DeviceAutoConnect::clear()
{
	const QStringList uris = Preferences::get("autoconnect_devices").toMap().keys();
	for(const QString &key : uris) {
		removeDevice(key);
	}
}

bool DeviceAutoConnect::isAutoConnectEnabled(QString uri)
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	return devicesMap.contains(uri);
}
