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

#include "swiotplugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <stylehelper.h>
#include <menusectionwidget.h>
#include <iioutil/cmdqpingtask.h>

#include "swiot_logging_categories.h"
#include "max14906/max14906.h"
#include "faults/faults.h"
#include "ad74413r/ad74413r.h"

#include <iioutil/connectionprovider.h>

#include <src/config/swiotconfig.h>

#include <pluginbase/preferences.h>
#include <pluginbase/statusbarmanager.h>

using namespace scopy::swiot;

bool SWIOTPlugin::compatible(QString m_param, QString category)
{
	m_name = "SWIOT1L";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_SWIOT) << "No context available for swiot";
		return false;
	}

	iio_device *swiotDevice = iio_context_find_device(conn->context(), "swiot");
	if(swiotDevice) {
		ret = true;
	}

	ConnectionProvider::close(m_param);

	return ret;
}

void SWIOTPlugin::preload()
{
	m_displayName = "SWIOT1L";
	m_swiotController = new SwiotController(m_param, this);
	m_statusContainer = nullptr;
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this, &SWIOTPlugin::onIsRuntimeCtxChanged);
	connect(m_swiotController, &SwiotController::modeAttributeChanged, this, &SWIOTPlugin::onModeAttributeChanged);
	connect(m_swiotController, &SwiotController::writeModeFailed, m_swiotController,
		&SwiotController::disconnectSwiot);
}

bool SWIOTPlugin::loadPage()
{
	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new swiot::SwiotInfoPage(m_page);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(m_swiotController, &SwiotController::readTemperature, this, [=, this](double temperature) {
		if(m_isRuntime) {
			m_infoPage->update("Temperature", QString::number(temperature));
		}
	});
	connect(m_infoPage, &SwiotInfoPage::temperatureReadEnabled, this, [=, this](bool toggled) {
		if(toggled) {
			m_swiotController->startTemperatureTask();
		} else {
			m_swiotController->stopTemperatureTask();
		}
	});

	Connection *conn = ConnectionProvider::open(m_param);

	ssize_t attributeCount = iio_context_get_attrs_count(conn->context());
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(conn->context(), i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_SWIOT) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}

	ConnectionProvider::close(m_param);
	m_page->ensurePolished();

	return true;
}

bool SWIOTPlugin::loadExtraButtons()
{
	m_btnIdentify = new QPushButton("Identify");
	m_extraButtons.append(m_btnIdentify);
	connect(m_btnIdentify, SIGNAL(clicked()), m_swiotController, SLOT(identify()));

	return true;
}

bool SWIOTPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/swiot/swiot_icon.svg);");
	return true;
}

void SWIOTPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(CONFIG_TME_ID, "Config", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(AD74413R_TME_ID, "AD74413R",
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(MAX14906_TME_ID, "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(FAULTS_TME_ID, "Faults", ":/swiot/tool_faults.svg"));
}

void SWIOTPlugin::unload()
{
	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this, &SWIOTPlugin::onIsRuntimeCtxChanged);
	disconnect(m_swiotController, &SwiotController::modeAttributeChanged, this,
		   &SWIOTPlugin::onModeAttributeChanged);
	disconnect(m_swiotController, &SwiotController::writeModeFailed, m_swiotController,
		   &SwiotController::disconnectSwiot);
	delete m_infoPage;
	delete m_swiotController;
}

bool SWIOTPlugin::onConnect()
{

	Connection *conn = ConnectionProvider::open(m_param);
	if(!conn) {
		return false;
	}

	m_pingTask = new CmdQPingTask(conn, "sw_trig", this);
	m_cyclicalTask = new CyclicalTask(m_pingTask);

	m_runtime = new SwiotRuntime(m_param, this);
	connect(m_runtime, &SwiotRuntime::writeModeAttribute, this, &SWIOTPlugin::setCtxMode);
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		&SwiotRuntime::onIsRuntimeCtxChanged);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, this, &SWIOTPlugin::powerSupplyStatus);

	m_swiotController->connectSwiot();
	m_swiotController->readModeAttribute();
	m_swiotController->startPowerSupplyTask("ext_psu");

	return true;
}

bool SWIOTPlugin::onDisconnect()
{
	auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);
	auto ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	auto max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	auto faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setRunning(false);
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->tool()->deleteLater();
		tme->setTool(nullptr);
	}

	disconnect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::writeModeAttribute, this,
		   &SWIOTPlugin::setCtxMode);
	disconnect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::configBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::configBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);

	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, this, &SWIOTPlugin::powerSupplyStatus);

	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		   &SwiotRuntime::onIsRuntimeCtxChanged);

	m_swiotController->stopPowerSupplyTask();
	m_swiotController->stopTemperatureTask();
	m_swiotController->disconnectSwiot();

	if(m_runtime) {
		disconnect(m_runtime, &SwiotRuntime::writeModeAttribute, this, &SWIOTPlugin::setCtxMode);
		delete m_runtime;
		m_runtime = nullptr;
	}

	if(m_statusContainer) {
		delete m_statusContainer;
		m_statusContainer = nullptr;
	}
	clearPingTask();
	ConnectionProvider::close(m_param);

	if(m_switchCmd) {
		m_switchCmd = false;
		switchCtx();
	}

	return true;
}

void SWIOTPlugin::startPingTask() { m_cyclicalTask->start(PING_PERIOD); }

void SWIOTPlugin::stopPingTask() { m_cyclicalTask->stop(); }

void SWIOTPlugin::onPausePingTask(bool pause)
{
	if(pause) {
		stopPingTask();
	} else {
		startPingTask();
	}
}

void SWIOTPlugin::onIsRuntimeCtxChanged(bool isRuntimeCtx)
{
	m_isRuntime = isRuntimeCtx;
	setupToolList();
}

void SWIOTPlugin::powerSupplyStatus(bool ps)
{
	if(!ps) {
		if(!m_statusContainer) {
			createStatusContainer();
			StatusBarManager::pushWidget(m_statusContainer, "ExtPsuStatus");
		}
	} else {
		if(m_statusContainer) {
			delete m_statusContainer;
			m_statusContainer = nullptr;
		}
	}
}

void SWIOTPlugin::setCtxMode(QString mode)
{
	m_ctxMode = mode;
	m_switchCmd = true;
	Q_EMIT disconnectDevice();
}

void SWIOTPlugin::onModeAttributeChanged(QString mode)
{
	if(m_ctxMode.contains(mode)) {
		m_swiotController->disconnectSwiot();
		Q_EMIT connectDevice();
	}
}

void SWIOTPlugin::switchCtx()
{
	m_swiotController->connectSwiot();
	m_swiotController->writeModeAttribute(m_ctxMode);
}

void SWIOTPlugin::createStatusContainer()
{
	m_statusContainer = new QWidget();
	m_statusContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0, 0, 0, 0);

	auto exclamationIcon = new QPushButton(m_statusContainer);
	StyleHelper::NoBackgroundIconButton(exclamationIcon, QIcon::fromTheme(":/swiot/warning.svg"));

	auto statusLabel = new QLabel("AD-SWIOT1L-SL: The system is powered at limited capacity.");
	statusLabel->setWordWrap(true);
	StyleHelper::WarningLabel(statusLabel, "extPsuStatusLabel");

	m_statusContainer->layout()->addWidget(exclamationIcon);
	m_statusContainer->layout()->addWidget(statusLabel);
}

void SWIOTPlugin::setupToolList()
{
	m_infoPage->enableTemperatureReadBtn(m_isRuntime);
	m_swiotController->startTemperatureTask();

	auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);
	auto ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	auto max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	auto faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(true);
		tme->setVisible(true);
		if(!m_isRuntime) {
			if(tme->id().compare(CONFIG_TME_ID)) {
				tme->setVisible(false);
			}
		} else {
			if(tme->id().compare(CONFIG_TME_ID)) {
				tme->setRunBtnVisible(true);
				tme->setRunning(false);
			} else {
				tme->setVisible(false);
			}
		}
	}

	if(m_isRuntime) {
		ad74413rTme->setTool(new swiot::Ad74413r(m_param, ad74413rTme));
		max14906Tme->setTool(new swiot::Max14906(m_param, max14906Tme));
		faultsTme->setTool(new swiot::Faults(m_param, faultsTme));
	} else {
		configTme->setTool(new swiot::SwiotConfig(m_param));
	}

	connect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::writeModeAttribute, this,
		&SWIOTPlugin::setCtxMode);

	connect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::configBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::configBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
}

void SWIOTPlugin::clearPingTask()
{
	if(m_cyclicalTask) {
		m_cyclicalTask->deleteLater();
		m_cyclicalTask = nullptr;
	}
	if(m_pingTask) {
		m_pingTask->deleteLater();
		m_pingTask = nullptr;
	}
}

QString SWIOTPlugin::description() { return "Adds functionality specific to SWIOT1L board"; }

void SWIOTPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("ad74413r_start_tutorial", true);
	p->init("max14906_start_tutorial", true);
	p->init("faults_start_tutorial", true);
	p->init("swiote_config_start_tutorial", true);
}

bool SWIOTPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->setMargin(0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QWidget *resetTutorialAd74413rWidget = new QWidget();
	QHBoxLayout *resetTutorialAd74413rWidgetLayout = new QHBoxLayout();

	resetTutorialAd74413rWidget->setLayout(resetTutorialAd74413rWidgetLayout);
	resetTutorialAd74413rWidgetLayout->setMargin(0);

	QPushButton *resetTutorialAd74413r = new QPushButton("Reset", generalSection);
	connect(resetTutorialAd74413r, &QPushButton::clicked, this,
		[=, this]() { p->set("ad74413r_start_tutorial", true); });

	QWidget *resetTutorialMax14906Widget = new QWidget();
	QHBoxLayout *resetTutorialMax14906WidgetLayout = new QHBoxLayout();

	resetTutorialMax14906Widget->setLayout(resetTutorialMax14906WidgetLayout);
	resetTutorialMax14906WidgetLayout->setMargin(0);

	QPushButton *resetTutorialMax14906 = new QPushButton("Reset ", generalSection);
	connect(resetTutorialMax14906, &QPushButton::clicked, this,
		[=, this]() { p->set("max14906_start_tutorial", true); });

	QWidget *resetTutorialFaultsWidget = new QWidget();
	QHBoxLayout *resetTutorialFaultsWidgetLayout = new QHBoxLayout();

	resetTutorialFaultsWidget->setLayout(resetTutorialFaultsWidgetLayout);
	resetTutorialFaultsWidgetLayout->setMargin(0);

	QPushButton *resetTutorialFaults = new QPushButton("Reset ", generalSection);
	connect(resetTutorialFaults, &QPushButton::clicked, this,
		[=, this]() { p->set("faults_start_tutorial", true); });

	QWidget *resetTutorialConfigurationWidget = new QWidget();
	QHBoxLayout *resetTutorialConfigurationWidgetLayout = new QHBoxLayout();

	resetTutorialConfigurationWidget->setLayout(resetTutorialConfigurationWidgetLayout);
	resetTutorialConfigurationWidgetLayout->setMargin(0);

	QPushButton *resetTutorialConfig = new QPushButton("Reset ", generalSection);
	connect(resetTutorialConfig, &QPushButton::clicked, this,
		[=, this]() { p->set("swiote_config_start_tutorial", true); });

	StyleHelper::BlueButton(resetTutorialAd74413r, "resetBtn");
	resetTutorialMax14906WidgetLayout->addWidget(new QLabel("Ad74413r tutorial"), 6);
	resetTutorialMax14906WidgetLayout->addWidget(resetTutorialAd74413r, 1);
	generalSection->contentLayout()->addWidget(resetTutorialAd74413rWidget);

	StyleHelper::BlueButton(resetTutorialMax14906, "resetBtn");
	resetTutorialAd74413rWidgetLayout->addWidget(new QLabel("Max14906 tutorial"), 6);
	resetTutorialAd74413rWidgetLayout->addWidget(resetTutorialMax14906, 1);
	generalSection->contentLayout()->addWidget(resetTutorialMax14906Widget);

	StyleHelper::BlueButton(resetTutorialFaults, "resetBtn");
	resetTutorialFaultsWidgetLayout->addWidget(new QLabel("Faults tutorial"), 6);
	resetTutorialFaultsWidgetLayout->addWidget(resetTutorialFaults, 1);
	generalSection->contentLayout()->addWidget(resetTutorialFaultsWidget);

	StyleHelper::BlueButton(resetTutorialConfig, "resetBtn");
	resetTutorialConfigurationWidgetLayout->addWidget(new QLabel("Configuration tutorial"), 6);
	resetTutorialConfigurationWidgetLayout->addWidget(resetTutorialConfig, 1);
	generalSection->contentLayout()->addWidget(resetTutorialConfigurationWidget);

	return true;
}

void SWIOTPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":101,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*", "!debugger"],
	   "include-forced":["regmap"]
	}
)plugin");
}

#include "moc_swiotplugin.cpp"
