/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef CONFIGMODEL_H
#define CONFIGMODEL_H

#include <QObject>

#include <QMap>
#include <qcoro/qcorotask.h>

namespace scopy {
namespace component {
class Device;
class Attribute;
} // namespace component

namespace swiot {
class ConfigModel : public QObject
{
	Q_OBJECT
public:
	explicit ConfigModel(component::Device *device, int channelId);
	~ConfigModel();

	QCoro::Task<void> readEnabled();
	QCoro::Task<void> writeEnabled(const QString &enabled);

	QCoro::Task<void> readDevice();
	QCoro::Task<void> writeDevice(const QString &device);

	QCoro::Task<void> readFunction();
	QCoro::Task<void> writeFunction(const QString &function);

	QCoro::Task<void> readDeviceAvailable();
	QCoro::Task<void> readFunctionAvailable();
Q_SIGNALS:
	void readConfigChannelEnabled(bool);
	void readConfigChannelDevice(QString);
	void readConfigChannelFunction(QString);
	void readConfigChannelDeviceAvailable(QStringList);
	void readConfigChannelFunctionAvailable(QStringList);

	void configChannelEnabled();
	void configChannelDevice();
	void configChannelFunction();

private:
	component::Attribute *getAttr(const QString &name);

	component::Device *m_device;
	int m_channelId;

	QString m_enableAttribute;
	QString m_functionAttribute;
	QString m_functionAvailableAttribute;
	QString m_deviceAttribute;
	QString m_deviceAvailableAttribute;

	QString m_enabled;
	QStringList m_availableDevices;
	QStringList m_availableFunctions;
};
} // namespace swiot
} // namespace scopy
#endif // CONFIGMODEL_H
