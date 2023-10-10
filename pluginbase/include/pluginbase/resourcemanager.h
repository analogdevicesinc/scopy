#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "scopy-pluginbase_export.h"

#include <QMap>
#include <QObject>

namespace scopy {

class SCOPY_PLUGINBASE_EXPORT ResourceUser
{
public:
	virtual void stop() = 0; // call ResourceManager::close
};

class SCOPY_PLUGINBASE_EXPORT ResourceManager : public QObject
{
	Q_OBJECT
protected:
	ResourceManager(QObject *parent = nullptr);
	~ResourceManager();

public:
	// singleton
	ResourceManager(ResourceManager &other) = delete;
	void operator=(const ResourceManager &) = delete;
	static ResourceManager *GetInstance();

	static bool open(QString resource, ResourceUser *res, bool force = true);
	static void close(QString resource);

private:
	QMap<QString, ResourceUser *> map;
	static ResourceManager *pinstance_;
};
} // namespace scopy
#endif // RESOURCEMANAGER_H
