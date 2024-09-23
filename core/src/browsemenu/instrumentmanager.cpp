#include "browsemenu/instrumentmanager.h"
#include <browsemenu/instrumentwidget.h>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>
#include <QButtonGroup>
#include <baseheaderwidget.h>
#include <menuheader.h>

Q_LOGGING_CATEGORY(CAT_INSTR_MANAGER, "InstrumentManager")
using namespace scopy;

InstrumentManager::InstrumentManager(ToolStack *ts, DetachedToolWindowManager *dtm, InstrumentMenu *instrumentMenu,
				     QObject *parent)
	: QObject(parent)
	, m_ts(ts)
	, m_dtm(dtm)
	, m_instrumentMenu(instrumentMenu)
{}

InstrumentManager::~InstrumentManager() {}

void InstrumentManager::addMenuItem(QString deviceId, DeviceInfo devInfo, QList<ToolMenuEntry *> tools, int itemIndex)
{
	MenuSectionCollapseWidget *devSection =
		new MenuSectionCollapseWidget(devInfo.name, MenuCollapseSection::MHCW_ARROW, m_instrumentMenu);
	devSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	createMenuSectionLabel(devSection, devInfo.uri);
	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	for(ToolMenuEntry *tme : tools) {
		InstrumentWidget *instrWidget = createInstrWidget(tme, devSection);
		devSection->add(instrWidget);
		menuBtnGroup->addButton(instrWidget->getToolBtn());
		connect(tme, &ToolMenuEntry::updateTool, this, &InstrumentManager::updateTool);
		connect(tme, &ToolMenuEntry::updateToolAttached, this,
			[this, instrWidget](bool oldAttach) { updateToolAttached(oldAttach, instrWidget); });
		Q_EMIT tme->updateToolEntry();
	}
	m_instrumentMenu->add(itemIndex, deviceId, devSection);
	m_itemMap[deviceId] = devSection;
	m_devInfoMap[deviceId] = devInfo;
	devSection->hide();
}

void InstrumentManager::removeMenuItem(QString deviceId)
{
	if(!m_itemMap.contains(deviceId)) {
		qInfo(CAT_INSTR_MANAGER) << "No entry with id:" << deviceId;
		return;
	}
	MenuSectionCollapseWidget *devSection = m_itemMap[deviceId];
	m_itemMap.remove(deviceId);
	m_instrumentMenu->remove(devSection);
	m_devInfoMap.remove(deviceId);
	delete devSection;
	devSection = nullptr;
}

void InstrumentManager::changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools)
{
	if(!m_itemMap.contains(deviceId)) {
		qInfo(CAT_INSTR_MANAGER) << "No entry with id:" << deviceId;
		return;
	}
	for(ToolMenuEntry *tme : tools) {
		tme->disconnect(this);
	}
	int itemIndex = m_instrumentMenu->indexOf(m_itemMap[deviceId]);
	DeviceInfo devInfo = m_devInfoMap[deviceId];
	removeMenuItem(deviceId);
	addMenuItem(deviceId, devInfo, tools, itemIndex);
	showMenuItem(deviceId);
}

void InstrumentManager::showMenuItem(QString id)
{
	if(!m_connectedDev.contains(m_prevItem))
		hideMenuItem(m_prevItem);
	if(!m_itemMap.contains(id))
		return;
	m_itemMap[id]->show();
	m_prevItem = id;
}

void InstrumentManager::hideMenuItem(QString id)
{
	if(!m_itemMap.contains(id))
		return;
	m_itemMap[id]->hide();
}

void InstrumentManager::updateTool(QWidget *old)
{

	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QString id = tme->uuid();
	QWidget *tool = tme->tool();

	if(old != nullptr) { // we had a widget
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
	qDebug(CAT_INSTR_MANAGER) << "updating tool for " << tme->name() << " - " << id;
}

void InstrumentManager::updateToolAttached(bool oldAttach, InstrumentWidget *instrWidget)
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
		attachSuccesful(instrWidget);
	} else {
		// tool is attached, it will detach
		if(m_ts->contains(id)) {
			m_ts->remove(id);
		}
		m_dtm->add(id, tme);
		detachSuccesful(instrWidget);
	}
	// by this time, the tool has changed it's state, either from attached to detached, or from detached to attached
	saveToolAttachedState(tme);
	showTool(instrWidget);

	// highlight the current tool from the main window
	if(tme->attached()) {
		// the selected tool just attached, so it will be at the top of the stack, therefore highlighted
		if(instrWidget) {
			instrWidget->getToolBtn()->setChecked(true);
			instrWidget->setSelected(true);
		}
	} else {
		// the top tool just detached, so we need to find the tool that is positioned at the new top of the
		// stack.
		if(instrWidget) {
			instrWidget->getToolBtn()->toggle();
		}
	}
}

void InstrumentManager::saveToolAttachedState(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;
	prefId = tme->id() + "_attached";
	bool attach = tme->attached();
	p->set(prefId, attach);
}

void InstrumentManager::detachSuccesful(InstrumentWidget *instrWidget)
{
	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	if(instrWidget) {
		instrWidget->setSelected(false);
		menuBtnGroup->removeButton(instrWidget->getToolBtn());
	}
}

void InstrumentManager::attachSuccesful(InstrumentWidget *instrWidget)
{
	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	if(instrWidget) {
		menuBtnGroup->addButton(instrWidget->getToolBtn());
	}
}

void InstrumentManager::showTool(InstrumentWidget *instrWidget)
{
	if(instrWidget) {
		instrWidget->getToolBtn()->setChecked(true);
	}
	Q_EMIT requestToolSelect(instrWidget->getId());
}

void InstrumentManager::selectInstrument(InstrumentWidget *instrWidget, bool on)
{
	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	if(menuBtnGroup->id(instrWidget->getToolBtn()) != -1) {
		instrWidget->setSelected(on);
	}
}

void InstrumentManager::setTmeAttached(ToolMenuEntry *tme)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_doubleclick_attach").toBool())
		return;
	tme->setAttached(!tme->attached());
}

void InstrumentManager::createMenuSectionLabel(MenuSectionCollapseWidget *section, QString uri)
{
	MenuCollapseHeader *collapseHeader = dynamic_cast<MenuCollapseHeader *>(section->collapseSection()->header());
	if(!collapseHeader) {
		return;
	}
	BaseHeaderWidget *headerWidget = dynamic_cast<BaseHeaderWidget *>(collapseHeader->headerWidget());
	if(headerWidget) {
		QLabel *uriLabel = new QLabel(uri, headerWidget);
		headerWidget->layout()->addWidget(uriLabel);
	}
}

InstrumentWidget *InstrumentManager::createInstrWidget(ToolMenuEntry *tme, QWidget *parent)
{
	InstrumentWidget *instrWidget = new InstrumentWidget(tme->uuid(), tme->name(), tme->icon(), parent);
	connect(instrWidget->getToolRunBtn(), &QPushButton::toggled, tme, &ToolMenuEntry::runToggled);
	connect(instrWidget->getToolRunBtn(), &QPushButton::clicked, tme, &ToolMenuEntry::runClicked);
	connect(instrWidget->getToolBtn(), &QPushButton::clicked, this,
		[=]() { Q_EMIT requestToolSelect(instrWidget->getId()); });
	connect(instrWidget->getToolBtn(), &QPushButton::toggled, this,
		[this, instrWidget](bool on) { selectInstrument(instrWidget, on); });
	connect(instrWidget, &InstrumentWidget::doubleclick, this, [this, tme]() { setTmeAttached(tme); });
	connect(tme, &ToolMenuEntry::updateToolEntry, instrWidget, &InstrumentWidget::updateItem);

	return instrWidget;
}

void InstrumentManager::loadToolAttachedState(ToolMenuEntry *tme)
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
void InstrumentManager::deviceConnected(QString id)
{
	m_connectedDev.append(id);
	showMenuItem(id);
}

void InstrumentManager::deviceDisconnected(QString id)
{
	m_connectedDev.removeAll(id);
	if(m_prevItem.compare(id) != 0) {
		hideMenuItem(id);
	}
}

void InstrumentManager::onDisplayNameChanged(QString id, QString devName)
{
	m_devInfoMap[id].name = devName;
	m_itemMap[id]->collapseSection()->setTitle(devName);
}
