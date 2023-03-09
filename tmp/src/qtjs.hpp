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

#ifndef SCOPY_QTJS_HPP
#define SCOPY_QTJS_HPP

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
class QJSEngine;

namespace adiscope {

class QtJs : public QObject
{
	Q_OBJECT

public:
	explicit QtJs(QJSEngine *engine);

	Q_INVOKABLE void exit();
	Q_INVOKABLE void sleep(unsigned long s);
	Q_INVOKABLE void msleep(unsigned long ms);
	Q_INVOKABLE void printToConsole(const QString& text);
	Q_INVOKABLE QString readFromConsole(const QString& text);
	Q_INVOKABLE void returnToApplication();

private:
	QFutureWatcher<QString> watcher;
	QFuture<QString> future;
	QString input;
	QString readInput();
};

}

#endif /* SCOPY_QTJS_HPP */
