#ifndef PLUGINREPOSITORY_H
#define PLUGINREPOSITORY_H

#include <QMap>
#include <QObject>
#include "pluginmanager.h"
#include "scopy-core_export.h"

namespace scopy {

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
