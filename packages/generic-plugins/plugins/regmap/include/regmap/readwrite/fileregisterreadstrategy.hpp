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

#ifndef FILEREGISTERREADSTRATEGY_HPP
#define FILEREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"
#include "registervaluesfile.hpp"

namespace scopy::regmap {
// Reads registers from a register values file, as if it were a device.
class SCOPY_REGMAP_EXPORT FileRegisterReadStrategy : public IRegisterReadStrategy
{
public:
	explicit FileRegisterReadStrategy(QString path);
	QCoro::Task<scopy::Result<QVector<uint32_t>>> read(uint32_t address, int count = 1) override;
	// every value in the file, in file order (used to write a list of values)
	scopy::Result<QVector<QPair<uint32_t, uint32_t>>> readAll() const;

private:
	RegisterValuesFile m_file;
};
} // namespace scopy::regmap
#endif // FILEREGISTERREADSTRATEGY_HPP
