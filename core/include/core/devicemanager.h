/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
	bool busy();
	int connectedDeviceCount();
	void saveSessionDevices();

public Q_SLOTS:

	void addDevice(Device *d);
	QString createDevice(QString category, QString param, bool async = true,
			     QList<QString> plugins = QList<QString>());
	void removeDevice(QString category, QString id);

	void removeDeviceById(QString id);
	QString restartDevice(QString id);
	void disconnectAll();

	void save(QSettings &s);
	void load(QSettings &s);
	void requestConnectedDev();
	//	void updateScan(QStringList ls);

Q_SIGNALS:
	void connectedDevices(QMap<QString, QStringList> connectedDev);

private Q_SLOTS:
	void changeToolListDevice();
	void connectingDevice();
	void connectDevice();
	void disconnectingDevice();
	void disconnectDevice();
	void restartDevice();

Q_SIGNALS:
	void deviceChangedToolList(QString, QList<ToolMenuEntry *>);
	void deviceAddStarted(QString);
	void deviceAdded(QString, Device *);
	void deviceRemoveStarted(QString, Device *);
	void deviceRemoved(QString);
	void deviceConnecting(QString);
	void deviceConnected(QString id, Device *);
	void deviceDisconnecting(QString id);
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

	void disconnectingDevice(QString id);
	void disconnectDevice(QString id);
	void connectingDevice(QString id);
	void connectDevice(QString id);
};
} // namespace scopy

#endif // DEVICEMANAGER_H
