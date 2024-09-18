#include "browsemenu/instrumentmanager.h"
#include <browsemenu/instrumentwidget.h>
#include <QLoggingCategory>
#include <QButtonGroup>

Q_LOGGING_CATEGORY(CAT_INSTR_MANAGER, "InstrumentManager")
using namespace scopy;

InstrumentManager::InstrumentManager(ToolStack *ts, InstrumentMenu *instrumentMenu, QObject *parent)
	: QObject(parent)
	, m_ts(ts)
	, m_instrumentMenu(instrumentMenu)
{}

InstrumentManager::~InstrumentManager() {}

void InstrumentManager::addMenuItem(QString deviceId, QString device, QList<ToolMenuEntry *> tools)
{
	MenuSectionCollapseWidget *devSection =
		new MenuSectionCollapseWidget(device, MenuCollapseSection::MHCW_ARROW, m_instrumentMenu);

	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	for(ToolMenuEntry *tme : tools) {
		InstrumentWidget *instrWidget = createInstrWidget(tme, devSection);
		devSection->add(instrWidget);
		menuBtnGroup->addButton(instrWidget->getToolBtn());
		connect(tme, &ToolMenuEntry::updateTool, this, &InstrumentManager::updateTool);
		Q_EMIT tme->updateToolEntry();
	}
	m_instrumentMenu->add(deviceId, devSection);
	m_itemMap[deviceId] = devSection;
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
	QString deviceName = m_itemMap[deviceId]->collapseSection()->title();
	removeMenuItem(deviceId);
	addMenuItem(deviceId, deviceName, tools);
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
	}
	if(tool != nullptr) { // we have a new widget
		if(tme->attached()) {
			m_ts->add(id, tool);
		}
	}
	qDebug(CAT_INSTR_MANAGER) << "updating tool for " << tme->name() << " - " << id;
}

void InstrumentManager::selectInstrument(InstrumentWidget *instrWidget, bool on)
{
	QButtonGroup *menuBtnGroup = m_instrumentMenu->btnGroup();
	if(menuBtnGroup->id(instrWidget->getToolBtn()) != -1) {
		instrWidget->setSelected(on);
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
	connect(tme, &ToolMenuEntry::updateToolEntry, instrWidget, &InstrumentWidget::updateItem);

	return instrWidget;
}

void InstrumentManager::deviceConnected(QString id)
{
	m_connectedDev.append(id);
	showMenuItem(id);
}

void InstrumentManager::deviceDisconnected(QString id) { m_connectedDev.removeAll(id); }
