#include "qpluginloader.h"
#include <QTest>
#include <QList>
#include <pluginbase/plugin.h>

using namespace adiscope;

class TST_BareMinimum : public QObject
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
#define FILENAME PLUGIN_LOCATION "/libscopybareminimumplugin.so"


void TST_BareMinimum::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	qDebug()<<QDir::currentPath();
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_BareMinimum::isLibrary()
{
	QVERIFY(QLibrary::isLibrary(FILENAME));
}

void TST_BareMinimum::className()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.metaData().value("className") == "BareMinimum");
}

void TST_BareMinimum::loaded()
{
	QPluginLoader qp(FILENAME,this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_BareMinimum::instanceNotNull()
{
	QPluginLoader qp(FILENAME,this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_BareMinimum::multipleInstances()
{
	QPluginLoader qp1(FILENAME,this);
	QPluginLoader qp2(FILENAME,this);

	QVERIFY(qp1.instance() == qp2.instance());
}


void TST_BareMinimum::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME,this);
	auto instance = qobject_cast<Plugin*>(qp.instance());
	QVERIFY(instance != nullptr );
}


void TST_BareMinimum::clone()
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

void TST_BareMinimum::name() {
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	p1 = original->clone();
	qDebug()<<p1->name();
}

void TST_BareMinimum::metadata()
{
	QPluginLoader qp(FILENAME,this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin*>(qp.instance());
	original->initMetadata();
	p1 = original->clone();
	qDebug()<<p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}


QTEST_MAIN(TST_BareMinimum)

#include "tst_pluginloader.moc"
