#ifndef SCOPY_FILEDEMODATASTRATEGY_H
#define SCOPY_FILEDEMODATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/attrdatastrategy.h"
#include "attrdata.h"

namespace scopy::attr {
class FileDemoDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit FileDemoDataStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_FILEDEMODATASTRATEGY_H
