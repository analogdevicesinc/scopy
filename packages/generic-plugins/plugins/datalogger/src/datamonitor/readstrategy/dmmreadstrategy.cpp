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

#include "datamonitor/readstrategy/dmmreadstrategy.hpp"
#include <QDate>
#include <QDebug>
#include <QwtDate>
#include <timemanager.hpp>

using namespace scopy;
using namespace datamonitor;

DMMReadStrategy::DMMReadStrategy(iio_device *dev, iio_channel *chn)
	: dev(dev)
	, chn(chn)
{}

void DMMReadStrategy::read()
{
	double raw = 0;
	int readRaw = iio_channel_attr_read_double(chn, "raw", &raw);

	if(readRaw < 0) {
		char err[1024];
		iio_strerror(-(int)readRaw, err, sizeof(err));
		qDebug() << "device read error " << err;

	} else {
		double result = raw;
		qDebug() << "dmm read success  ";

		auto &&timeTracker = TimeManager::GetInstance();
		double currentTime = QwtDate::toDouble(timeTracker->lastReadValue());

		Q_EMIT readDone(currentTime, result);
	}
}
