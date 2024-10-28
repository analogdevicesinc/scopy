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

#include "regreadwrite.hpp"

#include "logging_categories.h"

#include <QLoggingCategory>
#include <qdebug.h>

using namespace scopy;
using namespace regmap;

RegReadWrite::RegReadWrite(struct iio_device *dev, QObject *parent)
	: dev(dev)
	, QObject{parent}
{}

RegReadWrite::~RegReadWrite() {}

void RegReadWrite::read(uint32_t address)
{
	uint32_t reg_val;

	ssize_t read = iio_device_reg_read(dev, address, &reg_val);
	if(read < 0) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << read;
		Q_EMIT readError("device read error");
	} else {
		qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value " << reg_val;
		Q_EMIT readDone(address, reg_val);
	}
}

void RegReadWrite::write(uint32_t address, uint32_t val)
{
	ssize_t write = iio_device_reg_write(dev, address, val);
	if(write < 0) {
		qDebug(CAT_IIO_OPERATION) << "device write error " << write;
		Q_EMIT writeError("device write err");
	} else {
		qDebug(CAT_IIO_OPERATION)
			<< "device write successfull for register " << address << " with value " << val;
		Q_EMIT writeSuccess(address);
	}
}
