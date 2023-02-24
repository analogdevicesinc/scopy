#include "pluginmanager.h"
#include "qpluginloader.h"
#include <QLoggingCategory>
#include <QFile>
#include <algorithm>
#include <QJsonArray>

Q_LOGGING_CATEGORY(CAT_PLUGINMANAGER, "PluginManager")
using namespace adiscope;


struct less_than_key
{
    inline bool operator() (Plugin* p1, Plugin* p2)
    {
	return (p1->metadata()["priority"].toInt() < p2->metadata()["priority"].toInt());
    }
};


struct greater_than_key
{
    inline bool operator() (Plugin* p1, Plugin* p2)
    {
	return (p1->metadata()["priority"].toInt() > p2->metadata()["priority"].toInt());
    }
};

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{

}

PluginManager::~PluginManager() {}

void PluginManager::add(QStringList pluginFileList)
{
	for(const QString &pluginFileName : pluginFileList) {
		add(pluginFileName);
	}
}

void PluginManager::add(QString pluginFileName)
{
	Plugin* p = nullptr;
	p = loadPlugin(pluginFileName);
	if(p) {
		qInfo(CAT_PLUGINMANAGER) << "Found plugin:" << p->name() << "in " << pluginFileName;
		list.append(p);
		p->initMetadata();
		applyMetadata(p, &m_metadata);
	}
}

int PluginManager::count()
{
	return list.count();
}

void PluginManager::applyMetadata(Plugin* plugin, QJsonObject *metadata){
	if(metadata->contains(plugin->name())) {
		plugin->setMetadata(metadata->value(plugin->name()).toObject());
	}
}

void PluginManager::sort() {
	std::sort(list.begin(),list.end(),greater_than_key());

	qDebug(CAT_PLUGINMANAGER) << "New plugin order:";
	for(Plugin* plugin : qAsConst(list)) {
		qDebug(CAT_PLUGINMANAGER) << plugin->name();
	}
}

void PluginManager::clear() {
	list.clear();
}

QList<Plugin *> PluginManager::getPlugins(QString category)
{
	QList<Plugin *> newlist;
	for(Plugin* plugin : qAsConst(list)) {
		if(!pluginInCategory(plugin,category))
			continue;
		newlist.append(plugin->clone());
	}
	return newlist;
}

QList<Plugin *> PluginManager::getCompatiblePlugins(QString uri, QString category)
{
	QList<Plugin *> comp;
	for(Plugin* plugin : qAsConst(list)) {
		if(!pluginInCategory(plugin,category))
			continue;
		if(plugin->compatible(uri)) {
			Plugin* p = plugin->clone();
			p->setUri(uri);
			comp.append(p);
		}
	}
	return comp;
}

void PluginManager::setMetadata(QJsonObject metadata)
{
	m_metadata = metadata;
}

bool PluginManager::pluginInCategory(Plugin* p, QString category) { // PluginFilter class (?)
	if(category.isEmpty()) // no category selected
		return true;
	if(!p->metadata().contains("category")) // plugin metadata does not have category
		return true;
	QJsonValue categoryVal = p->metadata().value("category");
	if(categoryVal.isString()) // single category
		return category == p->metadata().value("category").toString();
	if(categoryVal.isArray()) { // list category
		for(auto v : categoryVal.toArray()) {
			if(!v.isString()) {
				continue;
			}
			if(v.toString() == category) {
				return true;
			}

		}
	}
	return false;

}

Plugin* PluginManager::loadPlugin(QString file)
{
	bool ret;
	Plugin *original = nullptr;
	Plugin *clone = nullptr;

	if(!QFile::exists(file))
		return nullptr;

	if(!QLibrary::isLibrary(file))
		return nullptr;

	QPluginLoader qp(file);
	ret = qp.load();
	if(!ret)
		return nullptr;

	QObject *inst = qp.instance();
	if(!inst)
		return nullptr;

	original = qobject_cast<Plugin*>(qp.instance());
	if(!original)
		return nullptr;

	clone = original->clone();
	if(!clone)
		return nullptr;

	QString cloneName;
	cloneName = clone->name();

	if(cloneName == "")
		return nullptr;

	return clone;
}

QJsonObject PluginManager::metadata() const
{
	return m_metadata;
}
