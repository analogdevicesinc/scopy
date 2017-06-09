/*
 * Copyright 2017 Analog Devices, Inc.
 * Licensed under the GPLv3; see the file LICENSE.
 */

#include "qtjs.hpp"

#include <QApplication>
#include <QJSEngine>
#include <QMetaProperty>
#include <QThread>
#include <QElapsedTimer>

using namespace adiscope;

QtJs::QtJs(QJSEngine *engine) : QObject(engine)
{
	QJSValue js_obj = engine->newQObject(this);
	auto meta = metaObject();

	for (unsigned int i = meta->methodOffset();
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
