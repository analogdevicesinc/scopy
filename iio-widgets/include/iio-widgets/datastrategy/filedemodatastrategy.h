#ifndef SCOPY_FILEDEMODATASTRATEGY_H
#define SCOPY_FILEDEMODATASTRATEGY_H

#include <QWidget>
#include <iio.h>
#include "datastrategy/channelattrdatastrategy.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT FileDemoDataStrategy : public QWidget, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit FileDemoDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);

public Q_SLOTS:
	void save(QString data) override;
	void requestData() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions);
	void emitStatus(int status);
};
} // namespace scopy

#endif // SCOPY_FILEDEMODATASTRATEGY_H
