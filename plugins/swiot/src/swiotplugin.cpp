/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "swiotplugin.h"

#include "src/config/swiotconfig.h"
#include "src/runtime/faults/faults.h"
#include "src/runtime/max14906/max14906.h"
#include "src/runtime/swiotruntime.h"
#include "src/swiot_logging_categories.h"
#include "swiotinfopage.h"
#include "utils.h"

#include <iio.h>

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

#include <iioutil/contextprovider.h>

using namespace scopy::swiot;

void SWIOTPlugin::preload()
{
	m_displayName = "SWIOT1L";
	m_swiotController = new SwiotController(m_param, this);
	connect(m_swiotController, &SwiotController::contextSwitched, this, &SWIOTPlugin::onCtxSwitched,
		Qt::QueuedConnection);
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this, &SWIOTPlugin::onIsRuntimeCtxChanged);
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
	connect(m_btnTutorial, &QPushButton::clicked, this, &SWIOTPlugin::startTutorial);

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);

	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_SWIOT) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}

	cp->close(m_param);
	m_page->ensurePolished();

	return true;
}

bool SWIOTPlugin::loadExtraButtons()
{
	m_btnIdentify = new QPushButton("Identify");
	m_extraButtons.append(m_btnIdentify);
	connect(m_btnIdentify, SIGNAL(clicked()), m_swiotController, SLOT(identify()));

	// The tutorial button will only be clickable when the user connects to the device
	m_btnTutorial = new QPushButton(tr("Tutorial"));
	m_extraButtons.append(m_btnTutorial);
	m_btnTutorial->setEnabled(false);
	m_btnTutorial->setToolTip("The tutorial will be available once the device is connected.");

	return true;
}

bool SWIOTPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/swiot/swiot_icon.svg);");
	return true;
}

void SWIOTPlugin::showPageCallback() {}

void SWIOTPlugin::hidePageCallback() {}

void SWIOTPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(CONFIG_TME_ID, "Config", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(AD74413R_TME_ID, "AD74413R",
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(MAX14906_TME_ID, "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(FAULTS_TME_ID, "Faults", ":/swiot/tool_faults.svg"));
}

void SWIOTPlugin::unload()
{
	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this, &SWIOTPlugin::onIsRuntimeCtxChanged);
	disconnect(m_swiotController, &SwiotController::contextSwitched, this, &SWIOTPlugin::onCtxSwitched);
	delete m_infoPage;
	delete m_swiotController;
}

bool SWIOTPlugin::compatible(QString m_param, QString category)
{
	m_name = "SWIOT1L";
	bool ret = false;
	auto &&cp = ContextProvider::GetInstance();

	iio_context *ctx = cp->open(m_param);

	if(!ctx) {
		qWarning(CAT_SWIOT) << "No context available for swiot";
		return false;
	}

	iio_device *swiotDevice = iio_context_find_device(ctx, "swiot");
	if(swiotDevice) {
		ret = true;
	}

	cp->close(m_param);

	return ret;
}

void SWIOTPlugin::setupToolList()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context *ctx = cp->open(m_param);

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
		ad74413rTme->setTool(new swiot::Ad74413r(ctx, ad74413rTme));
		max14906Tme->setTool(new swiot::Max14906(ctx, max14906Tme));
		faultsTme->setTool(new swiot::Faults(ctx, faultsTme));
	} else {
		configTme->setTool(new swiot::SwiotConfig(ctx));
	}

	connect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::writeModeAttribute, m_swiotController,
		&SwiotController::writeModeAttribute);
	connect(m_swiotController, &SwiotController::modeAttributeChanged,
		dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::modeAttributeChanged);
	connect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::configBtnPressed, this,
		&SWIOTPlugin::startCtxSwitch);
	connect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::backBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::backBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);

	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Faults *>(faultsTme->tool()),
		&Faults::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::externalPowerSupply);
	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);

	if(!m_isRuntime) {
		requestTool(configTme->id());
	} else {
		requestTool(ad74413rTme->id());
	}
	cp->close(m_param);
}

void SWIOTPlugin::onIsRuntimeCtxChanged(bool isRuntimeCtx)
{
	m_isRuntime = isRuntimeCtx;
	setupToolList();
}

bool SWIOTPlugin::onConnect()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context *ctx = cp->open(m_param);

	m_runtime = new SwiotRuntime(ctx, this);
	connect(m_runtime, &SwiotRuntime::writeModeAttribute, m_swiotController, &SwiotController::writeModeAttribute);
	connect(m_swiotController, &SwiotController::modeAttributeChanged, m_runtime,
		&SwiotRuntime::modeAttributeChanged);
	connect(m_runtime, &SwiotRuntime::backBtnPressed, this, &SWIOTPlugin::startCtxSwitch);
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		&SwiotRuntime::onIsRuntimeCtxChanged);

	m_swiotController->connectSwiot(ctx);
	m_swiotController->startPingTask();
	m_swiotController->startPowerSupplyTask("ext_psu");
	m_btnTutorial->setEnabled(true);
	m_btnTutorial->setToolTip("");

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

	disconnect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::configBtnPressed, this,
		   &SWIOTPlugin::startCtxSwitch);
	disconnect(m_swiotController, &SwiotController::modeAttributeChanged,
		   dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::modeAttributeChanged);
	disconnect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::writeModeAttribute, m_swiotController,
		   &SwiotController::writeModeAttribute);
	disconnect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::backBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::backBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);

	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		   dynamic_cast<Faults *>(faultsTme->tool()), &Faults::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		   dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		   dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply,
		   dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::externalPowerSupply);

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);

	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		   &SwiotRuntime::onIsRuntimeCtxChanged);
	disconnect(m_runtime, &SwiotRuntime::backBtnPressed, this, &SWIOTPlugin::startCtxSwitch);
	disconnect(m_swiotController, &SwiotController::modeAttributeChanged, m_runtime,
		   &SwiotRuntime::modeAttributeChanged);
	disconnect(m_runtime, &SwiotRuntime::writeModeAttribute, m_swiotController,
		   &SwiotController::writeModeAttribute);

	m_swiotController->stopPingTask();
	m_swiotController->stopPowerSupplyTask();
	m_swiotController->stopTemperatureTask();
	m_swiotController->disconnectSwiot();

	m_btnTutorial->setEnabled(false);

	if(m_runtime) {
		delete m_runtime;
		m_runtime = nullptr;
	}

	ContextProvider *cp = ContextProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void SWIOTPlugin::startCtxSwitch()
{
	m_swiotController->stopPingTask();
	m_swiotController->stopPowerSupplyTask();
	//	if (m_isRuntime) {
	//		m_swiotController->stopTemperatureTask();
	//	}

	m_swiotController->disconnectSwiot();
	Q_EMIT disconnectDevice();
	m_swiotController->startSwitchContextTask(m_isRuntime);
}

void SWIOTPlugin::onCtxSwitched()
{
	m_swiotController->stopSwitchContextTask();
	Q_EMIT connectDevice();
}

void SWIOTPlugin::startTutorial()
{
	if(m_isRuntime) {
		qInfo(CAT_SWIOT) << "Starting SWIOT runtime tutorial.";

		// The tutorial builder is responsible for deleting itself after the tutorial is finished, so creating
		// a new one each time the tutorial is started will not create memory leaks.
		ToolMenuEntry *ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
		ToolMenuEntry *max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
		ToolMenuEntry *faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

		QWidget *ad74413rTool = ad74413rTme->tool();
		QWidget *max14906Tool = max14906Tme->tool();
		QWidget *faultsTool = faultsTme->tool();
		QWidget *parent = Util::findContainingWindow(ad74413rTool);

		m_ad74413rTutorial =
			new gui::TutorialBuilder(ad74413rTool, ":/swiot/tutorial_chapters.json", "ad74413r", parent);
		m_max14906Tutorial =
			new gui::TutorialBuilder(max14906Tool, ":/swiot/tutorial_chapters.json", "max14906", parent);
		m_faultsTutorial =
			new gui::TutorialBuilder(faultsTool, ":/swiot/tutorial_chapters.json", "faults", parent);

		connect(m_ad74413rTutorial, &gui::TutorialBuilder::finished, this, &SWIOTPlugin::startMax14906Tutorial);
		connect(m_max14906Tutorial, &gui::TutorialBuilder::finished, this, &SWIOTPlugin::startFaultsTutorial);
		connect(m_ad74413rTutorial, &gui::TutorialBuilder::aborted, this, &SWIOTPlugin::abortTutorial);

		this->startAd74413rTutorial();
	} else {
		qInfo(CAT_SWIOT) << "Starting SWIOT config tutorial.";
		auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);

		auto configTool = configTme->tool();
		auto parent = Util::findContainingWindow(configTool);
		auto tut = new gui::TutorialBuilder(configTool, ":/swiot/tutorial_chapters.json", "config", parent);

		tut->setTitle("CONFIG");
		requestTool(configTme->id());
		tut->start();
	}
}

void SWIOTPlugin::startAd74413rTutorial()
{
	qInfo(CAT_SWIOT) << "Starting ad74413r tutorial.";
	ToolMenuEntry *ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	this->requestTool(ad74413rTme->id());
	m_ad74413rTutorial->setTitle("AD74413R");
	m_ad74413rTutorial->start();
}

void SWIOTPlugin::startMax14906Tutorial()
{
	qInfo(CAT_SWIOT) << "Starting max14906 tutorial.";
	ToolMenuEntry *max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	this->requestTool(max14906Tme->id());
	m_max14906Tutorial->setTitle("MAX14906");
	m_max14906Tutorial->start();
}

void SWIOTPlugin::startFaultsTutorial()
{
	qInfo(CAT_SWIOT) << "Starting faults tutorial.";
	ToolMenuEntry *faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);
	this->requestTool(faultsTme->id());
	m_faultsTutorial->setTitle("FAULTS");
	m_faultsTutorial->start();
}

void SWIOTPlugin::abortTutorial()
{
	disconnect(m_ad74413rTutorial, &gui::TutorialBuilder::finished, this, &SWIOTPlugin::startMax14906Tutorial);
	disconnect(m_max14906Tutorial, &gui::TutorialBuilder::finished, this, &SWIOTPlugin::startFaultsTutorial);
}

QString SWIOTPlugin::description() { return "Adds functionality specific to SWIOT1L board"; }

void SWIOTPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*", "!debuggerplugin"],
	   "include-forced":["regmapplugin"]
	}
)plugin");
}

#include "moc_swiotplugin.cpp"
