/*
 * Copyright 2017 Analog Devices, Inc.
 * Licensed under the GPLv3; see the file LICENSE.
 */

#include "qtjs.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QJSEngine>
#include <QMetaProperty>
#include <QThread>
#include <QtConcurrent>

#include <iostream>
#include <thread>

using std::cout;
using namespace adiscope;

QtJs::QtJs(QJSEngine *engine) : QObject(engine) {
	QJSValue js_obj = engine->newQObject(this);
	auto meta = metaObject();
	input = "";

	for (unsigned int i = meta->methodOffset(); i < meta->methodCount();
	     i++) {
		QString name(meta->method(i).name());

		engine->globalObject().setProperty(name, js_obj.property(name));
	}

	engine->globalObject().setProperty(
		"inspect",
		engine->evaluate(
			"function(o) { for (each in o) { print(each); } }"));
}

void QtJs::exit() { QApplication::closeAllWindows(); }

void QtJs::sleep(unsigned long s) { msleep(s * 1000); }

void QtJs::msleep(unsigned long ms) {
	QElapsedTimer timer;

	timer.start();
	while (!timer.hasExpired(ms)) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

void QtJs::printToConsole(const QString &text) {
	cout << text.toStdString() << std::endl;
}

QString QtJs::readFromConsole(const QString &request) {
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

QString QtJs::readInput() {
	std::string in;
	std::cin >> in;
	return QString::fromStdString(in);
}
