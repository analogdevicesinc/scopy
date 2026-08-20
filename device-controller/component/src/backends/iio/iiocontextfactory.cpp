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
	scopy::iio::IBackend *backend = scopy::iio::IIOBackendLoader::instance()->backend(m_version);
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
