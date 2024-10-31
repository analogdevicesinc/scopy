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

#include "fileregisterreadstrategy.hpp"

#include "../logging_categories.h"
#include "iregisterreadstrategy.hpp"
#include "utils.hpp"

#include <QFile>
#include <QString>
#include <QStringList>

using namespace scopy::regmap;

FileRegisterReadStrategy::FileRegisterReadStrategy(QString path)
	: path(path)
{}

void FileRegisterReadStrategy::read(uint32_t address)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << file.errorString();
		Q_EMIT readError("device read error");
	} else {
		while(!file.atEnd()) {
			QString line(file.readLine());
			QString addr(line.split(',').first());
			if(addr.contains(QString::number(address, 16))) {
				Q_EMIT readDone(address, Utils::convertQStringToUint32(line.split(',').at(1)));
				qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value "
							  << Utils::convertQStringToUint32(line.split(',').at(1));
				break;
			}
		}
	}
}

void FileRegisterReadStrategy::readAll()
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << file.errorString();
		Q_EMIT readError("device read error");
	} else {
		while(!file.atEnd()) {
			QString line(file.readLine());
			uint32_t address = Utils::convertQStringToUint32(line.split(',').first());
			uint32_t value = Utils::convertQStringToUint32(line.split(',').at(1));

			Q_EMIT readDone(address, value);
			qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value " << value;
		}
	}
}
