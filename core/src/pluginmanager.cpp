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

#include "pluginmanager.h"
#include "qpluginloader.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QLoggingCategory>
#include <pluginfilter.h>

#include <algorithm>

Q_LOGGING_CATEGORY(CAT_PLUGINMANAGER, "PluginManager")
using namespace scopy;

PluginManager::PluginManager(QObject *parent)
	: QObject(parent)
{}

PluginManager::~PluginManager()
{
	for(const PluginInfo &p : qAsConst(m_plugins)) {
		if(p.pluginInstance()) {
			p.pluginInstance()->deinit();
		}
	}
}

void PluginManager::add(QStringList pluginFileList)
{
	for(const QString &pluginFileName : pluginFileList) {
		add(pluginFileName);
	}
}

void PluginManager::add(QString pluginFileName)
{
	Plugin *p = nullptr;
	p = loadPlugin(pluginFileName);
	if(p) {
		Q_EMIT startLoadPlugin(p->name());
		qInfo(CAT_PLUGINMANAGER) << "Found plugin:" << p->name() << "in " << pluginFileName;
		p->initMetadata();
		applyMetadata(p, &m_metadata);
		p->init();
		QObject *obj = dynamic_cast<QObject *>(p);
		if(obj) {
			obj->setParent(this);
		}
	}
}

int PluginManager::count() { return m_plugins.count(); }

void PluginManager::applyMetadata(Plugin *plugin, QJsonObject *metadata)
{
	if(metadata->contains(plugin->name())) {
		plugin->setMetadata(metadata->value(plugin->name()).toObject());
	}
}

void PluginManager::sort(bool ascending)
{
	std::sort(m_plugins.begin(), m_plugins.end(), [ascending](const PluginInfo &p1, const PluginInfo &p2) {
		if(!p1.isLoaded() && !p2.isLoaded()) {
			return ascending ? p1.name() < p2.name() : p1.name() > p2.name();
		}
		if(!p1.isLoaded()) {
			return !ascending;
		}
		if(!p2.isLoaded()) {
			return ascending;
		}
		int priority1 = p1.pluginInstance()->metadata()["priority"].toInt();
		int priority2 = p2.pluginInstance()->metadata()["priority"].toInt();
		return ascending ? priority1 < priority2 : priority1 > priority2;
	});

	qDebug(CAT_PLUGINMANAGER) << "New plugin order:";
	for(const PluginInfo &plugin : qAsConst(m_plugins)) {
		qDebug(CAT_PLUGINMANAGER) << plugin.name();
	}
}

void PluginManager::clear() { m_plugins.clear(); }

QList<Plugin *> PluginManager::getPlugins(QString category)
{
	QList<Plugin *> newlist;
	const QList<PluginInfo> loaded = getLoadedPlugins();
	for(const PluginInfo &pluginInfo : loaded) {
		Plugin *plugin = pluginInfo.pluginInstance();
		if(!PluginFilter::pluginInCategory(plugin, category)) {
			continue;
		}
		Plugin *p = plugin->clone();
		newlist.append(p);
	}
	return newlist;
}

QList<Plugin *> PluginManager::getCompatiblePlugins(QString param, QString category)
{
	QList<Plugin *> comp;
	const QList<PluginInfo> loaded = getLoadedPlugins();
	for(const PluginInfo &pluginInfo : loaded) {
		Plugin *plugin = pluginInfo.pluginInstance();
		if(!PluginFilter::pluginInCategory(plugin, category)) {
			continue;
		}
		bool enable = (!PluginFilter::pluginInExclusionList(comp, plugin));
		bool forcedInclusion = (PluginFilter::pluginForcedInclusionList(comp, plugin));

		if(plugin->compatible(param, category) || forcedInclusion) {
			Plugin *p = plugin->clone();
			p->setParam(param, category);
			p->setEnabled(enable);
			comp.append(p);
		}
	}
	return comp;
}

QList<PluginInfo> PluginManager::getPluginsInfo() const { return m_plugins; }

QList<PluginInfo> PluginManager::getLoadedPlugins() const
{
	QList<PluginInfo> loaded;
	for(const PluginInfo &pInfo : qAsConst(m_plugins)) {
		if(pInfo.isLoaded()) {
			loaded.append(pInfo);
		}
	}
	return loaded;
}

QList<PluginInfo> PluginManager::getUnloadedPlugins() const
{
	QList<PluginInfo> unloaded;
	for(const PluginInfo &pInfo : qAsConst(m_plugins)) {
		if(!pInfo.isLoaded()) {
			unloaded.append(pInfo);
		}
	}
	return unloaded;
}

void PluginManager::setMetadata(QJsonObject metadata) { m_metadata = metadata; }

Plugin *PluginManager::loadPlugin(QString file)
{
	bool ret;
	Plugin *original = nullptr;
	Plugin *clone = nullptr;
	QObject *inst = nullptr;
	QPluginLoader qp;
	QString cloneName;
	QString errorMsg;
	PluginInfo::PluginState state = PluginInfo::PLUGIN_UNLOADED;
	if(!QFile::exists(file)) {
		state = PluginInfo::PLUGIN_FILE_NOT_FOUND;
		goto finish;
	}

	if(!QLibrary::isLibrary(file)) {
		state = PluginInfo::PLUGIN_INVALID_LIBRARY;
		goto finish;
	}

	qp.setFileName(file);
	ret = qp.load();
	if(!ret) {
		errorMsg = "Cannot load library " + qp.fileName() + "- err: " + qp.errorString();
		state = PluginInfo::PLUGIN_LOAD_FAILED;
		goto finish;
	}

	inst = qp.instance();
	if(!inst) {
		state = PluginInfo::PLUGIN_INSTANCE_FAILED;
		goto finish;
	}

	original = qobject_cast<Plugin *>(qp.instance());
	if(!original) {
		state = PluginInfo::PLUGIN_INVALID_PLUGIN;
		goto finish;
	}

	clone = original->clone(this);
	if(!clone) {
		state = PluginInfo::PLUGIN_CLONE_FAILED;
		goto finish;
	}

	cloneName = clone->name();

	if(cloneName == "") {
		state = PluginInfo::PLUGIN_CLONE_FAILED;
		goto finish;
	}
	state = PluginInfo::PLUGIN_LOADED;
finish:
	PluginInfo pluginInfo(file, state, errorMsg, clone);
	m_plugins.append(pluginInfo);
	if(!pluginInfo.isLoaded()) {
		qWarning(CAT_PLUGINMANAGER) << pluginInfo.getErrorMessage();
	}
	return clone;
}

QList<Plugin *> PluginManager::getOriginalPlugins() const
{
	QList<Plugin *> originalList;
	const QList<PluginInfo> loaded = getLoadedPlugins();
	for(const PluginInfo &pluginInfo : loaded) {
		originalList.append(pluginInfo.pluginInstance());
	}
	return originalList;
}

QJsonObject PluginManager::metadata() const { return m_metadata; }

#include "moc_pluginmanager.cpp"
