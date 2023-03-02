#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "device.h"
#include "deviceimpl.h"
#include "pluginmanager.h"
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT DeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DeviceManager(PluginManager *pm, QObject *parent = nullptr);
	~DeviceManager();
	Device* getDevice(QString uri);
	void setExclusive(bool);
	bool getExclusive() const;

public Q_SLOTS:
	void addDevice(QString uri);
	void removeDevice(QString uri);
	void restartDevice(QString uri);
	void disconnectAll();

	void save(QSettings &s);
	void load(QSettings &s);
//	void updateScan(QStringList ls);

private Q_SLOTS:
	void changeToolListDevice();
	void connectDevice();
	void disconnectDevice();
	void restartDevice();

Q_SIGNALS:
	void deviceChangedToolList(QString, QList<ToolMenuEntry*>);
	void deviceAddStarted(QString);
	void deviceAdded(QString,Device*);
	void deviceRemoveStarted(QString, Device*);
	void deviceRemoved(QString);
	void deviceConnected(QString uri);
	void deviceDisconnected(QString uri);
	void requestDevice(QString uri);
	void requestTool(QString id);
private:
	void connectDeviceToManager(DeviceImpl *d);
	void disconnectDeviceFromManager(DeviceImpl *d);
private:
	bool exclusive = false;
	QStringList scannedDev;
	QStringList connectedDev;
	QMap<QString,Device*> map;
	PluginManager *pm;

};
}

#endif // DEVICEMANAGER_H
