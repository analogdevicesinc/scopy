#ifndef SCOPY_TRIGGERDATASTRATEGY_H
#define SCOPY_TRIGGERDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/channelattrdatastrategy.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT TriggerDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit TriggerDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_TRIGGERDATASTRATEGY_H
