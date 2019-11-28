/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "jsfileio.h"

#include <QFile>
#include <QTextStream>

using namespace adiscope;

JsFileIo::JsFileIo(QObject *parent): QObject(parent)
{
}

void JsFileIo::writeToFile(const QString& data, const QString& path)
{
	QFile file(path);

	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << data;
	}
}

void JsFileIo::appendToFile(const QString& data, const QString& path)
{
	QFile file(path);

	if (file.open(QIODevice::Append)) {
		QTextStream stream(&file);
		stream << data;
	}
}

QString JsFileIo::readAll(const QString& path)
{
	QFile file(path);
	QString data;

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		data = stream.readAll();
	}
	return data;
}

QString JsFileIo::readLine(const QString& path, const int lineNumber)
{
	QFile file(path);
	int i = 0;
	QString data;

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		while( i <= lineNumber ){
			data = stream.readLine();
			if(stream.atEnd())
				break;
			i++;
		}
	}
	return data;
}
