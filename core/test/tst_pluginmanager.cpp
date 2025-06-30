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

#include "core/pluginmanager.h"
#include "pkg-manager/pkgmanager.h"
#include "pkg-manager/pkgmanifestfields.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLibrary>
#include <QList>
#include <QTest>

using namespace scopy;

class TST_PluginManager : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void libsFound();
	void loadLibs();
	void metadataOps();
	void exclusion();
	void exclusionSpecificLowercase();
	void exclusionSpecificUppercase();
	void exclusionExcept();
	void exclusionExceptUppercase();
	void exclusionExceptLowercase();

private:
	void initFileList();
	bool basePkgsExist();
	bool hasBasePkgs = false;
	QStringList libs;
	const QStringList basePkgs = {"test-plugins", "generic-plugins"};
};

#define NONPLUGIN_LIBRARY_LOCATION "../libscopycore.so"
#define PACKAGES_LOCATION "../../packages"

void TST_PluginManager::libsFound()
{
	initFileList();
	hasBasePkgs = basePkgsExist();
	if(!hasBasePkgs) {
		return;
	}
	QVERIFY2(libs.count() > 0, "No libs not found");
}

void TST_PluginManager::loadLibs()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	p->clear();
	QVERIFY2(p->count() == 0, "Clear libs failed");

	for(const auto &lib : qAsConst(libs)) {
		p->add(lib);
	}
	QVERIFY2(p->count() > 0, "Add 1-by-1 failed");

	QList<Plugin *> plugins;
	QList<Plugin *> sortedplugins;
	plugins = p->getPlugins("");

	QVERIFY2(plugins.count() == p->count(), "Plugin clone failed");

	p->sort();
	sortedplugins = p->getPlugins("");
	for(auto p : qAsConst(plugins)) {
		for(auto q : qAsConst(sortedplugins)) {
			if(p == q)
				QFAIL("duplicates found in sortedplugins vs plugins");
		}
	}

	QVERIFY2(plugins.count() == sortedplugins.count(), "Subsequent call to get plugin gives different counts");
	for(int i = 1; i < sortedplugins.count(); i++) {
		if(sortedplugins[i - 1]->metadata()["priority"].toInt() <
		   sortedplugins[i]->metadata()["priority"].toInt())
			QFAIL("Sort by priority failed");
	}

	QList<Plugin *> usbPlugins = p->getCompatiblePlugins("usb:", "test");
	for(auto &&p : usbPlugins) {
		QVERIFY2(p->param() == "usb:", "param not set to plugin");
	}

	QList<Plugin *> ipPlugins = p->getCompatiblePlugins("ip:", "test");
	bool found = false;
	QVERIFY2(ipPlugins.count() > 0, "No ip: plugins found");
	for(auto &&p : ipPlugins) {
		if(p->name() == "TestPluginIp")
			found = true;
	}
	QVERIFY2(found, "TestPluginIp not found");

	usbPlugins.clear();
	usbPlugins = p->getCompatiblePlugins("usb:", "test");
	for(auto &&p : usbPlugins) {
		QVERIFY2(p->param() == "usb:", "param not set to plugin");
	}

	delete p;
}

void TST_PluginManager::metadataOps()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testplugin.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getPlugins("test");
	QVERIFY2(plugins.count() >= 2, "Exactly 2 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is not loaded with highest priority");
	QVERIFY2(plugins[0]->metadata()["priority"] == 1000, "TestPluginIp priority not overridden");
}

void TST_PluginManager::exclusion()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testpluginexclude.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Only TestPluginIp plugin compatible compatible");
	qDebug() << plugins[0]->name();
	qDebug() << plugins[1]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPlugin is the first plugin");
	QVERIFY2(plugins[0]->metadata()["exclude"] == "*", "TestPluginIp excludes everything");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "TestPluginIp is the second plugin");

	QVERIFY2(plugins[0]->enabled() == true, "TestPluginIp not enabled");
	QVERIFY2(plugins[1]->enabled() == false, "TestPlugin is enabled");
}

void TST_PluginManager::exclusionSpecificLowercase()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testPluginExcludeSpecificLower.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Exactly 1 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is the first plugin");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "testplugin", "TestPluginIP ");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "TestPlugin is the second plugin");

	QVERIFY2(plugins[0]->enabled() == true, "TestPluginIp not enabled");
	QVERIFY2(plugins[1]->enabled() == false, "TestPlugin is enabled");
}

void TST_PluginManager::exclusionSpecificUppercase()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testPluginExcludeSpecificUpper.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Exactly 1 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is the first plugin");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "TESTPLUGIN", "TestPluginIP ");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "TestPlugin is the second plugin");

	QVERIFY2(plugins[0]->enabled() == true, "TestPluginIp not enabled");
	QVERIFY2(plugins[1]->enabled() == false, "TestPlugin is enabled");
}

void TST_PluginManager::exclusionExcept()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testpluginexclude2.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Exactly 1 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is not loaded");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "*" &&
			 plugins[0]->metadata()["exclude"].toArray()[1] == "!TestPlugin",
		 "TestPluginIP ");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "Second TestPlugin is not loaded");
}

void TST_PluginManager::exclusionExceptUppercase()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testPluginExcludeUpper.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Exactly 1 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is not loaded");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "*" &&
			 plugins[0]->metadata()["exclude"].toArray()[1] == "!TESTPLUGIN",
		 "TestPluginIP ");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "Second TestPlugin is not loaded");
}

void TST_PluginManager::exclusionExceptLowercase()
{
	if(!hasBasePkgs) {
		return;
	}
	PluginManager *p = new PluginManager(this);
	p->add(libs);
	QVERIFY2(p->count() > 0, "Load libs failed");

	QString json = QString(
#include "testPluginExcludeLower.json"
	);
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical() << "JSON Parse error !" << err.errorString();
		qCritical() << json;
		qCritical() << QString(" ").repeated(err.offset) + "^";
	}
	QJsonObject obj = doc.object();
	p->clear();
	p->setMetadata(obj);
	p->add(libs);
	p->sort();
	QVERIFY2(p->count() > 0, "Load libs failed");

	auto plugins = p->getCompatiblePlugins("ip:", "unittest");
	QVERIFY2(plugins.count() == 2, "Exactly 1 unit tests not found");
	qDebug() << plugins[0]->name();

	QVERIFY2(plugins[0]->name() == "TestPluginIp", "TestPluginIp is not loaded");
	QVERIFY2(plugins[0]->metadata()["exclude"].toArray()[0] == "*" &&
			 plugins[0]->metadata()["exclude"].toArray()[1] == "!testplugin",
		 "TestPluginIP ");
	QVERIFY2(plugins[1]->name() == "TestPlugin", "Second TestPlugin is not loaded");
}

void TST_PluginManager::initFileList()
{
	PkgManager::GetInstance()->init(QSet<QString>() << PACKAGES_LOCATION);
	const QFileInfoList files = PkgManager::listFilesInfo(QStringList() << "plugins", QStringList() << "*.so");
	libs.clear();
	for(const QFileInfo &file : files) {
		if(QLibrary::isLibrary(file.fileName())) {
			qDebug() << "Library: " << file.fileName();
			libs.append(file.absoluteFilePath());
		}
	}
}

bool TST_PluginManager::basePkgsExist()
{
	QStringList idList;
	const QList<QVariantMap> pkgsMeta = PkgManager::getPkgsMeta();
	std::transform(pkgsMeta.cbegin(), pkgsMeta.cend(), std::back_inserter(idList),
		       [](const QVariantMap &v) { return v[PkgManifest::PKG_ID].toString(); });
	qDebug() << "Available packages:" << idList;
	for(const QString &pkgId : basePkgs) {
		if(!idList.contains(pkgId)) {
			qDebug() << "Base package cannot be found:" << pkgId;
			return false;
		}
	}
	return true;
}

QTEST_MAIN(TST_PluginManager)

#include "tst_pluginmanager.moc"
