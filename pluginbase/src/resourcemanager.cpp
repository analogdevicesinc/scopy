#include "resourcemanager.h"
#include <QApplication>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CAT_RESOURCEMANAGER,"ResourceManager");

using namespace adiscope;


ResourceManager* ResourceManager::pinstance_{nullptr};
ResourceManager::ResourceManager(QObject *parent) : QObject(parent)
{

}

ResourceManager::~ResourceManager() {

}

bool ResourceManager::open(QString resource, ResourceUser *res, bool force)
{
	ResourceManager *rm = ResourceManager::GetInstance();
	if(rm->map.contains(resource)) {
		if(force) {
			rm->map[resource]->stop();
			rm->map.insert(resource,res);
			return true;
		}
	} else {
		rm->map.insert(resource,res);
		return true;
	}
	return false;
}

void ResourceManager::close(QString resource) {
	ResourceManager *rm = ResourceManager::GetInstance();
	if(rm->map.contains(resource)) {
		rm->map.remove(resource);
	}
}

ResourceManager *ResourceManager::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new ResourceManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}




#include "moc_resourcemanager.cpp"
