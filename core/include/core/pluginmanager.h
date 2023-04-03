#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QList>
#include <pluginbase/plugin.h>
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT PluginManager : public QObject
{

	Q_OBJECT
public:
	PluginManager(QObject *parent = nullptr);
	~PluginManager();
	void add(QStringList pluginFileList);
	void add(QString pluginFileName);
	int count();
	void sort();
	void clear();
	QList<Plugin*> getOriginalPlugins() const;
	QList<Plugin*> getPlugins(QString category = "");
	QList<Plugin*> getCompatiblePlugins(QString param, QString category = "");
	void setMetadata(QJsonObject metadata);
	QJsonObject metadata() const;

private:
	Plugin* loadPlugin(QString file);
	QList<Plugin*> list;
	QJsonObject m_metadata;

	void applyMetadata(Plugin *plugin, QJsonObject *metadata);
	bool pluginInCategory(Plugin *p, QString category);
};
}
#endif // PLUGINMANAGER_H
