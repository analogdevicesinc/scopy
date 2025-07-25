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

#include "pluginrepository.h"
#include <pkg-manager/pkgmanager.h>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#include <Winbase.h>
#endif

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLibrary>
#include <QLoggingCategory>
#include <QtGlobal>

Q_LOGGING_CATEGORY(CAT_PLUGINREPOSTIORY, "PluginRepository");

using namespace scopy;

PluginRepository *PluginRepository::pinstance_{nullptr};

PluginRepository::PluginRepository(QObject *parent)
	: QObject(parent)
{
	pm = new PluginManager(this);
}

PluginRepository::~PluginRepository() {}

PluginRepository *PluginRepository::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new PluginRepository(QApplication::instance());
	}
	return pinstance_;
}

void PluginRepository::init(QString location) { pinstance_->_init(location); }

void PluginRepository::_init(QString location)
{
	qInfo(CAT_PLUGINREPOSTIORY) << "initializing plugins from: " << location;
	const QString pluginMetaFileName = "plugin.json";
	QString pluginMetaFilePath = "";
	QFileInfoList plugins = PkgManager::listFilesInfo(QStringList() << "plugins");
	if(!location.isEmpty() && QFile::exists(location)) {
		QDir loc(location);
		plugins.append(loc.entryInfoList(QDir::Files));
	}
	QStringList pluginFiles;

	for(const QFileInfo &p : qAsConst(plugins)) {
		if(p.fileName() == pluginMetaFileName) {
			pluginMetaFilePath = p.absoluteFilePath();
			continue;
		}
		pluginFiles.append(p.absoluteFilePath());
	}

	if(!pluginMetaFilePath.isEmpty()) {
		QFile f(pluginMetaFilePath);
		f.open(QFile::ReadOnly);
		QString json = f.readAll();
		QJsonParseError err;
		QJsonDocument pluginMetaDocument = QJsonDocument::fromJson(json.toUtf8(), &err);
		if(err.error != QJsonParseError::NoError) {
			qCritical(CAT_PLUGINREPOSTIORY) << "JSON Parse error !" << err.errorString();
			qCritical(CAT_PLUGINREPOSTIORY) << json;
			qCritical(CAT_PLUGINREPOSTIORY) << QString(" ").repeated(err.offset) + "^";
		} else {
			qDebug(CAT_PLUGINREPOSTIORY) << "Found valid json @ " << pluginMetaFilePath;
		}
		metadata = pluginMetaDocument.object();
		pm->setMetadata(metadata);
	}

#ifdef Q_OS_WINDOWS
	bool b = SetDllDirectoryA(QApplication::applicationDirPath().toStdString().c_str());
	if(!b) {
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		qWarning(CAT_PLUGINREPOSTIORY)
			<< "cannot add .exe folder to library search path - " << QString::fromStdString(message);
		;
	}
#endif

	pm->clear();
	pm->add(pluginFiles);
	pm->sort();
}

PluginManager *PluginRepository::getPluginManager() { return pinstance_->_getPluginManager(); }

PluginManager *PluginRepository::_getPluginManager() { return pm; }

QList<Plugin *> PluginRepository::getOriginalPlugins()
{
	PluginManager *pm = getPluginManager();
	return pm->getOriginalPlugins();
}

QList<Plugin *> PluginRepository::getPlugins(QString category)
{
	PluginManager *pm = getPluginManager();
	return pm->getPlugins(category);
}

QList<Plugin *> PluginRepository::getCompatiblePlugins(const QString &param, const QString &category)
{
	PluginManager *pm = getPluginManager();
	return pm->getCompatiblePlugins(param, category);
}

QList<PluginInfo> PluginRepository::getPluginsInfo()
{
	PluginManager *pm = getPluginManager();
	return pm->getPluginsInfo();
}

#include "moc_pluginrepository.cpp"
