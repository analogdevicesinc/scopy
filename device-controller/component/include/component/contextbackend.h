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

#include <QString>
#include <QStringList>
#include <QVector>

namespace scopy::component {

class Context;
class DeviceMonitor;

// A backend is identified by a string id ("iio", "m2k", "audio", ...), so a package can add
// one at runtime without changing the device-controller.
using BackendId = QString;

struct ScanResult
{
	QString uri;
	QString description;
	BackendId backend;
};

// One unit per backend: context creation and discovery (scan + optional auto-scan monitor).
// Each concrete backend owns all backend-specific knowledge (loader, handle, builder), so the
// Controller stays backend-agnostic.
class ContextBackend
{
public:
	virtual ~ContextBackend() = default;

	virtual BackendId id() const = 0;

	// Builds a fully built, executor-backed Context. Called with the Controller's context mutex
	// held: must not call back into the Controller.
	virtual Context *create(const QString &uri, const int maxThreads) = 0;

	// Blocking scan, call it off the UI thread. filter is backend-specific (e.g. "usb", "ip:").
	// Results carry a display-ready description.
	virtual QVector<ScanResult> scan(const QString &filter = QString()) = 0;

	// Scan filters supported by this backend (e.g. "usb", "ip", "serial" for libiio).
	virtual QStringList scanFilters() const { return {}; }

	// Optional auto-scan source, owned by the backend. Lives in the thread that first calls monitor().
	virtual DeviceMonitor *monitor() { return nullptr; }
};

} // namespace scopy::component
