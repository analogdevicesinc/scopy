/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "deviceimpl.h"

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
#include <QtConcurrent/QtConcurrent>
#include <deviceiconbuilder.h>
#include <pluginrepository.h>
#include <style.h>

#include <common/debugtimer.h>
#include <common/scopyconfig.h>
#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/connectionloadingbar.h>
#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "Device")

namespace scopy {
DeviceImpl::DeviceImpl(QString param, QString category, QObject *parent)
	: QObject{parent}
	, m_param(param)
	, m_category(category)
{
	m_state = DEV_INIT;
	m_id = "dev_" + category + "_" + param + "_" + scopy::config::getUuid();
	qDebug(CAT_DEVICEIMPL) << m_param << "ctor";
}

void DeviceImpl::init()
{
	DebugTimer benchmark;
	m_plugins = PluginRepository::getCompatiblePlugins(m_param, m_category);
	for(Plugin *p : qAsConst(m_plugins)) {
		QObject *obj = dynamic_cast<QObject *>(p);
		if(obj) {
			obj->setParent(this);
		} else {
			qWarning(CAT_DEVICEIMPL, "Plugin not a QObject");
		}
	}
	DEBUGTIMER_LOG(benchmark, "Dev init took:");
}

void DeviceImpl::preload()
{
	for(auto &p : m_plugins) {
		p->preload();
	}
}

void DeviceImpl::loadPlugins()
{
	DebugTimer benchmark;
	removeDisabledPlugins();
	preload();
	loadName();
	loadIcons();
	loadBadges();
	loadPages();
	loadConfigPage();
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
	m_state = DEV_IDLE;
	DEBUGTIMER_LOG(benchmark, this->displayName() + " plugins load took:");
}

void DeviceImpl::unloadPlugins()
{
	DebugTimer benchmark;
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
	DEBUGTIMER_LOG(benchmark, this->displayName() + " plugins unload took:");
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
	QHBoxLayout *lay = new QHBoxLayout(m_icon);
	m_icon->setFixedHeight(100);
	m_icon->setFixedWidth(100);
	for(auto &p : m_plugins) {
		if(p->loadIcon()) {
			lay->addWidget(p->icon());
			return;
		}
	}

	QLabel *header = new QLabel("No Plugin");
	Style::setStyle(header, style::properties::label::deviceIcon, true);

	QWidget *noPluginIcon =
		DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).color("gray").headerWidget(header).build();
	lay->addWidget(noPluginIcon);
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

	Style::setStyle(connbtn, style::properties::button::basicButton);
	connbtn->setAutoDefault(true);
	m_buttonLayout->addWidget(connbtn);

	Style::setStyle(discbtn, style::properties::button::basicButton);
	discbtn->setAutoDefault(true);
	m_buttonLayout->addWidget(discbtn);
	discbtn->setVisible(false);

	connect(connbtn, &QPushButton::clicked, this, &DeviceImpl::connectDev);
	connect(discbtn, &QPushButton::clicked, this, &DeviceImpl::disconnectDev);
	connect(this, &DeviceImpl::connectionFailed, this, &DeviceImpl::onConnectionFailed, Qt::QueuedConnection);

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
			break; // Only display the page from the plugin with the highest priority
		}
	}
}

void DeviceImpl::loadConfigPage()
{
	m_configPage = new QTabWidget();
	m_configPage->setTabPosition(QTabWidget::South);

	for(auto &&p : m_plugins) {
		if(p->loadConfigPage()) {
			m_configPage->addTab(p->configPage(), p->name());
		}
	}
	// If no plugin has a configuration page, the device-level configuration page should not exist
	if(m_configPage->count() == 0) {
		delete m_configPage;
		m_configPage = nullptr;
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
	forgetBtn->setIcon(QPixmap(":/gui/icons/orange_close.svg"));
	connect(forgetBtn, &QPushButton::clicked, this, &DeviceImpl::forget);
	HoverWidget *forgetHover = new HoverWidget(forgetBtn, m_icon, m_icon);
	forgetHover->setStyleSheet("background-color: transparent; border: 0px;");
	forgetHover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	forgetHover->setContentPos(HoverPosition::HP_TOPLEFT);
	forgetHover->setVisible(true);
	forgetHover->raise();

	QPushButton *warningBtn = new QPushButton();
	warningBtn->setMaximumSize(25, 25);
	warningBtn->setIcon(QPixmap(":/gui/icons/warning.svg"));
	warningBtn->setToolTip(tr("The device is not available!\n"
				  "Verify the connection!"));
	HoverWidget *warningHover = new HoverWidget(warningBtn, m_icon, m_icon);
	warningHover->setStyleSheet("background-color: transparent; border: 0px;");
	warningHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	warningHover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	warningHover->raise();
	warningHover->hide();
	connect(this, &DeviceImpl::connectionFailed, warningHover, &HoverWidget::show);
	connect(this, &DeviceImpl::connecting, warningHover, &HoverWidget::hide);
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

void DeviceImpl::onConnectionFailed()
{
	m_state = DEV_ERROR;
	disconnectDev();
}

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
	m_state = DEV_CONNECTING;
	DebugTimer pluginConnBm;
	DebugTimer connectDevBm;
	ConnectionLoadingBar *connectionLoadingBar = new ConnectionLoadingBar();
	connectionLoadingBar->setProgressBarMaximum(m_plugins.size());
	StatusBarManager::pushUrgentWidget(connectionLoadingBar, "Connection Loading Bar");
	connectDevBm.startTimer();
	Preferences *pref = Preferences::GetInstance();
	bool disconnectDevice = false;
	connbtn->hide();
	discbtn->show();
	discbtn->setEnabled(false);
	m_icon->setFocus(); // temporarily set focus somewhere else

	// the device will always signal connecting->connected->disconnecting->disconnected
	// connection process started
	Q_EMIT connecting();
	QCoreApplication::processEvents();
	for(int i = 0; i < m_plugins.size(); ++i) {
		pluginConnBm.startTimer();
		connectionLoadingBar->setCurrentPlugin(m_plugins[i]->name());
		QCoreApplication::processEvents();
		bool pluginConnectionSucceeded = m_plugins[i]->onConnect();
		DEBUGTIMER_LOG(pluginConnBm, m_plugins[i]->name() + " connection took:");
		connectionLoadingBar->addProgress(1); // TODO: might change to better reflect the time
		QCoreApplication::processEvents();
		if(pluginConnectionSucceeded) {
			if(pref->get("general_save_session").toBool()) {
				QSettings s = QSettings(scopy::config::settingsFolderPath() + "/" +
								m_plugins[i]->name() + ".ini",
							QSettings::IniFormat);
				m_plugins[i]->loadSettings(s);
			}
			m_connectedPlugins.push_back(m_plugins[i]);
			setPingPlugin(m_plugins[i]);
		} else {
			QJsonValue obj = m_plugins[i]->metadata().value("disconnectDevOnConnectFailure");
			if(obj != QJsonValue::Undefined) {
				disconnectDevice = obj.toBool();
			} else {
				qWarning(CAT_DEVICEIMPL) << "Undefined json value";
			}

			if(disconnectDevice) {
				break;
			}
		}
	}

	if(disconnectDevice || m_connectedPlugins.isEmpty()) {
		// connectionFailed will trigger deviceDisconnect on a queued connection
		Q_EMIT connectionFailed();
	} else {
		discbtn->setEnabled(true);
		discbtn->setFocus();
		bindPing();
	}
	// connected will be sent regardless of connection result indicating that the process finished
	m_state = DEV_CONNECTED;
	Q_EMIT connected();
	delete connectionLoadingBar;
	DEBUGTIMER_LOG(connectDevBm, this->displayName() + " device connection took:");
}

void DeviceImpl::disconnectDev()
{
	DebugTimer pluginDisconnBm;
	DebugTimer disconnectDevBm;
	m_state = DEV_DISCONNECTING;
	Q_EMIT disconnecting();

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
		pluginDisconnBm.startTimer();
		p->onDisconnect();
		DEBUGTIMER_LOG(pluginDisconnBm, p->name() + " disconnection took:");
	}
	m_connectedPlugins.clear();
	connbtn->setFocus();
	m_state = DEV_IDLE;
	DEBUGTIMER_LOG(disconnectDevBm, this->displayName() + " device disconnection took:");
	Q_EMIT disconnected();
}

DeviceImpl::~DeviceImpl() { qDebug(CAT_DEVICEIMPL) << m_id << "dtor"; }

QString DeviceImpl::id() { return m_id; }

QString DeviceImpl::displayName() { return m_displayName; }

QString DeviceImpl::category() { return m_category; }

QString DeviceImpl::displayParam() { return m_displayParam; }

QString DeviceImpl::param() { return m_param; }

QWidget *DeviceImpl::icon() { return m_icon; }

QPixmap DeviceImpl::iconPixmap()
{
	QPixmap pixmap;
	QLayoutItem *item = m_icon->layout()->itemAt(0);
	if(!item || !item->widget()) {
		return pixmap;
	}
	QLabel *iconLabel = dynamic_cast<QLabel *>(item->widget());
	if(iconLabel) {
		pixmap = iconLabel->grab();
	}
	return pixmap;
}

QWidget *DeviceImpl::configPage() { return m_configPage; }

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

DeviceImpl::DeviceState_t DeviceImpl::state() { return m_state; }

} // namespace scopy

#include "moc_deviceimpl.cpp"
