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

#include "qtjs.hpp"

#include <QApplication>
#include <QJSEngine>
#include <QMetaProperty>
#include <QThread>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <thread>
#include <iostream>

#include <tool_launcher.hpp>

using std::cout;
using namespace adiscope;

QtJs::QtJs(QJSEngine *engine) : QObject(engine)
{
	QJSValue js_obj = engine->newQObject(this);
	auto meta = metaObject();
	input = "";

	for (int i = meta->methodOffset();
			i < meta->methodCount(); i++) {
		QString name(meta->method(i).name());

		engine->globalObject().setProperty(name, js_obj.property(name));
	}

	engine->globalObject().setProperty("inspect",
			engine->evaluate("function(o) { for (each in o) { print(each); } }"));
}

void QtJs::exit()
{
	QApplication::closeAllWindows();
}

void QtJs::returnToApplication()
{
	bool done;
	connect(getToolLauncherInstance(), &ToolLauncher::launcherClosed,[&done](){
		done=true;
	});

	while(!done) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

void QtJs::sleep(unsigned long s)
{
	msleep(s * 1000);
}

void QtJs::msleep(unsigned long ms)
{
	QElapsedTimer timer;

	timer.start();
	while (!timer.hasExpired(ms)) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

void QtJs::printToConsole(const QString& text)
{
	cout << text.toStdString() << std::endl;
}

QString QtJs::readFromConsole(const QString& request)
{
	bool done = false;
	input = "";

	std::cout << request.toStdString() << std::endl;
	connect(&watcher, &QFutureWatcher<QString>::finished, [&]() {
		input = watcher.result();
		done = true;
	});
	future = QtConcurrent::run(this, &QtJs::readInput);
	watcher.setFuture(future);

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while (!done && (input == ""));

	return input;
}

QString QtJs::readInput()
{
	std::string in;
	std::cin >> in;
	return QString::fromStdString(in);
}
