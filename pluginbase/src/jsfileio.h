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
#ifndef JSFILEIO_H
#define JSFILEIO_H

#include <QObject>

/* Private header */
namespace scopy {
class JsFileIo: public QObject
{
	Q_OBJECT

public:
	JsFileIo(QObject *parent = 0);

	Q_INVOKABLE void writeToFile(const QString& data, const QString& path);
	Q_INVOKABLE void appendToFile(const QString& data, const QString& path);
	Q_INVOKABLE QString readAll(const QString& path);
	Q_INVOKABLE QString readLine(const QString& path, const int lineNumber);
};

} /* namespace scopy */

#endif // JSFILEIO_H
