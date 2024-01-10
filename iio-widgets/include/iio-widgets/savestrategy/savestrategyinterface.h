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

#ifndef SCOPY_SAVESTRATEGYINTERFACE_H
#define SCOPY_SAVESTRATEGYINTERFACE_H

#include "iiowidgetdata.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLoggingCategory>

#include "gui/widgets/smallprogressbar.h"
#include <iio.h>
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT SaveStrategyInterface
{
public:
	~SaveStrategyInterface() = default;

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
	virtual void saveData(QString data) = 0;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::SaveStrategyInterface, "scopy::SaveStrategyInterface")
#endif // SCOPY_SAVESTRATEGYINTERFACE_H
