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

#ifndef SCOPY_EDITABLEGUISTRATEGY_H
#define SCOPY_EDITABLEGUISTRATEGY_H

#include <QWidget>
#include <iio.h>
#include <infoiconwidget.h>
#include "guistrategy/guistrategyinterface.h"
#include <gui/widgets/menulineedit.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT EditableGuiStrategy : public QObject, public GuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::GuiStrategyInterface)
public:
	/**
	 * @brief This contain a MenuLineEdit with no validation on what the text can or cannot be set. The
	 * parent of this QObject will be the parent of the ui() QWidget.
	 * */
	explicit EditableGuiStrategy(IIOWidgetFactoryRecipe recipe, bool isCompact = false, QWidget *parent = nullptr);
	~EditableGuiStrategy();

	/**
	 * @overload GuiStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

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
	MenuLineEdit *m_lineEdit;
	QString m_lastEmittedText;
	QLabel *m_title;
	InfoIconWidget *m_infoIcon;
};
} // namespace scopy

#endif // SCOPY_EDITABLEGUISTRATEGY_H
