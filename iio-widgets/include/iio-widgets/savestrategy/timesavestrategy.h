#ifndef SCOPY_TIMESAVESTRATEGY_H
#define SCOPY_TIMESAVESTRATEGY_H

#include "iiowidgetdata.h"
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT TimerSaveStrategy : public QWidget, public SaveStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::SaveStrategyInterface)
public:
	/**
	 * @brief This implements a saving method that only saved the data to the iio device only after the class did
	 * not receive any data for a set time. This is meant to reduce the amount of iio calls in case the UI will send
	 * a lot data in a short time and only the last one needs to become an iio call.
	 * */
	explicit TimerSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

Q_SIGNALS:
	void saveData(QString data);

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	IIOWidgetFactoryRecipe m_recipe;
	SmallProgressBar *m_progressBar;
	QString m_lastData;
};
} // namespace scopy
#endif // SCOPY_TIMESAVESTRATEGY_H
