#pragma once

#include "component/attributewriter.h"
#include "iioutil/handles.h"

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IAttrOps;
class AttrWriteCommand;
}
} // namespace scopy

namespace scopy::component::iio {

class IIOAttributeWriter : public AttributeWriter
{
	Q_OBJECT
public:
	IIOAttributeWriter(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle, scopy::ICmdExecutor *executor,
		QObject *parent = nullptr);

	Result<void> write(const QString &value) override;
	QUuid writeAsync(const QString &value) override;

private:
	QCoro::Task<Result<void>> writeInternal(scopy::iio::AttrWriteCommand *cmd);

	scopy::iio::IAttrOps *m_ops;
	scopy::iio::AttrHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
