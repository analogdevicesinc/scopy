#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "device.h"
#include <scopycore_export.h>

namespace adiscope {
class SCOPYCORE_EXPORT DeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DeviceManager(QObject *parent = nullptr);
	Device* getDevice(QString uri);
	void setExclusive(bool);
	bool getExclusive() const;

public Q_SLOTS:
	void addDevice(QString uri);
	void removeDevice(QString uri);
//	void updateScan(QStringList ls);

private Q_SLOTS:
	void changeToolListDevice();
	void connectDevice();
	void disconnectDevice();

Q_SIGNALS:
	void deviceChangedToolList(QString, QList<ToolMenuEntry>);
	void deviceAdded(QString,Device*);
	void deviceRemoved(QString);
	void deviceConnected(QString uri);
	void deviceDisconnected(QString uri);


private:
	bool exclusive = false;
	QStringList scannedDev;
	QStringList connectedDev;
	QMap<QString,Device*> map;

};
}

#endif // DEVICEMANAGER_H
