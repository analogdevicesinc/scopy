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

#ifndef SCOPY_DIOCONTROLLER_H
#define SCOPY_DIOCONTROLLER_H

#include <QString>
#include <QList>
#include <QObject>

namespace scopy {
namespace component {
class Device;
class Channel;
} // namespace component

namespace swiot {
class DioController : public QObject
{
	Q_OBJECT
public:
	explicit DioController(component::Device *device, QString deviceName = "max14906");
	~DioController() override;

	int getChannelCount();

	component::Device *getDevice() const;
	component::Channel *getChannel(unsigned int index);

	QString getChannelName(unsigned int index);
	QString getChannelType(unsigned int index);

private:
	QString m_deviceName;
	component::Device *m_device;
	QList<component::Channel *> m_channels;
};
} // namespace swiot
} // namespace scopy

#endif // SCOPY_DIOCONTROLLER_H
