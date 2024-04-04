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

#ifndef SCOPY_DATASTRATEGYINTERFACE_H
#define SCOPY_DATASTRATEGYINTERFACE_H

#include <QObject>
#include <QLoggingCategory>
#include <QDateTime>
#include <iio.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT DataStrategyInterface
{
public:
	/**
	 * @brief Interface for manipulating data in the IIO Widget system
	 * */
	virtual ~DataStrategyInterface() = default;

	/**
	 * @brief Returns the last data read
	 * */
	virtual QString data() = 0;

	/**
	 * @brief Return the last piece of optional data or empty string is not available
	 * */
	virtual QString optionalData() = 0;

public Q_SLOTS:
	/**
	 * @brief Performs a write operation
	 * */
	virtual void save(QString data) = 0;

	/**
	 * @brief Performs a read operation and emits the data
	 * */
	virtual void requestData() = 0;

Q_SIGNALS:
	/**
	 * @brief Emits the data read
	 * */
	virtual void sendData(QString data, QString dataOptions) = 0;

	/**
	 * @brief This signal is emitted before a write operation
	 * @param oldData String containing the data what is present before the write
	 * @param newData String containing the data that will be written
	 * */
	virtual void aboutToWrite(QString oldData, QString newData) = 0;

	/**
	 * @brief emitStatus Signal for emitting the status of the operation that was
	 * just performed.
	 * @param timestamp QDateTime that holds the timestamp
	 * @param oldData The data that is present in the iio-widget before the operation
	 * @param newData The new data that was given to the operation
	 * @param returnCode int representing the return code of that operation
	 * @param isReadOp Boolean value set to true if the operation is a read
	 * operation and false if it is a write operation.
	 */
	virtual void emitStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode,
				bool isReadOp) = 0;

protected:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::DataStrategyInterface, "scopy::DataStrategyInterface")
#endif // SCOPY_DATASTRATEGYINTERFACE_H
