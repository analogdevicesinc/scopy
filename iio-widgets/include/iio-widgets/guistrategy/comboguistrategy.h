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

#ifndef SCOPY_COMBOGUISTRATEGY_H
#define SCOPY_COMBOGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <infoiconwidget.h>
#include <gui/widgets/menucombo.h>
#include "guistrategy/guistrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT ComboAttrUi : public QObject, public GuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::GuiStrategyInterface)
public:
	/**
	 * @brief This contain a MenuComboWidget that takes the options for the combo from recipe->linkedAttributeValue.
	 * */
	explicit ComboAttrUi(IIOWidgetFactoryRecipe recipe, bool isCompact = false, QWidget *parent = nullptr);
	~ComboAttrUi();

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
	QWidget *m_ui;
	QComboBox *m_comboWidget;
	bool m_isCompact;
	QLabel *m_title;
	InfoIconWidget *m_infoIcon;
	MenuCombo *m_comboMenuWidget;
};
} // namespace scopy

#endif // SCOPY_COMBOGUISTRATEGY_H
