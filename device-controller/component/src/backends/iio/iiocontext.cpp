#include "component/backends/iio/iiocontext.h"

#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"

using namespace scopy::component::iio;

IIOContext::~IIOContext()
{
	if(!m_backend) {
		return;
	}

	if(m_handle.ptr) {
		m_backend->contextOps()->destroyContext(m_handle);
	}
}
