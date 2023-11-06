#ifndef SCOPY_SAVESTRATEGYINTERFACE_H
#define SCOPY_SAVESTRATEGYINTERFACE_H

#include "attrdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLoggingCategory>

#include "gui/widgets/smallprogressbar.h"
#include <iio.h>

namespace scopy::attr {
class SaveStrategyInterface : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief This should contain any extra widgets needed to save or manipulate the data received from the main UI.
	 * In case there is no extra UI needed, return nullptr.
	 * */
	virtual QWidget *ui() = 0;

	/**
	 * @brief Checks if the class implemented by this interface has the data it requires to properly function.
	 * @return true if the data provided is enough for the class to work properly.
	 * */
	virtual bool isValid() = 0;

public Q_SLOTS:
	/**
	 * @brief This function should save/manipulate the data received from the ui.
	 * */
	virtual void receiveData(QString data) = 0;

Q_SIGNALS:
	void saveData(QString data);
};
} // namespace scopy::attr

#endif // SCOPY_SAVESTRATEGYINTERFACE_H
