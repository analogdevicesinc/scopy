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

#ifndef READABLEDATAMONITORMODEL_HPP
#define READABLEDATAMONITORMODEL_HPP

#include <QObject>
#include "../scopy-datalogger_export.h"
#include "readstrategy/ireadstrategy.hpp"
#include "datamonitormodel.hpp"

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGER_EXPORT ReadableDataMonitorModel : public DataMonitorModel
{
	Q_OBJECT
public:
	ReadableDataMonitorModel(QObject *parent = nullptr);
	explicit ReadableDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
					  IReadStrategy *readStrategy = nullptr, QObject *parent = nullptr);

	void read();
	IReadStrategy *readStrategy() const;
	void setReadStrategy(IReadStrategy *newReadStrategy);
	void resetMinMax();

signals:

private:
	IReadStrategy *m_readStrategy;
};
} // namespace datamonitor
} // namespace scopy
#endif // READABLEDATAMONITORMODEL_HPP
