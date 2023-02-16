#include "toolmanager.h"
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_TOOLMANAGER, "ToolManager")
using namespace adiscope;
ToolManager::ToolManager(ToolMenu *tm, QObject *parent ) : QObject(parent)
{
	currentKey = "";
	this->tm = tm;
	qDebug(CAT_TOOLMANAGER) << "ctor";
}

ToolManager::~ToolManager() {
	qDebug(CAT_TOOLMANAGER) << "dtor";
}

void ToolManager::addToolList(QString s, QList<ToolMenuEntry> sl) {
	qDebug(CAT_TOOLMANAGER) << "added" << s << "with " << sl.length() << "entries";
	map[s] = {s,sl,false};
	if(map.count() == 1) {
		qDebug(CAT_TOOLMANAGER) << "first item, currentkey = " << s;
		currentKey = s;
		showToolList(s);
	}
}

void ToolManager::removeToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "removing" <<s;
	if(currentKey == s) {
		hideToolList(s);
	}
	map.take(s);
}

void ToolManager::changeToolListContents(QString s, QList<ToolMenuEntry> sl) {
	bool prev = map[s].lock;
	qInfo(CAT_TOOLMANAGER) << "changing" << s;
	removeToolList(s);
	addToolList(s,sl);
	if(prev == true || currentKey == s) {
		showToolList(s);
	}
}

void ToolManager::showToolList(QString s) {
	if(!map[currentKey].lock)
		hideToolList(currentKey);
	currentKey = s;

	qDebug(CAT_TOOLMANAGER) << "showing" << s;
	for(ToolMenuEntry &s : map[s].tools)
	{
		if(tm->getToolMenuItemFor(s.id) == nullptr)
			tm->addTool(s.id,s.name,s.icon);
	}

}

void ToolManager::hideToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "hiding" << s;
	for(ToolMenuEntry &s : map[s].tools)
	{
		if(tm->getToolMenuItemFor(s.id) != nullptr)
			tm->removeTool(s.id);
	}
}

void ToolManager::lockToolList(QString s) {
	map[s].lock = true;
	qDebug(CAT_TOOLMANAGER) << "locking" << s;
	if(currentKey != s) {
		showToolList(s);
	}

}

void ToolManager::unlockToolList(QString s){
	map[s].lock = false;
	qDebug(CAT_TOOLMANAGER) << "unlocking" << s;
	if(currentKey != s) {
		hideToolList(s);
	}
}


