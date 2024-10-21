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

#include "iioregisterreadstrategy.hpp"

#include "../logging_categories.h"
#include "iregisterreadstrategy.hpp"

using namespace scopy;
using namespace regmap;

IIORegisterReadStrategy::IIORegisterReadStrategy(struct iio_device *dev)
	: dev(dev)
{}

void IIORegisterReadStrategy::read(uint32_t address)
{
	uint32_t auxAddress = address | addressSpace;
	uint32_t reg_val;

	ssize_t read = iio_device_reg_read(dev, auxAddress, &reg_val);
	if(read < 0) {
		char err[1024];
		iio_strerror(-(int)read, err, sizeof(err));
		qDebug(CAT_IIO_OPERATION) << "device read error " << err;
		Q_EMIT readError("device read error");
	} else {
		qDebug(CAT_IIO_OPERATION)
			<< "device read success for register " << address << " with value " << reg_val;
		Q_EMIT readDone(address, reg_val);
	}
}

uint32_t IIORegisterReadStrategy::getAddressSpace() const { return addressSpace; }

void IIORegisterReadStrategy::setAddressSpace(uint32_t newAddressSpace) { addressSpace = newAddressSpace; }
