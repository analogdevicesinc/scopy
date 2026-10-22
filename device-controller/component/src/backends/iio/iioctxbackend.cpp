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

#include "component/backends/iio/iioctxbackend.h"

#include "component/backends/iio/iiocomponentbuilder.h"
#include "component/backends/iio/iiocontext.h"
#include "component/devicemonitor.h"

#include "core/pooledcmdexecutor.h"
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

IIOCtxBackend::IIOCtxBackend() = default;

IIOCtxBackend::~IIOCtxBackend() = default;

scopy::iio::IBackend *IIOCtxBackend::defaultBackend()
{
	// The backend is stateless and kept resident for the whole process.
	static scopy::iio::IBackend *s_backend = createIIOBackend();
	return s_backend;
}

Context *IIOCtxBackend::create(const QString &uri, const int maxThreads)
{
	scopy::iio::IBackend *backend = defaultBackend();
	if(!backend) {
		return nullptr;
	}

	const scopy::iio::ContextHandle handle = backend->contextOps()->createContext(uri);
	if(!handle.ptr) {
		return nullptr;
	}

	auto *ctx = new IIOContext;
	ctx->setUri(uri);
	ctx->setHandle(handle);
	ctx->setBackend(backend);

	auto *executor = new PooledCmdExecutor(maxThreads, ctx);
	ctx->setExecutor(executor);

	IIOComponentBuilder builder(backend);
	if(!builder.build(ctx, executor)) {
		delete ctx;
		return nullptr;
	}
	return ctx;
}

QVector<ScanResult> IIOCtxBackend::scan(const QString &filter)
{
	QVector<ScanResult> results;
	scopy::iio::IBackend *backend = defaultBackend();
	if(!backend) {
		return results;
	}
	const QVector<scopy::iio::ScanResult> found = backend->scanOps()->scan(filter);
	for(const scopy::iio::ScanResult &r : found) {
		results.append({r.uri, parseDescription(r.description), ID});
	}
	return results;
}

QStringList IIOCtxBackend::scanFilters() const
{
	scopy::iio::IBackend *backend = defaultBackend();
	return backend ? backend->scanOps()->availableBackends() : QStringList();
}

DeviceMonitor *IIOCtxBackend::monitor()
{
	if(!m_monitor) {
		m_monitor = std::make_unique<PollingDeviceMonitor>(this, QStringLiteral("usb"));
	}
	return m_monitor.get();
}
