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

#include "session_info.h"

#include "config.h"

#include <QSysInfo>

using namespace adiscope;

SessionInfo::SessionInfo()
	: ApiObject()
	, m_lastConnectedFirmware{}
	, m_lastConnectedSerialNumber{}

{
	setObjectName("SessionInfo");
}

QString SessionInfo::getOperatingSystem() const { return QSysInfo::prettyProductName(); }

QString SessionInfo::getGitTag() const { return SCOPY_VERSION_GIT; }

QString SessionInfo::getLastConnectedFirmware() const { return m_lastConnectedFirmware; }

void SessionInfo::setLastConnectedFirmware(const QString& firmware) { m_lastConnectedFirmware = firmware; }

QString SessionInfo::getLastConnectedSerialNumber() const { return m_lastConnectedSerialNumber; }

void SessionInfo::setLastConnectedSerialNumber(const QString& serial) { m_lastConnectedSerialNumber = serial; }
