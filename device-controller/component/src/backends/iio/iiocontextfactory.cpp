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

#include "component/backends/iio/iiocontextfactory.h"

#include "component/backends/iio/iiocomponentbuilder.h"
#include "component/backends/iio/iiocontext.h"

#include "core/pooledcmdexecutor.h"
#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

// Implemented by the libiio backend linked into `component` (iio-backend-v0 or iio-backend-v1,
// selected at build time by SCOPY_LIBIIO_VERSION). Resolved at link time — no dlopen.
extern "C" scopy::iio::IBackend *createIIOBackend();

scopy::iio::IBackend *IIOContextFactory::defaultBackend()
{
	// The backend is stateless and kept resident for the whole process.
	static scopy::iio::IBackend *s_backend = createIIOBackend();
	return s_backend;
}

Context *IIOContextFactory::create(const QString &uri, const int maxThreads)
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
