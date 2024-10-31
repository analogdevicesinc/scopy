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

#include "fileregisterwritestrategy.hpp"

#include "../logging_categories.h"

#include <QFile>
#include <QTextStream>

using namespace scopy;
using namespace regmap;

FileRegisterWriteStrategy::FileRegisterWriteStrategy(QString path)
	: path(path)
{}

void FileRegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	// TODO HOW SHOULD WE TREAT WRITE ?
	QFile file(path);
	QTextStream exportStream(&file);
	if(!file.isOpen()) {
		file.open(QIODevice::Append);
		exportStream << QString::number(address, 16) << "," << QString::number(val, 16) << "\n";
	} else {
		qDebug(CAT_IIO_OPERATION) << "File already opened! ";
		Q_EMIT writeError("device write err");
	}

	if(file.isOpen())
		file.close();
}
