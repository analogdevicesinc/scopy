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

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT ExternalSaveStrategy : public QWidget, public SaveStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::SaveStrategyInterface)
public:
	explicit ExternalSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~ExternalSaveStrategy();

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

Q_SIGNALS:
	void receivedData();
	void saveData(QString data);

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	IIOWidgetFactoryRecipe m_recipe;
	QPushButton *m_ui;
	QString m_data;
	bool m_dataReceived;
};
} // namespace scopy

#endif // SCOPY_EXTERNALSAVESTRATEGY_H
