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

#include "iioregisterwritestrategy.hpp"

#include "../logging_categories.h"

using namespace scopy::regmap;

IIORegisterWriteStrategy::IIORegisterWriteStrategy(struct iio_device *dev)
	: dev(dev)
{}

void IIORegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	uint32_t auxAddress = address | addressSpace;
	ssize_t write = iio_device_reg_write(dev, auxAddress, val);
	if(write < 0) {
		char err[1024];
		iio_strerror(-(int)write, err, sizeof(err));
		qDebug(CAT_IIO_OPERATION) << "device write error " << err;
		Q_EMIT writeError("device write err");
	} else {
		qDebug(CAT_IIO_OPERATION)
			<< "device write successfull for register " << address << " with value " << val;
		Q_EMIT writeSuccess(address);
	}
}

uint32_t IIORegisterWriteStrategy::getAddressSpace() const { return addressSpace; }

void IIORegisterWriteStrategy::setAddressSpace(uint32_t newAddressSpace) { addressSpace = newAddressSpace; }
