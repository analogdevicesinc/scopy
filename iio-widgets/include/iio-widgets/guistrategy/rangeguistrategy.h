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

#ifndef SCOPY_RANGEGUISTRATEGY_H
#define SCOPY_RANGEGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <gui/spinbox_a.hpp>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT RangeAttrUi : public QWidget, public AttrUiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::AttrUiStrategyInterface)
public:
	/**
	 * @brief This contain a PositionSpinButton that takes the 3 values from recipe->linkedAttributeValue. The
	 * string from recipe->linkedAttributeValue should look like "[begin step end]" where "begin", "step" and "end"
	 * will be converted to double.
	 * */
	explicit RangeAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~RangeAttrUi();

	/**
	 * @overload AttrUiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

public Q_SLOTS:
	void receiveData(QString currentData, QString optionalData) override;

Q_SIGNALS:
	void emitData(QString data);
	void requestData();

private:
	QWidget *m_ui;
	PositionSpinButton *m_positionSpinButton;
};
} // namespace scopy

#endif // SCOPY_RANGEGUISTRATEGY_H
