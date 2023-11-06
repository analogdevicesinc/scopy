#ifndef SCOPY_TIMESAVESTRATEGY_H
#define SCOPY_TIMESAVESTRATEGY_H

#include "attrdata.h"
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>

namespace scopy::attr {
class TimerSaveStrategy : public SaveStrategyInterface
{
	Q_OBJECT
public:
	/**
	 * @brief This implements a saving method that only saved the data to the iio device only after the class did
	 * not receive any data for a set time. This is meant to reduce the amount of iio calls in case the UI will send
	 * a lot data in a short time and only the last one needs to become an iio call.
	 * */
	explicit TimerSaveStrategy(AttributeFactoryRecipe recipe, QObject *parent = nullptr);

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	AttributeFactoryRecipe m_recipe;
	SmallProgressBar *m_progressBar;
	QString m_lastData;
};
} // namespace scopy::attr
#endif // SCOPY_TIMESAVESTRATEGY_H
