/*
 * Copyright 2017 Analog Devices, Inc.
 * Licensed under the GPLv3; see the file LICENSE.
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

private:
	QFutureWatcher<QString> watcher;
	QFuture<QString> future;
	QString input;
	QString readInput();
};

}

#endif /* SCOPY_QTJS_HPP */
