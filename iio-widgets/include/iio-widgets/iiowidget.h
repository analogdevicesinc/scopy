/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SCOPY_IIOWIDGET_H
#define SCOPY_IIOWIDGET_H

#include <QWidget>
#include "iioconfigurationpopup.h"
#include "iiowidgetdata.h"
#include "utils.h"
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/smallprogressbar.h>
#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/popupwidget.h>
#include "guistrategy/guistrategyinterface.h"
#include "datastrategy/datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"
#include <pluginbase/lazyloadwidget.h>
#include <functional>

namespace scopy {
class GuiStrategyInterface;
class DataStrategyInterface;

class SCOPY_IIO_WIDGETS_EXPORT IIOWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
	QWIDGET_LAZY_INIT(initialize)
public:
	typedef enum
	{
		Busy,
		Correct,
		Error
	} State;

	IIOWidget(GuiStrategyInterface *uiStrategy, DataStrategyInterface *dataStrategy, QWidget *parent = nullptr);

	/**
	 * @brief Performs a synchronous data read.
	 * @return Returns a QPair where the first value is a QString with the data read (e.g. the sample rate)
	 * and the second value is a QString with the options (e.g. the available sample rates). If no options
	 * are available, an empty string is returned.
	 * */
	QPair<QString, QString> read();

	/**
	 * @brief Performs a synchronous data write.
	 * @param data The data to write.
	 * @return The return code from this operation. It can also be accessed from the IIOWidget::lastReturnCode()
	 * function.
	 */
	int write(QString data);

	/**
	 * @brief Starts an asynchronous read operation. The result can be accessed by connecting a slot to the
	 * DataStrategyInterface::sendData() function from the Data Strategy of this IIOWidget.
	 */
	void readAsync();

	/**
	 * @brief Starts an asynchronous write operation. The result can be accessed by connecting a slot to the
	 * DataStrategyInterface::emitStatus() function from the Data Steategy of this IIOWidget.
	 * @param data
	 */
	void writeAsync(QString data);

	/**
	 * @brief swapDataStrategy Disconnects the current Data Strategy and connects the new Data Strategy.
	 * @param dataStrategy The new Data Strategy that will be connected to the UIC.
	 * @return The old Data Strategy.
	 */
	DataStrategyInterface *swapDataStrategy(DataStrategyInterface *dataStrategy);

	/**
	 * @brief Returns the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return AttributeFactoryRecipe
	 * */
	IIOWidgetFactoryRecipe getRecipe();

	/**
	 * @brief Sets the recipe that this widget is based on. This is optional, currently serves as a way to pass
	 * information about the current widget.
	 * @return void
	 * */
	void setRecipe(IIOWidgetFactoryRecipe recipe);

	/**
	 * @brief Returns the timestamp for the last operation
	 * @return QDateTime if there exists a last operation or nullptr if no operation was performed on this IIOWidget
	 * */
	QDateTime *lastOperationTimestamp();

	/**
	 * @brief Returns the state of the last operation
	 * @return IIOWidget::State if there exists a last operation or nullptr if no operation was performed on this
	 * IIOWidget
	 * */
	IIOWidget::State *lastOperationState();

	/**
	 * @brief lastReturnCode Returns the return code from the last operation performed.
	 * @return Int value that represents the last return code. If no operation was performed, 0 will be returned.
	 */
	int lastReturnCode();

	/**
	 * @brief setUItoDataConversion Adds a function that is called before sending the data given as imput by the
	 * user and modifies this data. The function receives as parameter a QString, converts it and returns the
	 * converted string that will be sent to the DS after.
	 * @param func std::function<QString(QString)>
	 */
	void setUItoDataConversion(std::function<QString(QString)> func);

	/**
	 * @brief setDataToUIConversion Adds a function that is called before sending the data read by the IIOWidget
	 * and modifies this data. The function receives as parameter a QString, converts it and returns the
	 * converted string that will be sent to the UI after.
	 * @param func std::function<QString(QString)>
	 */
	void setDataToUIConversion(std::function<QString(QString)> func);

	/**
	 * @brief setConfigurable A configurable widget will have a wheel button on the right and it will allow the
	 * use to modify the underlying data strategy at runtime.
	 * @param isConfigurable If true, the widget is configurable at runtime. The default is false.
	 */
	void setConfigurable(bool isConfigurable);

	/**
	 * @brief setUIEnabled Enables/Disables the UI.
	 * @param isEnabled True if the UI should be enabled, false if it should be disabled.
	 */
	void setUIEnabled(bool isEnabled);

	/**
	 * @brief optionalData Calls the optionalData function from the data strategy.
	 * @return QString representing the optional data.
	 */
	QString optionalData() const;

	/**
	 * @brief data Calls the data functipm from the data strategy.
	 * @return QString represrnting the data.
	 */
	QString data() const;

	/**
	 * @brief isDSInstanceOf Checks whether the current data strategy is an instance of the specified type.
	 * @return True if the type specified coincides with the type of the data strategy.
	 */
	template <typename T>
	bool isDSInstanceOf() const
	{
		return dynamic_cast<T *>(m_dataStrategy) != nullptr;
	}

	/**
	 * @brief isUISInstanceO Checks wheter the current UI strategy is an instance of the specified type.
	 * @return True if the type specified coincides with the type of the UI strategy.
	 */
	template <typename T>
	bool isUISInstanceOf() const
	{
		return dynamic_cast<T *>(m_uiStrategy) != nullptr;
	}

Q_SIGNALS:
	/**
	 * @brief Emits the current state of the IIOWidget system and a string containing a more
	 * elaborate explanation of the current state
	 * */
	void currentStateChanged(State currentState, QString explanation = "");

	/**
	 * @brief sendData Forwards the sendData signal from the Data Strategy (emits the newly read data).
	 * @param data The data read.
	 * @param dataOptions The data from the optional attribute read.
	 */
	void sendData(QString data, QString dataOptions);

	/**
	 * @brief emitStatus Forwarded signal from the Data Strategy for emitting the status of
	 * the operation that was just performed.
	 * @param timestamp QDateTime that holds the timestamp
	 * @param oldData The data that is present in the iio-widget before the operation
	 * @param newData The new data that was given to the operation
	 * @param returnCode int representing the return code of that operation
	 * @param isReadOp Boolean value set to true if the operation is a read
	 * operation and false if it is a write operation.
	 */
	void emitStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp);

	/**
	 * @brief This signal is emitted before a write operation
	 * @param oldData String containing the data what is present before the write
	 * @param newData String containing the data that will be written
	 * */
	void aboutToWrite(QString oldData, QString newData);

	/**
	 * @brief This will be the signal that the user changed the data, it is forwarded from the UI strategy.
	 * */
	void emitData(QString data);

	/**
	 * @brief This signal is emitted when the ui strategy receives new data from external sources,
	 * not from the user.
	 * @param data The data that will be displayed.
	 * @param optionalData The data options, if available. Empty string if the parameter is not
	 * necessary in this case.
	 * */
	void displayedNewData(QString data, QString optionalData);

public Q_SLOTS:
	void changeTitle(QString title);

protected Q_SLOTS:
	void saveData(QString data);
	void emitDataStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp);

	void startTimer(QString data);
	void storeReadInfo(QString data, QString optionalData);

	void convertUItoDS(QString data);
	void convertDStoUI(QString data, QString optionalData);

protected:
	void initialize();
	void reconfigure();

	void setLastOperationTimestamp(QDateTime timestamp);
	void setLastOperationState(IIOWidget::State state);

	/* Core variables */
	GuiStrategyInterface *m_uiStrategy;
	DataStrategyInterface *m_dataStrategy;
	IIOWidgetFactoryRecipe m_recipe;

	/* Logged data */
	QString m_lastData;
	SmallProgressBar *m_progressBar;
	QDateTime *m_lastOpTimestamp;
	int m_lastReturnCode;
	IIOWidget::State *m_lastOpState;

	/* Conversion functions */
	std::function<QString(QString)> m_UItoDS;
	std::function<QString(QString)> m_DStoUI;

	/* Optional configuration */
	QPushButton *m_configBtn;
	IIOConfigurationPopup *m_configPopup;
	bool m_isConfigurable;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGET_H
