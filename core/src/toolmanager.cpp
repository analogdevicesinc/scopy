#include "toolmanager.h"
#include "pluginbase/preferences.h"
#include <QLoggingCategory>
#include <QDebug>
#include <QEvent>


Q_LOGGING_CATEGORY(CAT_TOOLMANAGER, "ToolManager")
using namespace adiscope;

ToolManager::ToolManager(ToolMenu *tm, ToolStack *ts, DetachedToolWindowManager *dwm, QObject *parent ) : QObject(parent)
{
	currentKey = "";
	this->tm = tm;
	this->ts = ts;
	this->dwm = dwm;
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
	for(ToolMenuEntry *tme : qAsConst(map[s].tools)) {
		connect(tme,SIGNAL(updateToolAttached(bool)),this,SLOT(updateToolAttached(bool)));
		connect(tme,SIGNAL(updateTool(QWidget*)),this,SLOT(updateTool(QWidget*)));
	}
}

void ToolManager::removeToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "removing" <<s;
	if(currentKey == s) {
		hideToolList(s);
	}

	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		disconnect(tme,SIGNAL(updateToolAttached(bool)),this,SLOT(updateToolAttached(bool)));
		disconnect(tme,SIGNAL(updateTool(QWidget*)),this,SLOT(updateTool(QWidget*)));
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
			connect(m->getToolRunBtn(),SIGNAL(clicked(bool)),tme,SIGNAL(runClicked(bool)));
		}
		updateToolEntry(tme);
	}

}

void ToolManager::hideToolList(QString s) {
	qDebug(CAT_TOOLMANAGER) << "hiding" << s;
	for(ToolMenuEntry *tme : qAsConst(map[s].tools))
	{
		if(tm->getToolMenuItemFor(tme->uuid()) != nullptr) {
			disconnect(tme,SIGNAL(updateToolEntry()),this,SLOT(updateToolEntry()));
			tm->removeTool(tme->uuid());
		}
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

void ToolManager::updateToolEntry(ToolMenuEntry *tme) {
	auto m = tm->getToolMenuItemFor(tme->uuid());
	m->setVisible(tme->visible());
	m->setEnabled(tme->enabled());
	m->setName(tme->name());
	m->getToolRunBtn()->setEnabled(tme->runEnabled());
	m->getToolRunBtn()->setEnabled(tme->runBtnVisible());
	m->getToolRunBtn()->setChecked(tme->running());
	qDebug(CAT_TOOLMANAGER) << "updating toolmenuentry for " << tme->name() <<" - "<< tme->uuid();
}

void ToolManager::updateToolEntry() {
	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	Q_ASSERT(tme);
	updateToolEntry(tme);
}

void ToolManager::updateTool(QWidget* old) {

	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	Q_ASSERT(tme);
	QString id = tme->uuid();
	QString name = tme->name();
	QWidget* tool = tme->tool();

	if(old != nullptr) { // we had a widget
		saveToolAttachedState(tme);
		if(ts->contains(id)) {
			ts->remove(id);
		}
		if(dwm->contains(id)) {
			dwm->remove(id);
		}
	}
	if(tool != nullptr) { // we have a new widget
		if(tme->attached()) {
			ts->add(id,tool);
		} else {
			dwm->add(id,tme);
		}
		loadToolAttachedState(tme);
	}

	qDebug(CAT_TOOLMANAGER) << "updating tool for " << tme->name() <<" - "<< id;
}

void ToolManager::updateToolAttached(bool oldAttach) {
	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	Q_ASSERT(tme);
	QWidget* tool = tme->tool();
	QString id = tme->uuid();
	QString name = tme->name();

	if(tme->attached()) {
		if(dwm->contains(id)) {
			dwm->remove(id);
		}
		ts->add(id,tool);
		tm->attachSuccesful(id);
	} else {
		if(ts->contains(id)) {
			ts->remove(id);
		}
		dwm->add(id, tme);
		tm->detachSuccesful(id);
	}
	saveToolAttachedState(tme);
	showTool(id);
}

void ToolManager::showTool(QString s) {
	auto toolmenuitem = tm->getToolMenuItemFor(s);
	if(toolmenuitem)
		toolmenuitem->getToolBtn()->setChecked(true);
	Q_EMIT tm->requestToolSelect(s);
}

void ToolManager::toggleAttach(QString id)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_doubleclick_attach").toBool())
		return;
	for(auto &&toolist : map) {
		for( auto &&tme : toolist.tools) {
			if(tme->uuid() == id) {
				tme->setAttached(!tme->attached());
			}
		}
	}
}

void ToolManager::saveToolAttachedState(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;
	prefId = tme->id()+"_attached";
	bool attach = tme->attached();
	p->set(prefId, attach);
	qDebug(CAT_TOOLMANAGER)<<"Saving " << prefId << " " << attach;
}

void ToolManager::loadToolAttachedState(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	QString prefId;
	if(!p->get("general_save_attached").toBool())
		return;
//	QString prefGrp = m_name;

	prefId = tme->id()+"_attached";
	p->init(prefId, tme->attached());
	bool attach = p->get(prefId).toBool();
	tme->setAttached(attach);
	qDebug(CAT_TOOLMANAGER)<<"Loading " << prefId << " " << attach;
}

#include "moc_toolmanager.cpp"
