#ifndef PLUGINREPOSITORY_H
#define PLUGINREPOSITORY_H

#include "pluginmanager.h"
#include "scopy-core_export.h"

#include <QMap>
#include <QObject>

namespace scopy {

class SCOPY_CORE_EXPORT PluginRepository : public QObject
{
	Q_OBJECT
public:
	PluginRepository(QObject *parent);
	~PluginRepository();
	void init(QString location);
	PluginManager *getPluginManager() { return pm; }

private:
	PluginManager *pm;
	QJsonObject metadata;
};
} // namespace scopy

#endif // PLUGINREPOSITORY_H
