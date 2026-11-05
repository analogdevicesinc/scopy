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

#ifndef M2KCOMMON_H
#define M2KCOMMON_H
#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(CAT_M2KPLUGIN);

// The URI this plugin's own libiio v0 connections (libm2k, identify, temperature) use. Scopy's
// device controller (libiio v1) already holds the board's USB interface, and a second USB
// connection makes libiio v0 crash in usb_create_context(). Over USB an M2K is also reachable
// through its USB network link, so those connections go there instead.
inline QByteArray m2kV0Uri(const QString &uri)
{
	return (uri.startsWith("usb:") ? QStringLiteral("ip:192.168.2.1") : uri).toUtf8();
}

#endif // M2KCOMMON_H
