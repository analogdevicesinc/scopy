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
#include "iioutil/iiobackendloader.h"

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

#ifdef SCOPY_DC_DIRECT_LIBIIO_V0
// Direct-link path (Scopy integration): the libiio v0 backend is statically linked in,
// so we call its factory symbol directly and never touch IIOBackendLoader/dlopen. The
// backend is kept resident for the whole process, matching the loader's old semantics.
extern "C" scopy::iio::IBackend *createIIOBackend();
namespace {
scopy::iio::IBackend *directBackend()
{
	static scopy::iio::IBackend *s_backend = createIIOBackend();
	return s_backend;
}
} // namespace
#endif

Context *IIOContextFactory::create(const QString &uri)
{
#ifdef SCOPY_DC_DIRECT_LIBIIO_V0
	Q_UNUSED(m_version) // version is fixed to v0 at build time in direct-link mode
	scopy::iio::IBackend *backend = directBackend();
#else
	scopy::iio::IBackend *backend = scopy::iio::IIOBackendLoader::instance()->backend(m_version);
#endif
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

	auto *executor = new PooledCmdExecutor(1, ctx);
	ctx->setExecutor(executor);

	IIOComponentBuilder builder(backend);
	if(!builder.build(ctx, executor)) {
		delete ctx;
		return nullptr;
	}
	return ctx;
}
