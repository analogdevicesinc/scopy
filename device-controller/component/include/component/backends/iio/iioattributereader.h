#pragma once

#include "component/attributereader.h"
#include "iioutil/handles.h"

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IAttrOps;
class AttrReadCommand;
}
} // namespace scopy

namespace scopy::component::iio {

class IIOAttributeReader : public AttributeReader
{
	Q_OBJECT
public:
	IIOAttributeReader(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle, scopy::ICmdExecutor *executor,
		QObject *parent = nullptr);

	Result<QByteArray> read() override;
	QUuid readAsync() override;

private:
	QCoro::Task<Result<QByteArray>> readInternal(scopy::iio::AttrReadCommand *cmd);

	scopy::iio::IAttrOps *m_ops;
	scopy::iio::AttrHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
