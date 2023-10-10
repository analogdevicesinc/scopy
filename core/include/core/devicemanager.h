#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "device.h"
#include "deviceimpl.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"

#include <QMap>
#include <QObject>
#include <QSet>

namespace scopy {
class SCOPY_CORE_EXPORT DeviceManager : public QObject
{
	Q_OBJECT
	friend class ScopyMainWindow_API;

public:
	explicit DeviceManager(PluginManager *pm, QObject *parent = nullptr);
	~DeviceManager();
	Device *getDevice(QString id);
	void setExclusive(bool);
	bool getExclusive() const;

public Q_SLOTS:

	void addDevice(Device *d);
	QString createDevice(QString category, QString param, bool async = true);
	void removeDevice(QString category, QString id);

	void removeDeviceById(QString id);
	QString restartDevice(QString id);
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
	void deviceChangedToolList(QString, QList<ToolMenuEntry *>);
	void deviceAddStarted(QString);
	void deviceAdded(QString, Device *);
	void deviceRemoveStarted(QString, Device *);
	void deviceRemoved(QString);
	void deviceConnected(QString id, Device *);
	void deviceDisconnected(QString id, Device *);
	void requestDevice(QString id);
	void requestTool(QString id);

private:
	void connectDeviceToManager(DeviceImpl *d);
	void disconnectDeviceFromManager(DeviceImpl *d);

private:
	bool exclusive = false;
	QStringList scannedDev;
	QStringList connectedDev;
	QMap<QString, Device *> map;
	PluginManager *pm;

	void disconnectDevice(QString id);
	void connectDevice(QString id);
};
} // namespace scopy

#endif // DEVICEMANAGER_H
