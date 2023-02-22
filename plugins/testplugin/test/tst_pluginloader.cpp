#include "qpluginloader.h"
#include <QTest>
#include <QList>
#include <pluginbase/plugin.h>

using namespace adiscope;

class TST_TestPlugin : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void fileExists();
	void isLibrary();
	void fileName();	
	void loaded();
	void className();
	void instanceNotNull();
	void multipleInstances();
	void qobjectcast_to_plugin();
	void clone();
	void name();

};

#define FILENAME "/home/adi/tmp/build-tool_launcher-Desktop_Qt_5_15_2_GCC_64bit-Debug/plugins/testplugin/libscopytestplugin.so"


void TST_TestPlugin::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_TestPlugin::isLibrary()
{
	QVERIFY(QLibrary::isLibrary(FILENAME));
}

void TST_TestPlugin::fileName()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.fileName()==FILENAME);
}

void TST_TestPlugin::className()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.metaData().value("className") == "TestPlugin");
}

void TST_TestPlugin::loaded()
{
	QPluginLoader qp(FILENAME,this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_TestPlugin::instanceNotNull()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_TestPlugin::multipleInstances()
{
	QPluginLoader qp1(FILENAME,this);
	QPluginLoader qp2(FILENAME,this);

	QVERIFY(qp1.instance() == qp2.instance());
}


void TST_TestPlugin::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME,this);
	auto instance = qobject_cast<Plugin*>(qp.instance());
	QVERIFY(instance != nullptr );
}


void TST_TestPlugin::clone()
{
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	p1 = original->clone();
	QVERIFY(p1 != nullptr);
	p2 = original->clone();
	QVERIFY(p2 != nullptr);
	QVERIFY(p1 != p2);
}

void TST_TestPlugin::name() {
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	p1 = original->clone();
	qDebug()<<p1->name();
}


QTEST_MAIN(TST_TestPlugin)

#include "tst_pluginloader.moc"
