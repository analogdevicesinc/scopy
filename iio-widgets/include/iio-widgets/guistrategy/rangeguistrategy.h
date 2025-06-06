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
// #include <gui/widgets/titlespinbox.h>
#include <gui/widgets/menuspinbox.h>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT RangeAttrUi : public QObject, public GuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::GuiStrategyInterface)
public:
	/**
	 * @brief This contain a PositionSpinButton that takes the 3 values from recipe->linkedAttributeValue. The
	 * string from recipe->linkedAttributeValue should look like "[begin step end]" where "begin", "step" and "end"
	 * will be converted to double.
	 * */
	explicit RangeAttrUi(IIOWidgetFactoryRecipe recipe, bool isCompact = false, QWidget *parent = nullptr);
	~RangeAttrUi();

	/**
	 * @overload GuiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() final;

	void setCustomTitle(QString title) override;

	void setInfoMessage(QString infoMessage) override;

public Q_SLOTS:
	void receiveData(QString currentData, QString optionalData) override;

Q_SIGNALS:
	void displayedNewData(QString data, QString optionalData) override;
	void emitData(QString data) override;
	void requestData() override;

private:
	/**
	 * @brief tryParse will try an parse a QString to a double and if it fails it will try
	 * and parse it io an int and the cast it back to a double.
	 * @param number A QString that represents a double or an int.
	 * @param success This will be set to false if the QString parse fails and true if the
	 * number is parsed successfully.
	 * @return The double that was extracted from the QString.
	 */
	double tryParse(QString number, bool *success);

	QWidget *m_ui;
	gui::MenuSpinbox *m_spinBox;
};
} // namespace scopy

#endif // SCOPY_RANGEGUISTRATEGY_H
