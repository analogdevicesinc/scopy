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

#ifndef SCOPY_EXTERNALSAVESTRATEGY_H
#define SCOPY_EXTERNALSAVESTRATEGY_H

#include "iiowidgetdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>

#include "gui/widgets/smallprogressbar.h"
#include "savestrategy/savestrategyinterface.h"
#include <iio.h>
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT ExternalSaveStrategy : public QWidget, public SaveStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::SaveStrategyInterface)
public:
	explicit ExternalSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	~ExternalSaveStrategy();

	/**
	 * @overload SaveStrategyInterface::ui()
	 * */
	QWidget *ui() override;

	bool isValid() override;

Q_SIGNALS:
	void receivedData();
	void saveData(QString data);

public Q_SLOTS:
	void receiveData(QString data) override;
	void writeData();

private:
	IIOWidgetFactoryRecipe m_recipe;
	QPushButton *m_ui;
	QString m_data;
	bool m_dataReceived;
};
} // namespace scopy

#endif // SCOPY_EXTERNALSAVESTRATEGY_H
