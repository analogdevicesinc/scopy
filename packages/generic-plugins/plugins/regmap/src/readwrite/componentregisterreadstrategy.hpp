/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef COMPONENTREGISTERREADSTRATEGY_HPP
#define COMPONENTREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

namespace scopy::component::iio {
class IIORegisterReader;
}

namespace scopy::regmap {

// Register read strategy backed by a device-controller IIORegisterReader.
class ComponentRegisterReadStrategy : public IRegisterReadStrategy
{
public:
	explicit ComponentRegisterReadStrategy(scopy::component::iio::IIORegisterReader *reader);
	void read(uint32_t address) override;
	uint32_t getAddressSpace() const;
	void setAddressSpace(uint32_t newAddressSpace);

private:
	scopy::component::iio::IIORegisterReader *m_reader;
	uint32_t m_addressSpace = 0;
};
} // namespace scopy::regmap
#endif // COMPONENTREGISTERREADSTRATEGY_HPP
