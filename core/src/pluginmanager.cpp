#include "pluginmanager.h"
#include "qpluginloader.h"
#include <QLoggingCategory>
#include <QFile>
#include <algorithm>
#include <QJsonArray>
#include "pluginfilter.h"

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

PluginManager::~PluginManager() {
	for(Plugin *p : qAsConst(list)) {
		p->deinit();
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
	Plugin* p = nullptr;
	p = loadPlugin(pluginFileName);
	if(p) {
		qInfo(CAT_PLUGINMANAGER) << "Found plugin:" << p->name() << "in " << pluginFileName;
		list.append(p);
		p->initMetadata();
		applyMetadata(p, &m_metadata);
		p->init();
		QObject *obj = dynamic_cast<QObject*>(p);
		if(obj)
			obj->setParent(this);
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

QList<Plugin *> PluginManager::getPlugins(QString category, QObject *parent)
{
	QList<Plugin *> newlist;
	for(Plugin* plugin : qAsConst(list)) {
		if(!PluginFilter::pluginInCategory(plugin,category))
			continue;
		Plugin *p = plugin->clone();
		newlist.append(p);
		if(parent)
			dynamic_cast<QObject*>(p)->setParent(parent);
	}
	return newlist;
}

QList<Plugin *> PluginManager::getCompatiblePlugins(QString param, QString category, QObject *parent)
{
	QList<Plugin *> comp;
	for(Plugin* plugin : qAsConst(list)) {
		if(!PluginFilter::pluginInCategory(plugin,category))
			continue;
		if(!PluginFilter::pluginInExclusionList(comp,plugin))
			continue;
		if(plugin->compatible(param)) {
			Plugin* p = plugin->clone();
			if(parent)
				dynamic_cast<QObject*>(p)->setParent(parent);
			p->setParam(param);
			comp.append(p);
		}
	}
	return comp;
}

void PluginManager::setMetadata(QJsonObject metadata)
{
	m_metadata = metadata;
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

QList<Plugin *> PluginManager::getOriginalPlugins() const
{
	return list;
}

QJsonObject PluginManager::metadata() const
{
	return m_metadata;
}

#include "moc_pluginmanager.cpp"
