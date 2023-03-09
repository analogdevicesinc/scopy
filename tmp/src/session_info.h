/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SESSIONINFO_H
#define SESSIONINFO_H

#include "apiObject.hpp"

#include <QObject>
#include <QString>

namespace adiscope {
class SessionInfo : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString os READ getOperatingSystem WRITE setOperatingSystem)
	Q_PROPERTY(QString gitTag READ getGitTag WRITE setGitTag)
	Q_PROPERTY(QString firmware READ getLastConnectedFirmware WRITE setLastConnectedFirmware)
	Q_PROPERTY(QString serialNumber READ getLastConnectedSerialNumber WRITE setLastConnectedSerialNumber)
public:
	explicit SessionInfo();

	// write functions had to be written for all the properties
	// otherwise none of them would've been saved
	QString getOperatingSystem() const;
	void setOperatingSystem(const QString& /*os*/) {}

	QString getGitTag() const;
	void setGitTag(const QString& /*tag*/) {}

	QString getLastConnectedFirmware() const;
	void setLastConnectedFirmware(const QString& firmware);

	QString getLastConnectedSerialNumber() const;
	void setLastConnectedSerialNumber(const QString& serial);

private:
	QString m_lastConnectedFirmware;
	QString m_lastConnectedSerialNumber;
};
} // namespace adiscope
#endif // SESSIONINFO_H
