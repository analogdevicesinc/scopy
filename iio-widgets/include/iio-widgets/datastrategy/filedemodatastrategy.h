#ifndef SCOPY_FILEDEMODATASTRATEGY_H
#define SCOPY_FILEDEMODATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/channelattrdatastrategy.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT FileDemoDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit FileDemoDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_FILEDEMODATASTRATEGY_H
