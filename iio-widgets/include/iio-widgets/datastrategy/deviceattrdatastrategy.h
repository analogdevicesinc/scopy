#ifndef SCOPY_DEVICEATTRDATASTRATEGY_H
#define SCOPY_DEVICEATTRDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/datastrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT DeviceAttrDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit DeviceAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_DEVICEATTRDATASTRATEGY_H
