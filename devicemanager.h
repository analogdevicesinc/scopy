#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include "device.h"

namespace adiscope {
class DeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DeviceManager(QObject *parent = nullptr);
	Device* getDevice(QString uri);

public Q_SLOTS:
	void addDevice(QString uri);
	void removeDevice(QString uri);

Q_SIGNALS:
	void deviceAdded(QString, Device*);
	void deviceRemoved(QString);
	void deviceConnected(QString uri); // ???
	void deviceDisconnected(QString uri); // ???


private:
	QMap<QString,Device*> map;

};
}

#endif // DEVICEMANAGER_H
