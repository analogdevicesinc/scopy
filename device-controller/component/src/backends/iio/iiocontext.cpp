#include "component/backends/iio/iiocontext.h"

#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"
#include "iioutil/iiobackendloader.h"

using namespace scopy::component::iio;

IIOContext::~IIOContext()
{
	if(!m_handle.ptr) {
		return;
	}
	auto *loader = scopy::iio::IIOBackendLoader::instance();
	if(loader->isLoaded()) {
		loader->backend()->contextOps()->destroyContext(m_handle);
	}
}
