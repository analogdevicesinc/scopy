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

#include "scopyjs/scopyjs.h"
#include "scopyjs/jsfileio.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include <unistd.h>

#include <QApplication>

#include <QMetaProperty>
#include <QThread>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <thread>
#include <iostream>



using std::cout;
using namespace adiscope;

ScopyJS* ScopyJS::pinstance_{nullptr};
ScopyJS::ScopyJS(QObject *parent) : QObject(parent) {
}

void ScopyJS::init() {
	QJSValue js_obj = m_engine.newQObject(this);
	auto meta = metaObject();
	input = "";

	for (int i = meta->methodOffset();
			i < meta->methodCount(); i++) {
		QString name(meta->method(i).name());

		m_engine.globalObject().setProperty(name, js_obj.property(name));
	}

	m_engine.globalObject().setProperty("inspect()",
					  m_engine.evaluate("function(o) { for (each in o) { print(each); } }"));
	m_engine.installExtensions(QJSEngine::ConsoleExtension);
	m_engine.globalObject().setProperty("fileIO",
					     m_engine.newQObject(new JsFileIo(this)));

	if (isatty(STDIN_FILENO)) {
		// verify if stdin is a true TTY - prevents QtCreator Application output from flooding the application
		// notifier->setEnabled(false);
		notifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(hasText()));
	}
}

ScopyJS::~ScopyJS()
{

}

ScopyJS *ScopyJS::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new ScopyJS(QApplication::instance()); // singleton has the app as parent
		pinstance_->init();
	}
	return pinstance_;
}

void ScopyJS::exit()
{
	QApplication::closeAllWindows();
}

void ScopyJS::returnToApplication()
{
	bool done;
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,[&done](){
		done=true;
	});

	while(!done) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

QJSEngine *ScopyJS::engine()
{
	return &m_engine;
}

void ScopyJS::registerApi(ApiObject *obj)
{
	registerApi(obj,m_engine.globalObject());
}

void ScopyJS::unregisterApi(ApiObject *obj)
{
	unregisterApi(obj,m_engine.globalObject());
}

void ScopyJS::registerApi(ApiObject *obj, QJSValue parentObj)
{
	parentObj.setProperty(obj->objectName(), m_engine.newQObject(obj));
}

void ScopyJS::unregisterApi(ApiObject *obj, QJSValue parentObj)
{
	parentObj.deleteProperty(obj->objectName());
}

void ScopyJS::sleep(unsigned long s)
{
	msleep(s * 1000);
}

void ScopyJS::msleep(unsigned long ms)
{
	QElapsedTimer timer;

	timer.start();
	while (!timer.hasExpired(ms)) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

void ScopyJS::printToConsole(const QString& text)
{
	cout << text.toStdString() << std::endl;
}

QString ScopyJS::readFromConsole(const QString& request)
{
	bool done = false;
	input = "";

	std::cout << request.toStdString() << std::endl;
	connect(&watcher, &QFutureWatcher<QString>::finished, [&]() {
		input = watcher.result();
		done = true;
	});
	future = QtConcurrent::run(this, &ScopyJS::readInput);
	watcher.setFuture(future);

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while (!done && (input == ""));

	return input;
}

QString ScopyJS::readInput()
{
	std::string in;
	std::cin >> in;
	return QString::fromStdString(in);
}

void ScopyJS::hasText()
{
	QTextStream in(stdin);
	QTextStream out(stdout);

	QString str = in.readLine();
	js_cmd.append(str);

	qWarning()<<str;
	unsigned int nb_open_braces = js_cmd.count(QChar('{'));
	unsigned int nb_closing_braces = js_cmd.count(QChar('}'));

	if (nb_open_braces == nb_closing_braces) {
		QJSValue val = m_engine.evaluate(js_cmd);

		if (val.isError()) {
			out << "Exception:" << val.toString() << Qt::endl;
		} else if (!val.isUndefined()) {
			out << val.toString() << Qt::endl;
		}

		js_cmd.clear();
		out << "scopy > ";
	} else {
		js_cmd.append(QChar('\n'));

		out << "> ";
	}

	out.flush();
}
