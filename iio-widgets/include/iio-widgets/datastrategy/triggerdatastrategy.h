#ifndef SCOPY_TRIGGERDATASTRATEGY_H
#define SCOPY_TRIGGERDATASTRATEGY_H

#include <QWidget>
#include <iio.h>
#include "datastrategy/channelattrdatastrategy.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT TriggerDataStrategy : public QWidget, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit TriggerDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);

public Q_SLOTS:
	void save(QString data) override;
	void requestData() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions);
	void emitStatus(int status);
};
} // namespace scopy

#endif // SCOPY_TRIGGERDATASTRATEGY_H
