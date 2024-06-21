#include "deviceimpl.h"

#include "logging_categories.h"
#include "pluginbase/preferences.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include "qscrollarea.h"

#include "ui_devicepage.h"

#include <QDebug>
#include <QLabel>
#include <QLoggingCategory>
#include <QTextBrowser>
#include <QThread>
#include <style.h>

#include <common/scopyconfig.h>
#include <widgets/hoverwidget.h>

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "Device")

namespace scopy {
DeviceImpl::DeviceImpl(QString param, PluginManager *p, QString category, QObject *parent)
	: QObject{parent}
	, m_param(param)
	, m_category(category)
	, p(p)
{
	m_id = "dev_" + category + "_" + param + "_" + scopy::config::getUuid();
	qDebug(CAT_DEVICEIMPL) << m_param << "ctor";
}

void DeviceImpl::init()
{
	QElapsedTimer timer;
	timer.start();
	m_plugins = p->getCompatiblePlugins(m_param, m_category);
	for(Plugin *p : qAsConst(m_plugins)) {
		QObject *obj = dynamic_cast<QObject *>(p);
		if(obj) {
			obj->setParent(this);
		} else {
			qWarning(CAT_DEVICEIMPL, "Plugin not a QObject");
		}
	}
	qInfo(CAT_BENCHMARK) << this->displayName() << " init took: " << timer.elapsed() << "ms";
}

void DeviceImpl::preload()
{
	for(auto &p : m_plugins) {
		p->preload();
	}
}

void DeviceImpl::loadPlugins()
{
	QElapsedTimer timer;
	timer.start();
	removeDisabledPlugins();
	preload();
	loadName();
	loadIcons();
	loadBadges();
	loadPages();
	loadToolList();
	if(m_plugins.isEmpty()) {
		connbtn->hide();
	}
	for(auto &p : m_plugins) {
		connect(dynamic_cast<QObject *>(p), SIGNAL(connectDevice()), this, SLOT(connectDev()));
		connect(dynamic_cast<QObject *>(p), SIGNAL(disconnectDevice()), this, SLOT(disconnectDev()));
		connect(dynamic_cast<QObject *>(p), SIGNAL(toolListChanged()), this, SIGNAL(toolListChanged()));
		connect(dynamic_cast<QObject *>(p), SIGNAL(restartDevice()), this, SIGNAL(requestedRestart()));
		connect(dynamic_cast<QObject *>(p), SIGNAL(requestToolByUuid(QString)), this,
			SIGNAL(requestTool(QString)));
		p->postload();
	}
	qInfo(CAT_BENCHMARK) << this->displayName() << " plugins load took: " << timer.elapsed() << "ms";
}

void DeviceImpl::unloadPlugins()
{
	QElapsedTimer timer;
	timer.start();
	QList<Plugin *>::const_iterator pI = m_plugins.constEnd();
	while(pI != m_plugins.constBegin()) {
		--pI;
		disconnect(dynamic_cast<QObject *>(*pI), SIGNAL(connectDevice()), this, SLOT(connectDev()));
		disconnect(dynamic_cast<QObject *>(*pI), SIGNAL(disconnectDevice()), this, SLOT(disconnectDev()));
		disconnect(dynamic_cast<QObject *>(*pI), SIGNAL(toolListChanged()), this, SIGNAL(toolListChanged()));
		disconnect(dynamic_cast<QObject *>(*pI), SIGNAL(restartDevice()), this, SIGNAL(requestedRestart()));
		disconnect(dynamic_cast<QObject *>(*pI), SIGNAL(requestToolByUuid(QString)), this,
			   SIGNAL(requestTool(QString)));
		(*pI)->unload();
		delete(*pI);
	}
	m_plugins.clear();
	qInfo(CAT_BENCHMARK) << this->displayName() << " plugins unload took: " << timer.elapsed() << "ms";
}

bool DeviceImpl::verify() { return true; }

QMap<QString, QString> DeviceImpl::readDeviceInfo()
{
	QMap<QString, QString> map;
	return map;
}

void DeviceImpl::removeDisabledPlugins()
{
	QMutableListIterator<Plugin *> i(m_plugins);
	while(i.hasNext()) {
		if(i.next()->enabled() == false)
			i.remove();
	}
}

void DeviceImpl::loadName()
{
	if(m_plugins.count()) {
		m_displayName = m_plugins[0]->displayName();
		m_displayParam = m_plugins[0]->displayParam();
	} else {
		m_displayName = "NO_PLUGIN";
	}
}

void DeviceImpl::loadIcons()
{
	m_icon = new QWidget();
	m_icon->setFixedHeight(100);
	m_icon->setFixedWidth(100);
	new QHBoxLayout(m_icon);
	for(auto &p : m_plugins) {
		if(p->loadIcon()) {
			m_icon->layout()->addWidget(p->icon());
			return;
		}
	}

	new QLabel("No PLUGIN", m_icon);
}

void DeviceImpl::loadPages()
{
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

	connbtn->setProperty("device_page", true);
	connbtn->setProperty("blue_button", true);
	Style::setStyle(connbtn, style::button::basic);
	connbtn->setAutoDefault(true);
	m_buttonLayout->addWidget(connbtn);

	discbtn->setProperty("device_page", true);
	discbtn->setProperty("blue_button", true);
	Style::setStyle(discbtn, style::button::basic);
	discbtn->setAutoDefault(true);
	m_buttonLayout->addWidget(discbtn);
	discbtn->setVisible(false);

	connect(connbtn, &QPushButton::clicked, this, &DeviceImpl::connectDev);
	connect(discbtn, &QPushButton::clicked, this, &DeviceImpl::disconnectDev);
	connect(this, &DeviceImpl::connectionFailed, this, &DeviceImpl::onConnectionFailed);

	for(auto &&p : plugins()) {
		if(p->loadExtraButtons()) {
			for(auto &&b : p->extraButtons()) {
				b->setProperty("blue_button", true);
				b->setProperty("device_page", true);
				m_buttonLayout->addWidget(b);
			}
		}
	}
	m_buttonLayout->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Expanding));

	for(auto &&p : plugins()) {
		if(p->loadPage()) {
			m_scrollAreaLayout->addWidget(p->page());
		}
	}
}

void DeviceImpl::loadToolList()
{
	for(auto &&p : m_plugins) {
		p->loadToolList();
	}
}

void DeviceImpl::loadBadges()
{
	QPushButton *forgetBtn = new QPushButton();
	forgetBtn->setMaximumSize(25, 25);
	forgetBtn->setIcon(Style::getPixmap(":/gui/icons/orange_close.svg"));
	connect(forgetBtn, &QPushButton::clicked, this, &DeviceImpl::forget);
	HoverWidget *forgetHover = new HoverWidget(forgetBtn, m_icon, m_icon);
	forgetHover->setStyleSheet("background-color: transparent; border: 0px;");
	forgetHover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	forgetHover->setContentPos(HoverPosition::HP_TOPLEFT);
	forgetHover->setVisible(true);
	forgetHover->raise();

	QPushButton *warningBtn = new QPushButton();
	warningBtn->setMaximumSize(25, 25);
	warningBtn->setIcon(Style::getPixmap(":/gui/icons/warning.svg"));
	warningBtn->setToolTip(tr("The device is not available!\n"
				  "Verify the connection!"));
	HoverWidget *warningHover = new HoverWidget(warningBtn, m_icon, m_icon);
	warningHover->setStyleSheet("background-color: transparent; border: 0px;");
	warningHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	warningHover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	warningHover->raise();
	connect(this, &DeviceImpl::connectionFailed, warningHover, &HoverWidget::show);
	connect(this, &DeviceImpl::connected, warningHover, &HoverWidget::hide);
}

void DeviceImpl::setPingPlugin(Plugin *plugin)
{
	if(!m_pingPlugin && plugin->pingTask()) {
		m_pingPlugin = plugin;
	}
}

void DeviceImpl::bindPing()
{
	if(!m_pingPlugin) {
		return;
	}
	for(auto &&p : m_connectedPlugins) {
		connect(dynamic_cast<QObject *>(p), SIGNAL(pausePingTask(bool)), dynamic_cast<QObject *>(m_pingPlugin),
			SLOT(onPausePingTask(bool)));
	}
	connect(m_pingPlugin->pingTask(), &PingTask::pingFailed, this, &DeviceImpl::disconnectDev);
	m_pingPlugin->startPingTask();
}

void DeviceImpl::unbindPing()
{
	if(!m_pingPlugin) {
		return;
	}
	for(auto &&p : m_connectedPlugins) {
		disconnect(dynamic_cast<QObject *>(p), SIGNAL(pausePingTask(bool)),
			   dynamic_cast<QObject *>(m_pingPlugin), SLOT(onPausePingTask(bool)));
	}
	m_pingPlugin->stopPingTask();
	disconnect(m_pingPlugin->pingTask(), &PingTask::pingFailed, this, &DeviceImpl::disconnectDev);
	m_pingPlugin = nullptr;
}

void DeviceImpl::onConnectionFailed() { disconnectDev(); }

QList<Plugin *> DeviceImpl::plugins() const { return m_plugins; }

void DeviceImpl::showPage()
{
	for(auto &&p : m_plugins)
		p->showPageCallback();
	if(connbtn->isHidden()) {
		discbtn->setFocus();
	} else {
		connbtn->setFocus();
	}
}

void DeviceImpl::hidePage()
{
	for(auto &&p : m_plugins)
		p->hidePageCallback();
}

void DeviceImpl::save(QSettings &s)
{
	for(Plugin *p : qAsConst(m_plugins)) {
		s.beginGroup(p->name());
		p->saveSettings(s);
		s.endGroup();
	}
}

void DeviceImpl::load(QSettings &s)
{
	for(Plugin *p : qAsConst(m_plugins)) {
		s.beginGroup(p->name());
		p->loadSettings(s);
		s.endGroup();
	}
}

void DeviceImpl::connectDev()
{
	QElapsedTimer pluginTimer;
	QElapsedTimer timer;
	timer.start();
	Preferences *pref = Preferences::GetInstance();
	bool disconnectDevice = false;
	for(auto &&p : m_plugins) {
		pluginTimer.start();
		bool pluginConnectionSucceeded = p->onConnect();
		qInfo(CAT_BENCHMARK) << p->name() << " connection took: " << pluginTimer.elapsed() << "ms";
		if(pluginConnectionSucceeded) {
			if(pref->get("general_save_session").toBool()) {
				QSettings s = QSettings(scopy::config::settingsFolderPath() + "/" + p->name() + ".ini",
							QSettings::IniFormat);
				p->loadSettings(s);
			}
			m_connectedPlugins.push_back(p);
			setPingPlugin(p);
		} else {
			disconnectDevice = p->metadata().value("disconnectDevOnConnectFailure").toBool();
			if(disconnectDevice) {
				break;
			}
		}
	}
	if(disconnectDevice || m_connectedPlugins.isEmpty()) {
		Q_EMIT connectionFailed();
	} else {
		connbtn->hide();
		discbtn->show();
		discbtn->setFocus();
		bindPing();
		Q_EMIT connected();
	}
	qInfo(CAT_BENCHMARK) << this->displayName() << " device connection took: " << timer.elapsed() << "ms";
}

void DeviceImpl::disconnectDev()
{
	QElapsedTimer pluginTimer;
	QElapsedTimer timer;
	timer.start();
	unbindPing();
	connbtn->show();
	discbtn->hide();
	Preferences *pref = Preferences::GetInstance();
	for(auto &&p : m_connectedPlugins) {
		if(pref->get("general_save_session").toBool()) {
			QSettings s = QSettings(scopy::config::settingsFolderPath() + "/" + p->name() + ".ini",
						QSettings::IniFormat);
			p->saveSettings(s);
		}
		pluginTimer.start();
		p->onDisconnect();
		qInfo(CAT_BENCHMARK) << p->name() << " disconnection took: " << pluginTimer.elapsed() << "ms";
	}
	m_connectedPlugins.clear();
	connbtn->setFocus();
	Q_EMIT disconnected();
	qInfo(CAT_BENCHMARK) << this->displayName() << " device disconnection took: " << timer.elapsed() << "ms";
}

DeviceImpl::~DeviceImpl() { qDebug(CAT_DEVICEIMPL) << m_id << "dtor"; }

QString DeviceImpl::id() { return m_id; }

QString DeviceImpl::displayName() { return m_displayName; }

QString DeviceImpl::category() { return m_category; }

QString DeviceImpl::displayParam() { return m_displayParam; }

QString DeviceImpl::param() { return m_param; }

QWidget *DeviceImpl::icon() { return m_icon; }

QWidget *DeviceImpl::page() { return m_page; }

QList<ToolMenuEntry *> DeviceImpl::toolList()
{
	static int i;
	QList<ToolMenuEntry *> ret;

	for(auto &&p : m_plugins) {
		ret.append(p->toolList());
	}
	return ret;
}

} // namespace scopy

#include "moc_deviceimpl.cpp"
