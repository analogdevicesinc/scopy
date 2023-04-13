#include <QTest>
#include <QList>
#include "core/pluginmanager.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QLibrary>

using namespace scopy;

class TST_PluginManager : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void libsFound();
	void loadLibs();
	void metadataOps();
	void exclusion();
	void exclusionExcept();
private:

	void initFileList();
	QStringList libs;
};


#define NONPLUGIN_LIBRARY_LOCATION "../libscopycore.so"
#define PLUGIN_LOCATION "../../plugins/plugins"

void TST_PluginManager::libsFound()
{
	initFileList();
	QVERIFY2(libs.count() > 0, "No libs not found");
}

void TST_PluginManager::loadLibs()
{
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0,"Load libs failed");

	p->clear();
	QVERIFY2(p->count() == 0,"Clear libs failed");

	for(const auto &lib : qAsConst(libs)) {
		p->add(lib);
	}
	QVERIFY2(p->count() > 0,"Add 1-by-1 failed");

	QList<Plugin*> plugins;
	QList<Plugin*> sortedplugins;
	plugins = p->getPlugins("");

	QVERIFY2(plugins.count() == p->count(),"Plugin clone failed");

	p->sort();
	sortedplugins = p->getPlugins("");
	for(auto p : qAsConst(plugins)) {
		for(auto q : qAsConst(sortedplugins)) {
			if(p==q)
				QFAIL("duplicates found in sortedplugins vs plugins");
		}
	}

	QVERIFY2(plugins.count() == sortedplugins.count(),"Subsequent call to get plugin gives different counts");
	for (int i = 1;i<sortedplugins.count();i++) {
		if(sortedplugins[i-1]->metadata()["priority"].toInt() < sortedplugins[i]->metadata()["priority"].toInt())
			QFAIL("Sort by priority failed");
	}

	QList<Plugin*> usbPlugins = p->getCompatiblePlugins("usb:","test");
	for(auto &&p : usbPlugins) {
		QVERIFY2(p->param() == "usb:", "param not set to plugin");
	}

	QList<Plugin*> ipPlugins = p->getCompatiblePlugins("ip:","test");
	bool found = false;
	QVERIFY2(ipPlugins.count() > 0, "No ip: plugins found");
	for(auto &&p : ipPlugins) {
		if(p->name()=="TestPluginIp")
			found = true;
	}
	QVERIFY2(found, "TestPluginIp not found");

	p->add(NONPLUGIN_LIBRARY_LOCATION);
	QVERIFY2(p->count() == plugins.count(),"Added nonplugin library to manager");

	usbPlugins.clear();
	usbPlugins = p->getCompatiblePlugins("usb:","test");
	for(auto &&p : usbPlugins) {
		QVERIFY2(p->param() == "usb:", "param not set to plugin");
	}

	delete p;
}

void TST_PluginManager::metadataOps()
{
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0,"Load libs failed");

	QString json = QString(
			#include "testplugin.json"
				);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset)+"^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0,"Load libs failed");

	auto plugins = p->getPlugins("unittest");
	QVERIFY2(plugins.count() == 2,"Exactly 2 unit tests not found");
	qDebug()<<plugins[0]->name();

	QVERIFY2(plugins[0]->name()=="TestPluginIp", "TestPluginIp is not loaded with highest priority");
	QVERIFY2(plugins[0]->metadata()["priority"]==1000, "TestPluginIp priority not overridden");
}

void TST_PluginManager::exclusion()
{
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0,"Load libs failed");

	QString json = QString(
			#include "testpluginexclude.json"
				);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset)+"^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0,"Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:","unittest");
	QVERIFY2(plugins.count() == 1,"Exactly 1 unit tests not found");
	qDebug()<<plugins[0]->name();

	QVERIFY2(plugins[0]->name()=="TestPluginIp", "TestPluginIp is not the first plugin");
	QVERIFY2(plugins[0]->metadata()["exclude"]=="*", "TestPluginIp excludes everything");
}

void TST_PluginManager::exclusionExcept()
{
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0,"Load libs failed");

	QString json = QString(
			#include "testpluginexclude2.json"
				);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset)+"^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0,"Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:","unittest");
	QVERIFY2(plugins.count() == 2,"Exactly 1 unit tests not found");
	qDebug()<<plugins[0]->name();

	QVERIFY2(plugins[0]->name()=="TestPluginIp", "TestPluginIp is not loaded");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "*" && plugins[0]->metadata()["exclude"].toArray()[1]=="!TestPlugin", "TestPluginIP ");
	QVERIFY2(plugins[1]->name()=="TestPlugin", "Second TestPlugin is not loaded");
}

void TST_PluginManager::initFileList()
{
	QDir directory(PLUGIN_LOCATION);
	QStringList files = directory.entryList();
	libs.clear();
	for(const QString &file : files) {
		if(QLibrary::isLibrary(file)) {
			qDebug()<<"Library: " << file;
			libs.append(directory.absoluteFilePath(file));
		}
	}
}

QTEST_MAIN(TST_PluginManager)

#include "tst_pluginmanager.moc"
