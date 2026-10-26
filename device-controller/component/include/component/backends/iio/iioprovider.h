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

#include "component/connectionprovider.h"

#include "component/component_export.h"

#include <memory>

namespace scopy::iio {
class IBackend;
}

namespace scopy::component::iio {

// The libiio transport: opens iio_context handles with the libiio backend linked into this build
// (SCOPY_LIBIIO_VERSION selects v0 or v1 at compile time) and scans through IScanOps; its monitor
// polls "usb".
class COMPONENT_EXPORT IIOProvider : public ConnectionProvider
{
public:
	static constexpr const char *KIND = "iio";

	IIOProvider();
	~IIOProvider() override;

	ConnectionKind kind() const override { return KIND; }
	Connection *open(const QString &uri) override;
	QVector<ScanResult> scan(const QString &filter = QString()) override;
	QStringList scanFilters() const override;
	QString uriHint() const override { return "ip:192.168.2.1, usb:1.2.3, serial:/dev/ttyUSB0,115200,8n1"; }
	DeviceMonitor *monitor() override;

	// The process-wide libiio backend linked into this build. Never null, resident for the whole
	// process.
	static scopy::iio::IBackend *defaultBackend();

private:
	std::unique_ptr<DeviceMonitor> m_monitor;
};

} // namespace scopy::component::iio
