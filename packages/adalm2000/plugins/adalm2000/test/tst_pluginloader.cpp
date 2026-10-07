/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QTest>

#include <pluginbase/plugin.h>

using namespace scopy;

class TST_Adalm2000Plugin : public QObject
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
	void metadataExcludesGenericPlugins();
	void compatibleReturnsFalseWithoutContext();
};

#define PLUGIN_LOCATION "../.."
#define FILENAME PLUGIN_LOCATION "/libscopy-adalm2000.so"

void TST_Adalm2000Plugin::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_Adalm2000Plugin::isLibrary() { QVERIFY(QLibrary::isLibrary(FILENAME)); }

void TST_Adalm2000Plugin::className()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.metaData().value("className") == "Adalm2000Plugin");
}

void TST_Adalm2000Plugin::loaded()
{
	QPluginLoader qp(FILENAME, this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_Adalm2000Plugin::instanceNotNull()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_Adalm2000Plugin::multipleInstances()
{
	QPluginLoader qp1(FILENAME, this);
	QPluginLoader qp2(FILENAME, this);

	QVERIFY(qp1.instance() == qp2.instance());
}

void TST_Adalm2000Plugin::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME, this);
	auto instance = qobject_cast<Plugin *>(qp.instance());
	QVERIFY(instance != nullptr);
}

void TST_Adalm2000Plugin::clone()
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

void TST_Adalm2000Plugin::name()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	qDebug() << p1->name();
}

void TST_Adalm2000Plugin::metadata()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	p1 = original->clone();
	qDebug() << p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

void TST_Adalm2000Plugin::metadataExcludesGenericPlugins()
{
	// No generic plugin on an M2K device: plugins judged compatible against the
	// generic context would otherwise find the libm2k-shaped tree after the
	// backend swap.
	QPluginLoader qp(FILENAME, this);
	auto *original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	Plugin *p = original->clone();
	const QJsonObject md = p->metadata();

	QVERIFY(md.contains("exclude"));
	const QJsonArray exclude = md.value("exclude").toArray();
	QCOMPARE(exclude.size(), 1);
	QCOMPARE(exclude.at(0).toString(), QStringLiteral("*"));

	// A false from onConnect() must abort the connect.
	QCOMPARE(md.value("disconnectDevOnConnectFailure").toBool(), true);
}

void TST_Adalm2000Plugin::compatibleReturnsFalseWithoutContext()
{
	// Controller::context() is acquire-existing-only, so compatible() sees an empty
	// handle whenever nothing holds the URI and must answer false, not dereference.
	QPluginLoader qp(FILENAME, this);
	auto *original = qobject_cast<Plugin *>(qp.instance());
	Plugin *p = original->clone();
	QCOMPARE(p->compatible(QStringLiteral("ip:203.0.113.1"), QStringLiteral("iio")), false);
}

QTEST_MAIN(TST_Adalm2000Plugin)

#include "tst_pluginloader.moc"
