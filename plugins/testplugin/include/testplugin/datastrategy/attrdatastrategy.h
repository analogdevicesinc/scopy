#ifndef SCOPY_ATTRDATASTRATEGY_H
#define SCOPY_ATTRDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/datastrategyinterface.h"
#include "attrdata.h"

namespace scopy::attr {
class AttrDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit AttrDataStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_ATTRDATASTRATEGY_H
