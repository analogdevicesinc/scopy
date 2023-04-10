#include "toolmanager.h"
#include "pluginbase/preferences.h"
#include <QLoggingCategory>
#include <QDebug>
#include <QEvent>

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
	for(ToolMenuEntry *tme : qAsConst(map[s].tools)) {
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
			disconnect(tme,SIGNAL(updateTool()),this,SLOT(updateTool()));
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

void ToolManager::updateTool() {

	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	Q_ASSERT(tme);
	QString id = tme->uuid();
	QString name = tme->name();
	QWidget* tool = tme->tool();

	if(ts->get(id) != tool) {
		if(ts->contains(id)) {
			saveToolAttachedState(tme);
			if(!tme->attached())
				saveToolGeometry(tme,ts->get(id));
			ts->remove(id);
		}
		if(tool != nullptr) {
			ts->add(id,tool);
			loadToolAttachedState(tme);
		}
	}

	updateToolAttached();
	qDebug(CAT_TOOLMANAGER) << "updating tool for " << tme->name() <<" - "<< id;
}

void ToolManager::updateToolAttached() {
	ToolMenuEntry* tme = dynamic_cast<ToolMenuEntry*>(QObject::sender());
	Q_ASSERT(tme);
	QWidget* tool = tme->tool();
	QString id = tme->uuid();
	QString name = tme->name();

	if(ts->contains(id)) {
		saveToolAttachedState(tme);

		ts->setAttached(id,tme->attached());
		tool->setAttribute(Qt::WA_QuitOnClose, tme->attached());

		if(!tme->attached()) {
			tool->installEventFilter(this);
			loadToolGeometry(tme,tool);
		} else {
			tool->removeEventFilter(this);
		}
	}
}

bool ToolManager::eventFilter(QObject *object, QEvent *event)
{
	ToolMenuEntry *tme = nullptr;
	bool handled = false;
	if(event->type() == QEvent::ParentAboutToChange || event->type() == QEvent::Close || event->type() == QEvent::Hide) {
		handled = true;
		QWidget *w = dynamic_cast<QWidget*>(object);
		if(!w)
			return false;
		for(auto &&st : map) {
			tme = ToolMenuEntry::findToolMenuEntryByTool(st.tools,w);
			if(tme)
				break;
		}
		if(!tme)
			return false;

		saveToolGeometry(tme, w);
		if(event->type() == QEvent::Close) {
			tme->setAttached(true);

		}
		showTool(tme->uuid());
	}

	return handled;
}

void ToolManager::showTool(QString s) {
	tm->getToolMenuItemFor(s)->getToolBtn()->setChecked(true);
	Q_EMIT tm->requestToolSelect(s);
}

void ToolManager::toggleAttach(QString id)
{
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
//	QString prefGrp = m_name;

	prefId = tme->id()+"_attached";
	p->init(prefId, tme->attached());
	bool attach = p->get(prefId).toBool();
	tme->setAttached(attach);
	qDebug(CAT_TOOLMANAGER)<<"Loading " << prefId << " " << attach;
}

void ToolManager::saveToolGeometry(ToolMenuEntry *tme, QWidget *w) {
	Preferences *p = Preferences::GetInstance();
	QString prefId;
	if(w) {
		prefId = tme->id()+"_geometry";
		QRect geometry = w->geometry();
		p->set(prefId, geometry);
		qDebug(CAT_TOOLMANAGER)<<"Saving " << prefId << " " << geometry;
	}
}

void ToolManager::loadToolGeometry(ToolMenuEntry *tme, QWidget *w) {
	Preferences *p = Preferences::GetInstance();
	QString prefId;

	if(w && !tme->attached()) {
		prefId = tme->id()+"_geometry";
		p->init(prefId, tme->tool()->geometry());
		QRect geometry = p->get(prefId).toRect();
		w->setGeometry(geometry);
		qDebug(CAT_TOOLMANAGER)<<"Loading " <<prefId<<tme->tool()->geometry();
	}
}


#include "moc_toolmanager.cpp"
