/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "qpluginloader.h"

#include <QList>
#include <QTest>

#include <pluginbase/plugin.h>

using namespace scopy;

class TST_SWIOTREFACTORPlugin : public QObject
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
};

#define PLUGIN_LOCATION "../../plugins"
#define FILENAME PLUGIN_LOCATION "/libscopy-swiotrefactor.so"

void TST_SWIOTREFACTORPlugin::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_SWIOTREFACTORPlugin::isLibrary() { QVERIFY(QLibrary::isLibrary(FILENAME)); }

void TST_SWIOTREFACTORPlugin::className()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.metaData().value("className") == "SWIOTREFACTORPlugin");
}

void TST_SWIOTREFACTORPlugin::loaded()
{
	QPluginLoader qp(FILENAME, this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_SWIOTREFACTORPlugin::instanceNotNull()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_SWIOTREFACTORPlugin::multipleInstances()
{
	QPluginLoader qp1(FILENAME, this);
	QPluginLoader qp2(FILENAME, this);

	QVERIFY(qp1.instance() == qp2.instance());
}

void TST_SWIOTREFACTORPlugin::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME, this);
	auto instance = qobject_cast<Plugin *>(qp.instance());
	QVERIFY(instance != nullptr);
}

void TST_SWIOTREFACTORPlugin::clone()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	QVERIFY(p1 != nullptr);
	p2 = original->clone();
	QVERIFY(p2 != nullptr);
	QVERIFY(p1 != p2);
}

void TST_SWIOTREFACTORPlugin::name()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	qDebug() << p1->name();
}

void TST_SWIOTREFACTORPlugin::metadata()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	p1 = original->clone();
	qDebug() << p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

QTEST_MAIN(TST_SWIOTREFACTORPlugin)

#include "tst_pluginloader.moc"