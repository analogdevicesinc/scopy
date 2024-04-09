#ifndef SCOPY_CONTEXTATTRDATASTRATEGY_H
#define SCOPY_CONTEXTATTRDATASTRATEGY_H

#include <QWidget>
#include <iio.h>
#include "datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT ContextAttrDataStrategy : public QWidget, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit ContextAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);

	QString data() override;
	QString optionalData() override;

public Q_SLOTS:
	void save(QString data) override;
	void requestData() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions) override;
	void aboutToWrite(QString oldData, QString newData) override;
	void emitStatus(int status) override;

private:
	QString m_data;
	QString m_optionalData;
};
} // namespace scopy

#endif // SCOPY_CONTEXTATTRDATASTRATEGY_H
