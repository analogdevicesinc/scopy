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

#ifndef SCOPY_GUISTRATEGYINTERFACE_H
#define SCOPY_GUISTRATEGYINTERFACE_H

#include <QWidget>
#include <QLoggingCategory>
#include <iio.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/customSwitch.h>
#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT GuiStrategyInterface
{
public:
	~GuiStrategyInterface() = default;

	/**
	 * @brief This should implement the main method for displaying the value of an attribute and also editing data
	 * from that attribute.
	 * */
	virtual QWidget *ui() = 0;

	/**
	 * @brief Checks if the class implemented by this interface has the data it requires to properly function.
	 * @return true if the data provided is enough for the class to work properly.
	 * */
	virtual bool isValid() = 0;

public Q_SLOTS:
	/**
	 * @brief Reads the data from the attributes that are set in the recipe if available.
	 * */
	virtual void receiveData(QString currentData, QString optionalData) = 0;

Q_SIGNALS:
	/**
	 * @brief This signal is emitted when the ui strategy receives new data from external sources,
	 * not from the user.
	 * @param data The data that will be displayed.
	 * @param optionalData The data options, if available. Empty string if the parameter is not
	 * necessary in this case.
	 * */
	virtual void displayedNewData(QString data, QString optionalData) = 0;

	/**
	 * @brief This will be the signal that the user changed the data, it should be caught by an external function
	 * and set accordingly.
	 * */
	virtual void emitData(QString data) = 0;

	/**
	 * @brief Sends a request to the data handler to be able to complete the display
	 * */
	virtual void requestData() = 0;

protected:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::GuiStrategyInterface, "scopy::GuiStrategyInterface")
#endif // SCOPY_GUISTRATEGYINTERFACE_H
