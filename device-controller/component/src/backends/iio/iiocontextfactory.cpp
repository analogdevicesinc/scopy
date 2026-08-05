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

Context *IIOContextFactory::create(const QString &uri)
{
	auto *loader = scopy::iio::IIOBackendLoader::instance();
	if(!loader->load(m_version)) {
		return nullptr;
	}

	scopy::iio::IBackend *backend = loader->backend();
	const scopy::iio::ContextHandle handle = backend->contextOps()->createContext(uri);
	if(!handle.ptr) {
		return nullptr;
	}

	auto *ctx = new IIOContext;
	ctx->setUri(uri);
	ctx->setHandle(handle);

    auto *executor = new PooledCmdExecutor(1, ctx);
	ctx->setExecutor(executor);

	IIOComponentBuilder builder(backend);
	if(!builder.build(ctx, executor)) {
		delete ctx;
		return nullptr;
	}
	return ctx;
}
