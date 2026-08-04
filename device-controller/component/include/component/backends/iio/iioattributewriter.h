#pragma once

#include "component/attributewriter.h"
#include "iioutil/handles.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IAttrOps;
class AttrWriteCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOAttributeWriter : public AttributeWriter
{
	Q_OBJECT
public:
	IIOAttributeWriter(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle, scopy::ICmdExecutor *executor,
               QObject *parent = nullptr);

    QCoro::Task<CommandResponse<void>> writeAsync(const QString &value) override;

private:
	scopy::iio::IAttrOps *m_ops;
	scopy::iio::AttrHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
