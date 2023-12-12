#ifndef SCOPY_EXTERNALSAVESTRATEGY_H
#define SCOPY_EXTERNALSAVESTRATEGY_H

#include "iiowidgetdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>

#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT ExternalTriggerSaveStrategy : public SaveStrategyInterface
{
	Q_OBJECT
public:
	explicit ExternalTriggerSaveStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);
	~ExternalTriggerSaveStrategy();

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	IIOWidgetFactoryRecipe m_recipe;
	QPushButton *m_ui;
	QString m_data;
	bool m_dataReceived;
};
} // namespace scopy::attr

#endif // SCOPY_EXTERNALSAVESTRATEGY_H
