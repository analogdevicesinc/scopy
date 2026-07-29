#pragma once

#include "component/attribute.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// IIO attribute identity — carries the AttrHandle, no I/O logic. I/O lives on the
// IIOAttributeReader / IIOAttributeWriter capability children.
class IIOAttribute : public Attribute
{
	Q_OBJECT
public:
	explicit IIOAttribute(QObject *parent = nullptr)
		: Attribute(parent)
	{
	}

	scopy::iio::AttrHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::AttrHandle handle) { m_handle = handle; }

private:
	scopy::iio::AttrHandle m_handle;
};

} // namespace scopy::component::iio
