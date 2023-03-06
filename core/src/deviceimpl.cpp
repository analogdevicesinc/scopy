#include "deviceimpl.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include <QLabel>
#include <QTextBrowser>
#include <QLoggingCategory>
#include <QDebug>
#include <QThread>

#include "scopycore_config.h"
#ifdef ENABLE_SCOPYJS
#include "scopyjs/scopyjs.h"
#endif

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "Device")

namespace adiscope {
DeviceImpl::DeviceImpl(QString param, PluginManager *p, QString category ,QObject *parent)
	: QObject{parent},
	  m_param(param),
	  m_category(category),
	  p(p)
{
	m_id = QUuid::createUuid().toString();
	qDebug(CAT_DEVICEIMPL)<< m_param <<"ctor";
}

void DeviceImpl::loadCompatiblePlugins()
{
	plugins = p->getCompatiblePlugins(m_param,m_category,this);
}

void DeviceImpl::compatiblePreload() {
	for(auto &p : plugins) {
		p->preload();
	}
}


void DeviceImpl::loadPlugins() {

	loadName();
	loadIcons();
	loadPages();
	loadToolList();

	for(auto &p : plugins) {
		connect(dynamic_cast<QObject*>(p),SIGNAL(disconnectDevice()),this,SLOT(disconnectDev()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(toolListChanged()),this,SIGNAL(toolListChanged()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(restartDevice()),this,SIGNAL(requestedRestart()));
		connect(dynamic_cast<QObject*>(p),SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
		p->loadApi();
		p->postload();
	}
}

void DeviceImpl::unloadPlugins() {
	QList<Plugin*>::const_iterator pI = plugins.constEnd();
	while(pI != plugins.constBegin()) {
		--pI;
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(disconnectDevice()),this,SLOT(disconnectDev()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(toolListChanged()),this,SIGNAL(toolListChanged()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(restartDevice()),this,SIGNAL(requestedRestart()));
		disconnect(dynamic_cast<QObject*>(*pI),SIGNAL(requestTool(QString)),this,SIGNAL(requestTool(QString)));
		(*pI)->unload();
		delete (*pI);
	}
	plugins.clear();
}

void DeviceImpl::loadName() {
	if(plugins.count()) {
		m_name = plugins[0]->name();
		m_description = plugins[0]->uri();
	} else {
		m_name = "NO_PLUGIN";
	}
}

void DeviceImpl::loadIcons() {
	m_icon = new QWidget();
	m_icon->setFixedHeight(100);
	m_icon->setFixedWidth(100);
	new QHBoxLayout(m_icon);

	for( auto &p : plugins) {
		if(p->loadIcon()) {
			m_icon->layout()->addWidget(p->icon());
			return;
		}
	}

	new QLabel("No PLUGIN",m_icon);
}

void DeviceImpl::loadPages() {
	m_page = new QWidget();
	auto m_buttonLayout = new QHBoxLayout();
	auto m_pagelayout = new QVBoxLayout(m_page);
	connbtn = new QPushButton("connect", m_page);
	connbtn->setProperty("blue_button",true);
	m_buttonLayout->addWidget(connbtn);
	discbtn = new QPushButton("disconnect", m_page);
	discbtn->setProperty("blue_button",true);
	m_buttonLayout->addWidget(discbtn);
	discbtn->setVisible(false);

	connect(connbtn,SIGNAL(clicked()),this,SLOT(connectDev()));
	connect(discbtn,SIGNAL(clicked()),this,SLOT(disconnectDev()));
	m_pagelayout->addLayout(m_buttonLayout);

	for(auto &&p : plugins) {
		if(p->loadExtraButtons()) {
			for(auto &&b : p->extraButtons()) {
				b->setProperty("blue_button", true);
				m_buttonLayout->addWidget(b);
			}
		}
	}

	for(auto &&p : plugins) {
		if(p->loadPage()) {
			m_pagelayout->addWidget(p->page());
		}
	}
}

void DeviceImpl::loadToolList() {
	for(auto &&p : plugins) {
		p->loadToolList();
	}
}

void DeviceImpl::showPage() {
	for(auto &&p : plugins)
		p->showPageCallback();

}

void DeviceImpl::hidePage() {
	for(auto &&p : plugins)
		p->hidePageCallback();

}

void DeviceImpl::save(QSettings &s) {
	for(Plugin* p : qAsConst(plugins)) {
		p->saveSettings(s);
	}
}

void DeviceImpl::load(QSettings &s) {
	for(Plugin* p : qAsConst(plugins)) {
		p->loadSettings(s);
	}
}

void DeviceImpl::connectDev() {
	discbtn->show();
	connbtn->hide();
	for(auto &&p : plugins) {
		p->onConnect();
		p->loadSettings();
#ifdef ENABLE_SCOPYJS
		if(p->api())
			ScopyJS::GetInstance()->registerApi(p->api());
#endif
	}
	Q_EMIT connected();
}

void DeviceImpl::disconnectDev() {
	discbtn->hide();
	connbtn->show();
	for(auto &&p : plugins) {

#ifdef ENABLE_SCOPYJS
		if(p->api())
			ScopyJS::GetInstance()->unregisterApi(p->api());
#endif
		p->saveSettings();
		p->onDisconnect();
	}
	Q_EMIT disconnected();
}

DeviceImpl::~DeviceImpl() {

	qDebug(CAT_DEVICEIMPL)<< m_id <<"dtor";
}

QString DeviceImpl::id()
{
	return m_id;
}

QString DeviceImpl::name()
{
	return m_name;
}

QString DeviceImpl::category()
{
	return m_category;
}

QString DeviceImpl::description()
{
	return m_description;
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

	for(auto &&p : plugins) {
		ret.append(p->toolList());
	}
	return ret;
}
}
