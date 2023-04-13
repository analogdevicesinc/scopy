#include "qpluginloader.h"
#include <QTest>
#include <QList>
#include <pluginbase/plugin.h>

using namespace scopy;

class TST_M2k : public QObject
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
#define FILENAME PLUGIN_LOCATION "/libscopym2kplugin.so"


void TST_M2k::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	qDebug()<<QDir::currentPath();
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_M2k::isLibrary()
{
	QVERIFY(QLibrary::isLibrary(FILENAME));
}

void TST_M2k::className()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.metaData().value("className") == "M2kPlugin");
}

void TST_M2k::loaded()
{
	QPluginLoader qp(FILENAME,this);
	qp.load();
	qDebug()<<(qp.errorString());
	QVERIFY(qp.isLoaded());
}

void TST_M2k::instanceNotNull()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_M2k::multipleInstances()
{
	QPluginLoader qp1(FILENAME,this);
	QPluginLoader qp2(FILENAME,this);

	QVERIFY(qp1.instance() == qp2.instance());
}


void TST_M2k::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME,this);
	auto instance = qobject_cast<Plugin*>(qp.instance());
	QVERIFY(instance != nullptr );
}


void TST_M2k::clone()
{
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	p1 = original->clone(this);
	QVERIFY(p1 != nullptr);
	p2 = original->clone(this);
	QVERIFY(p2 != nullptr);
	QVERIFY(p1 != p2);
}

void TST_M2k::name() {
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	p1 = original->clone(this);
	qDebug()<<p1->name();
}

void TST_M2k::metadata()
{
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	original->initMetadata();
	p1 = original->clone(this);
	qDebug()<<p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

void TST_M2k::unload()
{
	QPluginLoader qp(FILENAME,this);
	auto original = qobject_cast<Plugin*>(qp.instance());

//	qp.unload();
	QVERIFY(!qp.isLoaded() == false);

}


QTEST_MAIN(TST_M2k)

#include "tst_pluginloader.moc"
