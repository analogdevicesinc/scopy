#pragma once

#include "component/context.h"
#include "iioutil/handles.h"

namespace scopy::iio {
class IBackend;
}

namespace scopy::component::iio {

// IIO identity root. Carries the libiio context handle on top of the shared
// Context identity (uri/name/description/executor).
class IIOContext : public Context
{
	Q_OBJECT
public:
	explicit IIOContext(QObject *parent = nullptr)
		: Context(parent)
	{
	}

	~IIOContext() override;

	scopy::iio::ContextHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::ContextHandle handle) { m_handle = handle; }

	// The backend this context was built with. The loader keeps every backend
	// resident for the whole process, so this is a non-owning pointer.
	scopy::iio::IBackend *backend() const { return m_backend; }
	void setBackend(scopy::iio::IBackend *backend) { m_backend = backend; }

private:
	scopy::iio::ContextHandle m_handle;
	scopy::iio::IBackend *m_backend = nullptr;
};

} // namespace scopy::component::iio
