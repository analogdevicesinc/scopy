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

#ifndef TESTREADSTRATEGY_HPP
#define TESTREADSTRATEGY_HPP

#include "ireadstrategy.hpp"
#include "../../scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT TestReadStrategy : public IReadStrategy
{
public:
	TestReadStrategy();

	// IReadStrategy interface
public:
	void read();
	double testDataValue = 0;
	double testDataTime = 0;
};
} // namespace datamonitor
} // namespace scopy
#endif // TESTREADSTRATEGY_HPP
