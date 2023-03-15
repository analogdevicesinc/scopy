#include "toolmanager.h"
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_TOOLMANAGER, "ToolManager")
using namespace adiscope;
ToolManager::ToolManager(ToolMenu *tm, ToolStack *ts, QObject *parent ) : QObject(parent)
{
	currentKey = "";
	this->tm = tm;
	this->ts = ts;
	qDebug(CAT_TOOLMANAGER) << "ctor";
}

ToolManager::~ToolManager() {
	qDebug(CAT_TOOLMANAGER) << "dtor";
}

void ToolManager::addToolList(QString s, QList<ToolMenuEntry*> sl) {
	qDebug(CAT_TOOLMANAGER) << "added" << s << "with " << sl.length() << "entries";
	map[s] = {s,sl,false};
	if(map.count() == 1) {
		qDebug(CAT_TOOLMANAGER) << "first item, currentkey = " << s;
		currentKey = s;
		showToolList(s);
	}
	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		connect(tme,SIGNAL(updateTool()),this,SLOT(updateTool()));
	}
}

void ToolManager::removeToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "removing" <<s;
	if(currentKey == s) {
		hideToolList(s);
	}

	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		disconnect(tme,SIGNAL(updateTool()),this,SLOT(updateTool()));
	}
	map.take(s);
}

void ToolManager::changeToolListContents(QString s, QList<ToolMenuEntry*> sl) {
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
	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		ToolMenuItem *m = tm->getToolMenuItemFor(tme->uuid());
		if( m == nullptr) {
			m = tm->addTool(tme->uuid(),tme->name(),tme->icon());
			connect(tme,SIGNAL(updateToolEntry()),this,SLOT(updateToolEntry()));
			connect(m->getToolRunBtn(),SIGNAL(toggled(bool)),tme,SIGNAL(runToggled(bool)));
		}
		updateToolEntry(tme, tme->uuid());
	}

}

void ToolManager::hideToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "hiding" << s;
	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		if(tm->getToolMenuItemFor(tme->uuid()) != nullptr)
			disconnect(tme,SIGNAL(updateToolEntry()),this,SLOT(updateTool()));
			tm->removeTool(tme->uuid());
	}
}

void ToolManager::lockToolList(QString s) {
	map[s].lock = true;
	lockedToolLists.append(s);
	qDebug(CAT_TOOLMANAGER) << "locking" << s;
	if(currentKey != s) {
		showToolList(s);
	}

}

void ToolManager::unlockToolList(QString s){
	map[s].lock = false;
	lockedToolLists.removeOne(s);
	qDebug(CAT_TOOLMANAGER) << "unlocking" << s;
	if(currentKey != s) {
		hideToolList(s);
	}
}

void ToolManager::updateToolEntry(ToolMenuEntry *tme, QString s) {
	auto m = tm->getToolMenuItemFor(s);
	m->setVisible(tme->visible());
	m->setEnabled(tme->enabled());
	m->setName(tme->name());
	m->getToolRunBtn()->setEnabled(tme->runBtnVisible());
	m->getToolRunBtn()->setChecked(tme->running());
	qDebug(CAT_TOOLMANAGER) << "updating toolmenuentry for " << tme->name() <<" - "<< tme->uuid();
}

void ToolManager::updateToolEntry() {
	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	if(tme)
		updateToolEntry(tme,tme->uuid());
	else
		qWarning(CAT_TOOLMANAGER())<<"wrong QObject::sender() for updateToolEntry()";

}

void ToolManager::updateTool() {

	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	QString id = tme->uuid();
	QString name = tme->name();
	QWidget* tool = tme->tool();
	if(tme) {
		if(ts->contains(id))
			ts->remove(id);
		if(tool != nullptr)
			ts->add(id,tool);
		qDebug(CAT_TOOLMANAGER) << "updating tool for " << tme->name() <<" - "<< id;
	} else {
		qWarning(CAT_TOOLMANAGER()) << "Id not tracked by toolmanager - cannot add tool" << id;
	}
}

void ToolManager::showTool(QString s) {
	tm->getToolMenuItemFor(s)->getToolBtn()->setChecked(true);
	Q_EMIT tm->requestToolSelect(s);
}




#include "moc_toolmanager.cpp"
