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
#include "iiowidgetdata.h"
#include "utils.h"
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/smallprogressbar.h>
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
	 * @brief Returns the UI of the IIOWidget
	 * @return GuiStrategyInterface
	 * */
	GuiStrategyInterface *getUiStrategy();

	/**
	 * @brief Returns the data save/load strategy
	 * @return DataStretegyInterface
	 * */
	DataStrategyInterface *getDataStrategy();

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

	void setUItoDataConversion(std::function<QString(QString)> func);
	void setDataToUIConversion(std::function<QString(QString)> func);

	void showProgressBar(bool show);

Q_SIGNALS:
	void progressBarVisible(bool);

	/**
	 * @brief Emits the current state of the IIOWidget system and a string containing a more
	 * elaborate explanation of the current state
	 * */
	void currentStateChanged(State currentState, QString explanation = "");

protected Q_SLOTS:
	void saveData(QString data);
	void emitDataStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp);

	void startTimer(QString data);
	void storeReadInfo(QString data, QString optionalData);

	void convertUItoDS(QString data);
	void convertDStoUI(QString data, QString optionalData);

protected:
	void initialize();

	void setLastOperationTimestamp(QDateTime timestamp);
	void setLastOperationState(IIOWidget::State state);

	GuiStrategyInterface *m_uiStrategy;
	DataStrategyInterface *m_dataStrategy;
	IIOWidgetFactoryRecipe m_recipe;

	QString m_lastData;
	SmallProgressBar *m_progressBar;
	QDateTime *m_lastOpTimestamp;
	int m_lastReturnCode;
	IIOWidget::State *m_lastOpState;

	/* Conversion functions */
	std::function<QString(QString)> m_UItoDS;
	std::function<QString(QString)> m_DStoUI;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGET_H
