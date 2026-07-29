#pragma once

#include "component/context.h"
#include "iioutil/handles.h"

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

private:
	scopy::iio::ContextHandle m_handle;
};

} // namespace scopy::component::iio
