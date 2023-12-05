#ifndef SCOPY_DEVICEATTRDATASTRATEGY_H
#define SCOPY_DEVICEATTRDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/datastrategyinterface.h"
#include "attrdata.h"

namespace scopy::attr {
class DeviceAttrDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit DeviceAttrDataStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_DEVICEATTRDATASTRATEGY_H
