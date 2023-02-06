#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include "device.h"


class DeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DeviceManager(QObject *parent = nullptr);
	/*Device* getDevice(QString uri);

public Q_SLOTS:
	void addDevice(QString uri);
	void forgetDevice(QString uri);

Q_SIGNALS:
	void deviceCompatible(QString uri);

*/
private:
	QMap<QString,Device> map;

};

#endif // DEVICEMANAGER_H
