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

#include <readwrite/fileregisterwritestrategy.hpp>

using namespace scopy;
using namespace scopy::regmap;

FileRegisterWriteStrategy::FileRegisterWriteStrategy(QString path)
	: m_file(path)
{}

QCoro::Task<Result<void>> FileRegisterWriteStrategy::write(uint32_t address, QVector<uint32_t> values)
{
	for(int i = 0; i < values.size(); i++) {
		auto r = m_file.write(address + i, values[i]);
		if(!r) {
			co_return Unexpected(r.error());
		}
	}
	co_return Result<void>();
}
