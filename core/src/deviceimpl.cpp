#include "deviceimpl.h"
#include "pluginbase/preferences.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include <QLabel>
#include <QTextBrowser>
#include <QLoggingCategory>
#include <QDebug>
#include <QThread>
#include <common/scopyconfig.h>
#include "qscrollarea.h"
#include "ui_devicepage.h"

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "Device")

namespace scopy {
DeviceImpl::DeviceImpl(QString param, PluginManager *p, QString category ,QObject *parent)
	: QObject{parent},
	  m_param(param),
	  m_category(category),
	  p(p)
{
	m_id = "dev_"+category+"_"+param+"_"+scopy::config::getUuid();
	qDebug(CAT_DEVICEIMPL)<< m_param <<"ctor";
}

void DeviceImpl::init()
{
	m_plugins = p->getCompatiblePlugins(m_param,m_category);
	for (Plugin *p : qAsConst(m_plugins)) {
		QObject* obj = dynamic_cast<QObject*>(p);
		if(obj) {
			obj->setParent(this);
		} else {
			qWarning(CAT_DEVICEIMPL, "Plugin not a QObject");
		}
	}
}

void DeviceImpl::preload() {
	for(auto &p : m_plugins) {
		p->preload();
	}
}

void DeviceImpl::loadPlugins() {
	removeDisabledPlugins();
	preload();
	loadName();
	loadIcons();
	loadWarningBadge();
	loadPages();
	loadToolList();
	if (m_plugins.isEmpty()) {
		connbtn->hide();
	}
	for(auto &p : m_plugins) {
		connect(dynamic_cast<QObject*>(p),SIGNAL(connectDevice()),this,SLOT(connectDev()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(disconnectDevice()),this,SLOT(disconnectDev()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(toolListChanged()),this,SIGNAL(toolListChanged()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(restartDevice()),this,SIGNAL(requestedRestart()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(requestToolByUuid(QString)),this,SIGNAL(requestTool(QString)));
		p->postload();
	}
}

void DeviceImpl::unloadPlugins() {
	QList<Plugin*>::const_iterator pI = m_plugins.constEnd();
	while(pI != m_plugins.constBegin()) {
		--pI;
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(connectDevice()),this,SLOT(connectDev()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(disconnectDevice()),this,SLOT(disconnectDev()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(toolListChanged()),this,SIGNAL(toolListChanged()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(restartDevice()),this,SIGNAL(requestedRestart()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
		(*pI)->unload();
		delete (*pI);
	}
	m_plugins.clear();
}

void DeviceImpl::removeDisabledPlugins() {
	QMutableListIterator<Plugin*> i(m_plugins);
	while (i.hasNext()) {
		if (i.next()->enabled() == false)
			i.remove();
	}
}

void DeviceImpl::loadName() {
	if(m_plugins.count()) {
		m_displayName = m_plugins[0]->displayName();
		m_displayParam = m_plugins[0]->displayParam();
	} else {
		m_displayName = "NO_PLUGIN";
	}
}

void DeviceImpl::loadIcons() {
	m_icon = new QWidget();
	m_icon->setFixedHeight(100);
	m_icon->setFixedWidth(100);
	new QHBoxLayout(m_icon);
	for( auto &p : m_plugins) {
		if(p->loadIcon()) {
			m_icon->layout()->addWidget(p->icon());
			return;
		}
	}

	new QLabel("No PLUGIN",m_icon);
}

void DeviceImpl::loadPages() {
	m_page = new QWidget();
	Ui::DevicePage *ui = new Ui::DevicePage();
	ui->setupUi(m_page);

	m_page->setProperty("device_page", true);
	connbtn = new QPushButton("Connect", m_page);
	discbtn = new QPushButton("Disconnect", m_page);
	auto m_buttonLayout = ui->m_buttonLayout;
	auto m_scrollArea = ui->m_scrollArea;
	auto m_scrollAreaContents = ui->m_scrollAreaContents;
	auto m_scrollAreaLayout = ui->m_scrollAreaLayout;

	connbtn->setProperty("device_page",true);
	connbtn->setProperty("blue_button",true);
	m_buttonLayout->addWidget(connbtn);

	discbtn->setProperty("device_page",true);
	discbtn->setProperty("blue_button",true);
	m_buttonLayout->addWidget(discbtn);
	discbtn->setVisible(false);

	connect(connbtn,&QPushButton::clicked,this,&DeviceImpl::connectDev);
	connect(discbtn,&QPushButton::clicked,this,&DeviceImpl::disconnectDev);
	connect(this, &DeviceImpl::connectionFailed, this, &DeviceImpl::onConnectionFailed);

	for(auto &&p : plugins()) {
		if(p->loadExtraButtons()) {
			for(auto &&b : p->extraButtons()) {
				b->setProperty("blue_button", true);
				b->setProperty("device_page",true);
				m_buttonLayout->addWidget(b);
			}
		}
	}
	m_buttonLayout->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding));

	for(auto &&p : plugins()) {
		if(p->loadPage()) {
			m_scrollAreaLayout->addWidget(p->page());
		}
	}
}

void DeviceImpl::loadToolList() {
	for(auto &&p : m_plugins) {
		p->loadToolList();
	}
}

void DeviceImpl::onConnectionFailed()
{
	m_warningBtn->show();
	disconnectDev();
}

void DeviceImpl::loadWarningBadge()
{
	m_warningBtn = new QPushButton(QString(), m_icon);
	m_warningBtn->installEventFilter(this);
	m_warningBtn->setObjectName(QString::fromUtf8("btnHelp"));
	m_warningBtn->setCheckable(false);
	m_warningBtn->setText(QString());
	m_warningBtn->setStyleSheet("background-color: transparent; border: 0px");
	QIcon warningIcon = QIcon(":/gui/icons/warning.svg");
	m_warningBtn->setIcon(warningIcon);
	m_warningBtn->setMaximumSize(25,25);
	m_warningBtn->move(80,0);
	m_warningBtn->raise();
	m_warningBtn->hide();
}

QList<Plugin *> DeviceImpl::plugins() const
{
	return m_plugins;
}

void DeviceImpl::showPage() {
	for(auto &&p : m_plugins)
		p->showPageCallback();

}

void DeviceImpl::hidePage() {
	for(auto &&p : m_plugins)
		p->hidePageCallback();

}

void DeviceImpl::save(QSettings &s) {
	for(Plugin* p : qAsConst(m_plugins)) {
		s.beginGroup(p->name());
		p->saveSettings(s);
		s.endGroup();
	}
}

void DeviceImpl::load(QSettings &s) {
	for(Plugin* p : qAsConst(m_plugins)) {
		s.beginGroup(p->name());
		p->loadSettings(s);
		s.endGroup();
	}
}

void DeviceImpl::connectDev() {
	Preferences *pref = Preferences::GetInstance();
	bool disconnectDevice = false;
	for(auto &&p : m_plugins) {
		bool pluginConnectionSucceeded = p->onConnect();
		if (pluginConnectionSucceeded) {
			if(pref->get("general_save_session").toBool()) {
				QSettings s = QSettings(scopy::config::settingsFolderPath() + "/" +p->name() +".ini", QSettings::IniFormat);
				p->loadSettings(s);
			}
			m_connectedPlugins.push_back(p);
		} else {
			disconnectDevice = p->metadata().value("disconnectDevOnConnectFailure").toBool();
			if (disconnectDevice) {
				break;
			}
		}
	}
	if (disconnectDevice || m_connectedPlugins.isEmpty()) {
		Q_EMIT connectionFailed();
	} else {
		connbtn->hide();
		discbtn->show();
		m_warningBtn->hide();
		Q_EMIT connected();
	}
}

void DeviceImpl::disconnectDev() {
	connbtn->show();
	discbtn->hide();
	Preferences *pref = Preferences::GetInstance();
	for(auto &&p : m_plugins) {
		if(m_connectedPlugins.contains(p) && pref->get("general_save_session").toBool()) {
			QSettings s = QSettings(scopy::config::settingsFolderPath() + "/" +p->name() +".ini", QSettings::IniFormat);
			p->saveSettings(s);
		}
		p->onDisconnect();
	}
	m_connectedPlugins.clear();
	Q_EMIT disconnected();
}

DeviceImpl::~DeviceImpl() {

	qDebug(CAT_DEVICEIMPL)<< m_id <<"dtor";
}

QString DeviceImpl::id()
{
	return m_id;
}

QString DeviceImpl::displayName()
{
	return m_displayName;
}

QString DeviceImpl::category()
{
	return m_category;
}

QString DeviceImpl::displayParam()
{
	return m_displayParam;
}


QString DeviceImpl::param()
{
	return m_param;
}

QWidget *DeviceImpl::icon()
{
	return m_icon;
}

QWidget *DeviceImpl::page()
{
	return m_page;
}

QList<ToolMenuEntry*> DeviceImpl::toolList()
{
	static int i;
	QList<ToolMenuEntry*> ret;

	for(auto &&p : m_plugins) {
		ret.append(p->toolList());
	}
	return ret;
}

// WIP: to be move in DeviceIconImpl
bool DeviceImpl::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == (QObject*)m_warningBtn) {
		if (event->type() == QEvent::Enter)
		{
			m_warningBtn->setToolTip("The device is not available!\n"
						 "Verify the connection!");
		}
		return false;
	} else {
		return QObject::eventFilter(obj, event);
	}
}

}

#include "moc_deviceimpl.cpp"
