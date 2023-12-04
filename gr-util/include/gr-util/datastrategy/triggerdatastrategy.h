#ifndef SCOPY_TRIGGERDATASTRATEGY_H
#define SCOPY_TRIGGERDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/attrdatastrategy.h"
#include "attrdata.h"

namespace scopy::attr {
class TriggerDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit TriggerDataStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_TRIGGERDATASTRATEGY_H
