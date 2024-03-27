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
#include <iioutil/commandqueue.h>

extern "C"
{
	struct iio_channel;
	struct iio_device;
}

namespace scopy::swiotrefactor {
class ConfigModel : public QObject
{
	Q_OBJECT
public:
	explicit ConfigModel(struct iio_device *device, int channelId, CommandQueue *m_commandQueue);
	~ConfigModel();

	void readEnabled();
	void writeEnabled(const QString &enabled);

	void readDevice();
	void writeDevice(const QString &device);

	void readFunction();
	void writeFunction(const QString &function);

	void readDeviceAvailable();
	void readFunctionAvailable();
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
	struct iio_device *m_device;
	int m_channelId;

	QString m_enableAttribute;
	QString m_functionAttribute;
	QString m_functionAvailableAttribute;
	QString m_deviceAttribute;
	QString m_deviceAvailableAttribute;

	QString m_enabled;
	QStringList m_availableDevices;
	QStringList m_availableFunctions;
	CommandQueue *m_commandQueue;
};
} // namespace scopy::swiotrefactor
#endif // CONFIGMODEL_H
