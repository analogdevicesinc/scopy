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

#include "component/backends/iio/iioprovider.h"

#include "component/backends/iio/iioconnection.h"
#include "component/devicemonitor.h"
#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"
#include "iioutil/iscanops.h"

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

namespace {
// libiio reports "name (hw_model ...), serial=..."; keep the part users recognise.
QString parseDescription(const QString &d)
{
	int startPos = d.indexOf('(');
	if(startPos == -1) {
		return d;
	}
	int endPos = d.indexOf(")), serial=");
	if(endPos == -1) {
		endPos = d.lastIndexOf(')');
	} else {
		startPos += 1;
	}
	return d.mid(startPos, endPos - startPos + 1);
}
} // namespace

// Implemented by the libiio backend linked into `component` (iio-backend-v0 or iio-backend-v1,
// selected at build time by SCOPY_LIBIIO_VERSION). Resolved at link time — no dlopen.
extern "C" scopy::iio::IBackend *createIIOBackend();

IIOProvider::IIOProvider() = default;

IIOProvider::~IIOProvider() = default;

scopy::iio::IBackend *IIOProvider::defaultBackend()
{
	static scopy::iio::IBackend *s_backend = createIIOBackend();
	return s_backend;
}

Connection *IIOProvider::open(const QString &uri)
{
	scopy::iio::IBackend *backend = defaultBackend();
	if(!backend) {
		return nullptr;
	}
	const scopy::iio::ContextHandle handle = backend->contextOps()->createContext(uri);
	if(!handle.ptr) {
		return nullptr;
	}
	return new IIOConnection(uri, backend, handle);
}

QVector<ScanResult> IIOProvider::scan(const QString &filter)
{
	QVector<ScanResult> results;
	scopy::iio::IBackend *backend = defaultBackend();
	if(!backend) {
		return results;
	}
	const QVector<scopy::iio::ScanResult> found = backend->scanOps()->scan(filter);
	for(const scopy::iio::ScanResult &r : found) {
		results.append({r.uri, parseDescription(r.description), KIND});
	}
	return results;
}

QStringList IIOProvider::scanFilters() const
{
	scopy::iio::IBackend *backend = defaultBackend();
	return backend ? backend->scanOps()->availableBackends() : QStringList();
}

DeviceMonitor *IIOProvider::monitor()
{
	if(!m_monitor) {
		m_monitor = std::make_unique<PollingDeviceMonitor>(this, QStringLiteral("usb"));
	}
	return m_monitor.get();
}
