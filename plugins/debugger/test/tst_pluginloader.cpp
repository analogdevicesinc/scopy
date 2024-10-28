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

#include "qpluginloader.h"

#include <QList>
#include <QTest>

#include <pluginbase/plugin.h>

using namespace scopy;

class TST_Debugger : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void fileExists();
	void isLibrary();
	void loaded();
	void className();
	void instanceNotNull();
	void multipleInstances();
	void qobjectcast_to_plugin();
	void clone();
	void name();
	void metadata();
	void unload();
};

#define PLUGIN_LOCATION "../../plugins"
#define FILENAME PLUGIN_LOCATION "/libscopy-debugger.so"

void TST_Debugger::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	qDebug() << QDir::currentPath();
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_Debugger::isLibrary() { QVERIFY(QLibrary::isLibrary(FILENAME)); }

void TST_Debugger::className()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.metaData().value("className") == "DebuggerPlugin");
}

void TST_Debugger::loaded()
{
	QPluginLoader qp(FILENAME, this);
	qp.load();
	qDebug() << (qp.errorString());
	QVERIFY(qp.isLoaded());
}

void TST_Debugger::instanceNotNull()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_Debugger::multipleInstances()
{
	QPluginLoader qp1(FILENAME, this);
	QPluginLoader qp2(FILENAME, this);

	QVERIFY(qp1.instance() == qp2.instance());
}

void TST_Debugger::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME, this);
	auto instance = qobject_cast<Plugin *>(qp.instance());
	QVERIFY(instance != nullptr);
}

void TST_Debugger::clone()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone(this);
	QVERIFY(p1 != nullptr);
	p2 = original->clone(this);
	QVERIFY(p2 != nullptr);
	QVERIFY(p1 != p2);
}

void TST_Debugger::name()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone(this);
	qDebug() << p1->name();
}

void TST_Debugger::metadata()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	p1 = original->clone(this);
	qDebug() << p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

void TST_Debugger::unload()
{
	QPluginLoader qp(FILENAME, this);
	auto original = qobject_cast<Plugin *>(qp.instance());

	//	qp.unload();
	QVERIFY(!qp.isLoaded() == false);
}

QTEST_MAIN(TST_Debugger)

#include "tst_pluginloader.moc"
