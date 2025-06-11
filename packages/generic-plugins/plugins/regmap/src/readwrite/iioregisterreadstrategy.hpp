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

#ifndef IIOREGISTERREADSTRATEGY_HPP
#define IIOREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

#include <iio.h>

#include <QObject>

namespace scopy::regmap {

class IIORegisterReadStrategy : public IRegisterReadStrategy
{
public:
	explicit IIORegisterReadStrategy(struct iio_device *dev);
	void read(uint32_t address);
	uint32_t getAddressSpace() const;
	void setAddressSpace(uint32_t newAddressSpace);

Q_SIGNALS:

private:
	struct iio_device *dev;
	uint32_t addressSpace = 0;
};
} // namespace scopy::regmap
#endif // IIOREGISTERREADSTRATEGY_HPP
