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
#include <QJSEngine>
#include "apiobject.h"
#include "scopypluginbase_export.h"
#include "scopypluginbase_config.h"
#include <QSocketNotifier>

class QJSEngine;

namespace scopy {

class SCOPYPLUGINBASE_EXPORT ScopyJS : public QObject
{
	Q_OBJECT

protected:
	ScopyJS(QObject *parent = nullptr);
	void init();
	~ScopyJS();

public:
	ScopyJS(ScopyJS &other) = delete;
	void operator=(const ScopyJS &) = delete;

	static ScopyJS *GetInstance();

	Q_INVOKABLE void exit();
	Q_INVOKABLE void sleep(unsigned long s);
	Q_INVOKABLE void msleep(unsigned long ms);
	Q_INVOKABLE void printToConsole(const QString& text);
	Q_INVOKABLE QString readFromConsole(const QString& text);
	Q_INVOKABLE void returnToApplication();

	QJSEngine *engine();

	void registerApi(ApiObject* obj);
	void unregisterApi(ApiObject* obj);
	void registerApi(ApiObject* obj, QJSValue parentObj);
	void unregisterApi(ApiObject* obj, QJSValue parentObj);

public Q_SLOTS:
	void hasText();

private:
	QFutureWatcher<QString> watcher;
	QFuture<QString> future;
	QString input;
	QString js_cmd;
	QString readInput();
#ifdef ENABLE_SCOPYJS
	QJSEngine m_engine;
#endif
	QSocketNotifier *notifier;
	bool done;
	static ScopyJS * pinstance_;
};

}

#endif /* SCOPY_QTJS_HPP */
