#ifndef SCOPY_CHANNELATTRDATASTRATEGY_H
#define SCOPY_CHANNELATTRDATASTRATEGY_H

#include <QObject>
#include <iio.h>
#include "datastrategy/datastrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT ChannelAttrDataStrategy : public DataStrategyInterface
{
	Q_OBJECT
public:
	explicit ChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);

	void save(QString data) override;
	void requestData() override;
};
} // namespace scopy::attr

#endif // SCOPY_CHANNELATTRDATASTRATEGY_H
