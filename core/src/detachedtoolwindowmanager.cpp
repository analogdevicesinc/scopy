#include "detachedtoolwindowmanager.h"

using namespace adiscope;

DetachedToolWindowManager::DetachedToolWindowManager(QObject *parent) : QObject(parent) {}

DetachedToolWindowManager::~DetachedToolWindowManager() {}

void DetachedToolWindowManager::add(QString id, ToolMenuEntry *tme) {
	if(map.contains(id)) {
		delete map.take(id);
	}
	map.insert(id, new DetachedToolWindow(nullptr,tme));
}

bool DetachedToolWindowManager::remove(QString id)
{
	if(map.contains(id)) {
		delete map.take(id);
		return true;
	}
	return false;
}

void DetachedToolWindowManager::show(QString id) {
	if(map.contains(id)) {
		map[id]->raise();
		map[id]->show();
	}
}

QWidget *DetachedToolWindowManager::getWidget(QString key)
{
	if(map.contains(key))
		return map[key];
	return nullptr;
}


bool DetachedToolWindowManager::contains(QString key) {
	return map.contains(key);
}

