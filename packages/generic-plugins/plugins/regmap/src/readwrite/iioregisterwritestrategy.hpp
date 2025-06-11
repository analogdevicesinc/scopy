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

#ifndef IIOREGISTERWRITESTRATEGY_HPP
#define IIOREGISTERWRITESTRATEGY_HPP

#include "iregisterwritestrategy.hpp"

#include <iio.h>

namespace scopy::regmap {
class IIORegisterWriteStrategy : public IRegisterWriteStrategy
{
public:
	IIORegisterWriteStrategy(struct iio_device *dev);

private:
	struct iio_device *dev;
	uint32_t addressSpace = 0;

	// IRegisterWriteStrategy interface
public:
	void write(uint32_t address, uint32_t val);
	uint32_t getAddressSpace() const;
	void setAddressSpace(uint32_t newAddressSpace);
};
} // namespace scopy::regmap
#endif // IIOREGISTERWRITESTRATEGY_HPP
