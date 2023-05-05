#ifndef PLUGINREPOSITORY_H
#define PLUGINREPOSITORY_H

#include <QMap>
#include <QObject>
#include "pluginmanager.h"
#include "scopy-core_export.h"

namespace scopy {


// Plugin path is different per system -
//  - Windows - In Scopy.exe location
//  - Linux - /usr/share/scopy/plugins,
//  -  macOS - similar(?)
//  - Android - Only in app cache - On init, copy plugins from data to cache (?))

//#if defined(Q_OS_WINDOWS) || defined(Q_OS_UNIX)
#define DEFAULT_PLUGIN_LOCATION "plugins/plugins"
//#endif

class SCOPY_CORE_EXPORT PluginRepository : public QObject
{
	Q_OBJECT
public:
	PluginRepository(QObject *parent);
	~PluginRepository();
	void init(QString location);
	PluginManager *getPluginManager() {return pm;}

private:
	PluginManager *pm;
	QJsonObject metadata;
};
}

#endif // PLUGINREPOSITORY_H
