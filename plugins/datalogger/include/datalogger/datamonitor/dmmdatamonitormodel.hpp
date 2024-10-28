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
 *
 */

#ifndef DMMDATAMONITORMODEL_HPP
#define DMMDATAMONITORMODEL_HPP

#include "datamonitor/readstrategy/dmmreadstrategy.hpp"
#include "readabledatamonitormodel.hpp"
#include "scopy-datalogger_export.h"
#include <QObject>

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGER_EXPORT DmmDataMonitorModel : public ReadableDataMonitorModel
{
	Q_OBJECT
public:
	DmmDataMonitorModel(QObject *parent = nullptr);
	explicit DmmDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
				     DMMReadStrategy *readStrategy = nullptr, QObject *parent = nullptr);

	iio_channel *iioChannel() const;
	void setIioChannel(iio_channel *newIioChannel);

	QString getDeviceName();
	iio_device *iioDevice() const;
	void setIioDevice(iio_device *newIioDevice);

private:
	iio_channel *m_iioChannel;
	iio_device *m_iioDevice;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMMDATAMONITORMODEL_HPP
