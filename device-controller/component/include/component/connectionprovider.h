/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include "component/connection.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace scopy::component {

class DeviceMonitor;

struct ScanResult
{
	QString uri;
	QString description;
	ConnectionKind kind;
};

// Transport: opens the physical link and discovers devices. One per connection kind ("iio",
// "audio", ...); libiio is built in, others can be registered at runtime by packages.
class ConnectionProvider
{
public:
	virtual ~ConnectionProvider() = default;

	virtual ConnectionKind kind() const = 0;

	// Opens the link. No executor yet: the Controller sets it after deciding the compatible
	// backends, then calls Connection::start(). Called with the Controller's context mutex held:
	// must not call back into the Controller.
	virtual Connection *open(const QString &uri) = 0;

	// Blocking scan, call it off the UI thread. filter is provider-specific (e.g. "usb", "ip:").
	// A provider reaching devices over a serial port lists the ports itself and recognises its
	// devices without opening them.
	virtual QVector<ScanResult> scan(const QString &filter = QString()) = 0;

	virtual QString uriHint() const { return {}; }
	virtual QStringList scanFilters() const { return {}; }

	// Optional auto-scan source, owned by the provider. Lives in the thread that first calls monitor().
	virtual DeviceMonitor *monitor() { return nullptr; }
};

} // namespace scopy::component
