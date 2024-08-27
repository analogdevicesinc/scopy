#include "pluginmanager.h"

#include "pluginfilter.h"
#include "qpluginloader.h"

#include <QFile>
#include <QJsonArray>
#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(CAT_PLUGINMANAGER, "PluginManager")
using namespace scopy;

struct less_than_key
{
	inline bool operator()(Plugin *p1, Plugin *p2)
	{
		return (p1->metadata()["priority"].toInt() < p2->metadata()["priority"].toInt());
	}
};

struct greater_than_key
{
	inline bool operator()(Plugin *p1, Plugin *p2)
	{
		return (p1->metadata()["priority"].toInt() > p2->metadata()["priority"].toInt());
	}
};

PluginManager::PluginManager(QObject *parent)
	: QObject(parent)
{}

PluginManager::~PluginManager()
{
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
	Plugin *p = nullptr;
	p = loadPlugin(pluginFileName);
	if(p) {
		qInfo(CAT_PLUGINMANAGER) << "Found plugin:" << p->name() << "in " << pluginFileName;
		list.append(p);
		p->initMetadata();
		applyMetadata(p, &m_metadata);
		p->init();
		QObject *obj = dynamic_cast<QObject *>(p);
		if(obj)
			obj->setParent(this);
	}
}

int PluginManager::count() { return list.count(); }

void PluginManager::applyMetadata(Plugin *plugin, QJsonObject *metadata)
{
	if(metadata->contains(plugin->name())) {
		plugin->setMetadata(metadata->value(plugin->name()).toObject());
	}
}

void PluginManager::sort()
{
	std::sort(list.begin(), list.end(), greater_than_key());

	qDebug(CAT_PLUGINMANAGER) << "New plugin order:";
	for(Plugin *plugin : qAsConst(list)) {
		qDebug(CAT_PLUGINMANAGER) << plugin->name();
	}
}

void PluginManager::clear() { list.clear(); }

QList<Plugin *> PluginManager::getPlugins(QString category)
{
	QList<Plugin *> newlist;
	for(Plugin *plugin : qAsConst(list)) {
		if(!PluginFilter::pluginInCategory(plugin, category))
			continue;
		Plugin *p = plugin->clone();
		newlist.append(p);
	}
	return newlist;
}

QList<Plugin *> PluginManager::getCompatiblePlugins(QString param, QString category)
{
	QList<Plugin *> comp;
	for(Plugin *plugin : qAsConst(list)) {
		if(!PluginFilter::pluginInCategory(plugin, category))
			continue;
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

void PluginManager::setMetadata(QJsonObject metadata) { m_metadata = metadata; }

Plugin *PluginManager::loadPlugin(QString file)
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
	if(!ret) {
		qWarning(CAT_PLUGINMANAGER) << "Cannot load library " + qp.fileName() + "- err: " + qp.errorString();
		return nullptr;
	}

	QObject *inst = qp.instance();
	if(!inst) {
		qWarning(CAT_PLUGINMANAGER) << "Cannot create QObject instance from loaded library";
		return nullptr;
	}

	original = qobject_cast<Plugin *>(qp.instance());
	if(!original) {
		qWarning(CAT_PLUGINMANAGER) << "Loaded library instance is not a Plugin*";
		return nullptr;
	}

	clone = original->clone(this);
	if(!clone) {
		qWarning(CAT_PLUGINMANAGER) << "clone method does not clone the object";
		return nullptr;
	}

	QString cloneName;
	cloneName = clone->name();

	if(cloneName == "")
		return nullptr;

	return clone;
}

QList<Plugin *> PluginManager::getOriginalPlugins() const { return list; }

QJsonObject PluginManager::metadata() const { return m_metadata; }

#include "moc_pluginmanager.cpp"