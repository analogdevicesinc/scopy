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

#ifndef IREGISTERREADSTRATEGY_HPP
#define IREGISTERREADSTRATEGY_HPP

#include <QObject>
#include "../../scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGER_EXPORT IReadStrategy : public QObject
{
	Q_OBJECT
public:
	virtual void read() = 0;

Q_SIGNALS:
	void readDone(double time, double value);
	void readError(const char *err);
};
} // namespace datamonitor
} // namespace scopy
#endif // IREGISTERREADSTRATEGY_HPP
