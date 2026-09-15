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

#ifndef DMMREADSTRATEGY_HPP
#define DMMREADSTRATEGY_HPP

#include "ireadstrategy.hpp"
#include "scopy-datalogger_export.h"

#include <optional>
#include <qcoro/qcorotask.h>
#include <component/attribute.h>
#include <component/attributereader.h>

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DMMReadStrategy : public IReadStrategy
{
public:
	// readAttr is the "raw" (DMM) or "input" (hwmon) attribute the value is read from.
	explicit DMMReadStrategy(component::Attribute *readAttr);

	void setUmScale(double scale);

	// IReadStrategy interface
public:
	void read();

private:
	QCoro::Task<void> readTask();

	component::Attribute *m_readAttr;
	std::optional<QCoro::Task<void>> m_task;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMMREADSTRATEGY_HPP
