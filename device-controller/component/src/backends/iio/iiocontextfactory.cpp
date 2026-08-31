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
