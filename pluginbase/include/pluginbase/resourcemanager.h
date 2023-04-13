#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QMap>
#include "scopypluginbase_export.h"

namespace scopy {

class SCOPYPLUGINBASE_EXPORT ResourceUser {
public:
	virtual void stop() = 0; // call ResourceManager::close
};

class SCOPYPLUGINBASE_EXPORT ResourceManager : public QObject
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
	QMap<QString, ResourceUser*> map;
	static ResourceManager * pinstance_;
};
}
#endif // RESOURCEMANAGER_H
