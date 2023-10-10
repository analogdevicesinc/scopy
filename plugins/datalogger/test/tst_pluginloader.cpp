#include "qpluginloader.h"

#include <QList>
#include <QTest>

#include <pluginbase/plugin.h>

using namespace scopy;

class TST_DataLoggerPlugin : public QObject
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
#define FILENAME PLUGIN_LOCATION "/libscopy-datalogger.so"

void TST_DataLoggerPlugin::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	qDebug() << QDir::currentPath();
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_DataLoggerPlugin::isLibrary() { QVERIFY(QLibrary::isLibrary(FILENAME)); }

void TST_DataLoggerPlugin::className()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.metaData().value("className") == "DataLoggerPlugin");
}

void TST_DataLoggerPlugin::loaded()
{
	QPluginLoader qp(FILENAME, this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_DataLoggerPlugin::instanceNotNull()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_DataLoggerPlugin::multipleInstances()
{
	QPluginLoader qp1(FILENAME, this);
	QPluginLoader qp2(FILENAME, this);

	QVERIFY(qp1.instance() == qp2.instance());
}

void TST_DataLoggerPlugin::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME, this);
	auto instance = qobject_cast<Plugin *>(qp.instance());
	QVERIFY(instance != nullptr);
}

void TST_DataLoggerPlugin::clone()
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

void TST_DataLoggerPlugin::name()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	qDebug() << p1->name();
}

void TST_DataLoggerPlugin::metadata()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	p1 = original->clone();
	qDebug() << p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

QTEST_MAIN(TST_DataLoggerPlugin)

#include "tst_pluginloader.moc"
