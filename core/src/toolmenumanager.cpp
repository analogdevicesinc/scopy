#include "toolmenumanager.h"
#include <toolmenuitem.h>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>
#include <QButtonGroup>
#include <baseheaderwidget.h>
#include <compositeheaderwidget.h>
#include <menuheader.h>

Q_LOGGING_CATEGORY(CAT_TOOLMENUMANAGER, "ToolMenuManager")
using namespace scopy;

ToolMenuManager::ToolMenuManager(ToolStack *ts, DetachedToolWindowManager *dtm, ToolMenu *toolMenu, QObject *parent)
	: QObject(parent)
	, m_ts(ts)
	, m_dtm(dtm)
	, m_toolMenu(toolMenu)
{}

ToolMenuManager::~ToolMenuManager() {}

void ToolMenuManager::addMenuItem(QString deviceId, QString devName, QList<ToolMenuEntry *> tools, int itemIndex)
{
	QString param;
	MenuSectionCollapseWidget *devSection = new MenuSectionCollapseWidget(
		devName, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_COMPOSITEWIDGET, m_toolMenu);
	devSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	if(!tools.isEmpty())
		param = tools.at(0)->param();
	createMenuSectionLabel(devSection, param);
	QButtonGroup *menuBtnGroup = m_toolMenu->btnGroup();
	for(ToolMenuEntry *tme : tools) {
		ToolMenuItem *toolMenuItem = createToolMenuItem(tme, devSection);
		devSection->add(toolMenuItem);
		menuBtnGroup->addButton(toolMenuItem->getToolBtn());
		connect(tme, &ToolMenuEntry::updateTool, this, &ToolMenuManager::updateTool);
		connect(tme, &ToolMenuEntry::updateToolAttached, this,
			[this, toolMenuItem](bool oldAttach) { updateToolAttached(oldAttach, toolMenuItem); });
		Q_EMIT tme->updateToolEntry();
	}
	m_toolMenu->add(itemIndex, deviceId, devSection);
	m_itemMap[deviceId] = devSection;
	devSection->setCollapsed(false);
	devSection->hide();
	qDebug(CAT_TOOLMENUMANAGER) << "Menu item with id" << deviceId << "has been added";
}

void ToolMenuManager::removeMenuItem(QString deviceId)
{
	if(!m_itemMap.contains(deviceId)) {
		qDebug(CAT_TOOLMENUMANAGER) << "No entry with id:" << deviceId;
		return;
	}
	MenuSectionCollapseWidget *devSection = m_itemMap[deviceId];
	m_itemMap.remove(deviceId);
	m_toolMenu->remove(devSection);
	delete devSection;
	devSection = nullptr;
	qDebug(CAT_TOOLMENUMANAGER) << "Menu item with id" << deviceId << "has been removed";
}

void ToolMenuManager::changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools)
{
	if(!m_itemMap.contains(deviceId)) {
		qDebug(CAT_TOOLMENUMANAGER) << "No entry with id:" << deviceId;
		return;
	}
	for(ToolMenuEntry *tme : tools) {
		tme->disconnect(this);
	}
	MenuSectionCollapseWidget *menuItem = m_itemMap[deviceId];
	QString devName = menuItem->collapseSection()->title();
	int itemIndex = m_toolMenu->indexOf(menuItem);
	removeMenuItem(deviceId);
	addMenuItem(deviceId, devName, tools, itemIndex);
	showMenuItem(deviceId);
}

void ToolMenuManager::showMenuItem(QString id)
{
	if(!m_connectedDev.contains(m_prevItem))
		hideMenuItem(m_prevItem);
	if(!m_itemMap.contains(id))
		return;
	m_itemMap[id]->show();
	m_prevItem = id;
}

void ToolMenuManager::hideMenuItem(QString id)
{
	if(!m_itemMap.contains(id))
		return;
	m_itemMap[id]->hide();
}

void ToolMenuManager::deviceConnected(QString id)
{
	m_connectedDev.append(id);
	showMenuItem(id);
}

void ToolMenuManager::deviceDisconnected(QString id)
{
	m_connectedDev.removeAll(id);
	if(m_prevItem.compare(id) != 0) {
		hideMenuItem(id);
	}
}

void ToolMenuManager::onDisplayNameChanged(QString id, QString devName)
{
	m_itemMap[id]->collapseSection()->setTitle(devName);
}

void ToolMenuManager::updateTool(QWidget *old)
{

	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QString id = tme->uuid();
	QWidget *tool = tme->tool();

	if(old != nullptr) { // we had a widget
		saveToolAttachedState(tme);
		if(m_ts->contains(id)) {
			m_ts->remove(id);
		}
		if(m_dtm->contains(id)) {
			m_dtm->remove(id);
		}
	}
	if(tool != nullptr) { // we have a new widget
		if(tme->attached()) {
			m_ts->add(id, tool);
		} else {
			m_dtm->add(id, tme);
		}
		loadToolAttachedState(tme);
	}
	qDebug(CAT_TOOLMENUMANAGER) << "updating tool for " << tme->name() << " - " << id;
}

void ToolMenuManager::updateToolAttached(bool oldAttach, ToolMenuItem *toolMenuItem)
{
	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QWidget *tool = tme->tool();
	QString id = tme->uuid();

	if(tme->attached()) {
		// tool is detached, it will attach to the main window
		if(m_dtm->contains(id)) {
			m_dtm->remove(id);
		}
		m_ts->add(id, tool);
		attachSuccesful(toolMenuItem);
	} else {
		// tool is attached, it will detach
		if(m_ts->contains(id)) {
			m_ts->remove(id);
		}
		m_dtm->add(id, tme);
		detachSuccesful(toolMenuItem);
	}
	// by this time, the tool has changed it's state, either from attached to detached, or from detached to attached
	saveToolAttachedState(tme);
	showTool(toolMenuItem);

	// highlight the current tool from the main window
	if(tme->attached()) {
		// the selected tool just attached, so it will be at the top of the stack, therefore highlighted
		if(toolMenuItem) {
			toolMenuItem->getToolBtn()->setChecked(true);
			toolMenuItem->setSelected(true);
		}
	} else {
		// the top tool just detached, so we need to find the tool that is positioned at the new top of the
		// stack.
		if(toolMenuItem) {
			toolMenuItem->getToolBtn()->toggle();
		}
	}
}

void ToolMenuManager::loadToolAttachedState(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	QString prefId;
	if(!p->get("general_save_attached").toBool())
		return;

	prefId = tme->id() + "_attached";
	p->init(prefId, tme->attached());
	bool attach = p->get(prefId).toBool();
	tme->setAttached(attach);
}

void ToolMenuManager::saveToolAttachedState(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;
	prefId = tme->id() + "_attached";
	bool attach = tme->attached();
	p->set(prefId, attach);
}

void ToolMenuManager::detachSuccesful(ToolMenuItem *toolMenuItem)
{
	QButtonGroup *menuBtnGroup = m_toolMenu->btnGroup();
	if(toolMenuItem) {
		toolMenuItem->setSelected(false);
		menuBtnGroup->removeButton(toolMenuItem->getToolBtn());
	}
}

void ToolMenuManager::attachSuccesful(ToolMenuItem *toolMenuItem)
{
	QButtonGroup *menuBtnGroup = m_toolMenu->btnGroup();
	if(toolMenuItem) {
		menuBtnGroup->addButton(toolMenuItem->getToolBtn());
	}
}

void ToolMenuManager::showTool(ToolMenuItem *toolMenuItem)
{
	if(toolMenuItem) {
		toolMenuItem->getToolBtn()->setChecked(true);
	}
	Q_EMIT requestToolSelect(toolMenuItem->getId());
}

void ToolMenuManager::selectTool(ToolMenuItem *toolMenuItem, bool on)
{
	QButtonGroup *menuBtnGroup = m_toolMenu->btnGroup();
	if(menuBtnGroup->id(toolMenuItem->getToolBtn()) != -1) {
		toolMenuItem->setSelected(on);
	}
}

void ToolMenuManager::setTmeAttached(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_doubleclick_attach").toBool())
		return;
	tme->setAttached(!tme->attached());
}

void ToolMenuManager::createMenuSectionLabel(MenuSectionCollapseWidget *section, QString uri)
{
	MenuCollapseHeader *collapseHeader = dynamic_cast<MenuCollapseHeader *>(section->collapseSection()->header());
	if(!collapseHeader) {
		return;
	}
	CompositeHeaderWidget *headerWidget = dynamic_cast<CompositeHeaderWidget *>(collapseHeader->headerWidget());
	if(headerWidget) {
		QLabel *uriLabel = new QLabel(uri, headerWidget);
		headerWidget->add(uriLabel);
	}
}

ToolMenuItem *ToolMenuManager::createToolMenuItem(ToolMenuEntry *tme, QWidget *parent)
{
	ToolMenuItem *toolMenuItem = new ToolMenuItem(tme->uuid(), tme->name(), tme->icon(), parent);
	connect(toolMenuItem->getToolRunBtn(), &QPushButton::toggled, tme, &ToolMenuEntry::runToggled);
	connect(toolMenuItem->getToolRunBtn(), &QPushButton::clicked, tme, &ToolMenuEntry::runClicked);
	connect(toolMenuItem->getToolBtn(), &QPushButton::clicked, this,
		[=]() { Q_EMIT requestToolSelect(toolMenuItem->getId()); });
	connect(toolMenuItem->getToolBtn(), &QPushButton::toggled, this,
		[this, toolMenuItem](bool on) { selectTool(toolMenuItem, on); });
	connect(toolMenuItem, &ToolMenuItem::doubleclick, this, [this, tme]() { setTmeAttached(tme); });
	connect(tme, &ToolMenuEntry::updateToolEntry, toolMenuItem, &ToolMenuItem::updateItem);

	return toolMenuItem;
}

#include "moc_toolmenumanager.cpp"
