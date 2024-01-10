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
#include "guistrategy/guistrategyinterface.h"
#include "savestrategy/savestrategyinterface.h"
#include "datastrategy/datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class AttrUiStrategyInterface;
class SaveStrategyInterface;
class DataStrategyInterface;

class SCOPY_IIO_WIDGETS_EXPORT IIOWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	IIOWidget(AttrUiStrategyInterface *uiStrategy, SaveStrategyInterface *saveStrategy,
		  DataStrategyInterface *dataStrategy, QWidget *parent = nullptr);

	SaveStrategyInterface *getSaveStrategy();
	AttrUiStrategyInterface *getUiStrategy();
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

	typedef enum
	{
		Busy,
		Correct,
		Error
	} State;

Q_SIGNALS:
	/**
	 * @brief 0 - busy, 1 - correct, 2 error
	 * */
	void currentStateChanged(State currentState, QString explanation = "");

protected Q_SLOTS:
	void saveData(QString data);
	void emitDataStatus(int status);

protected:
	SaveStrategyInterface *m_saveStrategy;
	AttrUiStrategyInterface *m_uiStrategy;
	DataStrategyInterface *m_dataStrategy;
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGET_H
