#include "swiotrefactor.h"

#include <QLoggingCategory>
#include <QLabel>

#include "swiot_logging_categories.h"
#include "max14906/max14906.h"
#include "faults/faults.h"
#include "ad74413r/ad74413r.h"

#include <iioutil/connectionprovider.h>

#include <src/config/swiotconfig.h>

#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_SWIOTREFACTOR, "SWIOTREFACTORPlugin")
using namespace scopy::swiotrefactor;

bool SWIOTREFACTORPlugin::compatible(QString m_param, QString category)
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

void SWIOTREFACTORPlugin::preload()
{
	m_displayName = "SWIOT1L";
	m_swiotController = new SwiotController(m_param, this);
	m_statusContainer = nullptr;
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this,
		&SWIOTREFACTORPlugin::onIsRuntimeCtxChanged);
	connect(m_swiotController, &SwiotController::modeAttributeChanged, this,
		&SWIOTREFACTORPlugin::onModeAttributeChanged);
	connect(m_swiotController, &SwiotController::writeModeFailed, m_swiotController,
		&SwiotController::disconnectSwiot);
}

bool SWIOTREFACTORPlugin::loadPage()
{
	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new swiotrefactor::SwiotInfoPage(m_page);
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
	connect(m_btnTutorial, &QPushButton::clicked, this, &SWIOTREFACTORPlugin::startTutorial);

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

bool SWIOTREFACTORPlugin::loadExtraButtons()
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

bool SWIOTREFACTORPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/swiotrefactor/swiot_icon.svg);");
	return true;
}

void SWIOTREFACTORPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(CONFIG_TME_ID, "Config", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(AD74413R_TME_ID, "AD74413R",
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY(MAX14906_TME_ID, "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(FAULTS_TME_ID, "Faults", ":/swiotrefactor/tool_faults.svg"));
}

void SWIOTREFACTORPlugin::unload()
{
	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, this,
		   &SWIOTREFACTORPlugin::onIsRuntimeCtxChanged);
	disconnect(m_swiotController, &SwiotController::modeAttributeChanged, this,
		   &SWIOTREFACTORPlugin::onModeAttributeChanged);
	disconnect(m_swiotController, &SwiotController::writeModeFailed, m_swiotController,
		   &SwiotController::disconnectSwiot);
	delete m_infoPage;
	delete m_swiotController;
}

bool SWIOTREFACTORPlugin::onConnect()
{

	Connection *conn = ConnectionProvider::open(m_param);
	if(!conn) {
		return false;
	}

	m_runtime = new SwiotRuntime(m_param, this);
	connect(m_runtime, &SwiotRuntime::writeModeAttribute, this, &SWIOTREFACTORPlugin::setCtxMode);
	connect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		&SwiotRuntime::onIsRuntimeCtxChanged);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, this,
		&SWIOTREFACTORPlugin::powerSupplyStatus);

	m_swiotController->connectSwiot();
	m_swiotController->readModeAttribute();
	m_swiotController->startPingTask();
	m_swiotController->startPowerSupplyTask("ext_psu");
	m_btnTutorial->setEnabled(true);
	m_btnTutorial->setToolTip("");

	return true;
}

bool SWIOTREFACTORPlugin::onDisconnect()
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
		   &SWIOTREFACTORPlugin::setCtxMode);
	disconnect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::configBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::configBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		   &SwiotRuntime::onBackBtnPressed);

	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, this,
		   &SWIOTREFACTORPlugin::powerSupplyStatus);

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTREFACTORPlugin::disconnectDevice);

	disconnect(m_swiotController, &SwiotController::isRuntimeCtxChanged, m_runtime,
		   &SwiotRuntime::onIsRuntimeCtxChanged);

	m_swiotController->stopPingTask();
	m_swiotController->stopPowerSupplyTask();
	m_swiotController->stopTemperatureTask();
	m_swiotController->disconnectSwiot();

	m_btnTutorial->setEnabled(false);

	if(m_runtime) {
		disconnect(m_runtime, &SwiotRuntime::writeModeAttribute, this, &SWIOTREFACTORPlugin::setCtxMode);
		delete m_runtime;
		m_runtime = nullptr;
	}

	if(m_statusContainer) {
		delete m_statusContainer;
		m_statusContainer = nullptr;
	}

	ConnectionProvider::close(m_param);

	if(m_switchCmd) {
		m_switchCmd = false;
		switchCtx();
	}

	return true;
}

void SWIOTREFACTORPlugin::onIsRuntimeCtxChanged(bool isRuntimeCtx)
{
	m_isRuntime = isRuntimeCtx;
	setupToolList();
}

void SWIOTREFACTORPlugin::startTutorial()
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

		m_ad74413rTutorial = new gui::TutorialBuilder(ad74413rTool, ":/swiotrefactor/tutorial_chapters.json",
							      "ad74413r", parent);
		m_max14906Tutorial = new gui::TutorialBuilder(max14906Tool, ":/swiotrefactor/tutorial_chapters.json",
							      "max14906", parent);
		m_faultsTutorial = new gui::TutorialBuilder(faultsTool, ":/swiotrefactor/tutorial_chapters.json",
							    "faults", parent);

		connect(m_ad74413rTutorial, &gui::TutorialBuilder::finished, this,
			&SWIOTREFACTORPlugin::startMax14906Tutorial);
		connect(m_max14906Tutorial, &gui::TutorialBuilder::finished, this,
			&SWIOTREFACTORPlugin::startFaultsTutorial);
		connect(m_ad74413rTutorial, &gui::TutorialBuilder::aborted, this, &SWIOTREFACTORPlugin::abortTutorial);

		this->startAd74413rTutorial();
	} else {
		qInfo(CAT_SWIOT) << "Starting SWIOT config tutorial.";
		auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);

		auto configTool = configTme->tool();
		auto parent = Util::findContainingWindow(configTool);
		auto tut = new gui::TutorialBuilder(configTool, ":/swiotrefactor/tutorial_chapters.json", "config",
						    parent);

		tut->setTitle("CONFIG");
		requestTool(configTme->id());
		tut->start();
	}
}

void SWIOTREFACTORPlugin::startAd74413rTutorial()
{
	qInfo(CAT_SWIOT) << "Starting ad74413r tutorial.";
	ToolMenuEntry *ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	this->requestTool(ad74413rTme->id());
	m_ad74413rTutorial->setTitle("AD74413R");
	m_ad74413rTutorial->start();
}

void SWIOTREFACTORPlugin::startMax14906Tutorial()
{
	qInfo(CAT_SWIOT) << "Starting max14906 tutorial.";
	ToolMenuEntry *max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	this->requestTool(max14906Tme->id());
	m_max14906Tutorial->setTitle("MAX14906");
	m_max14906Tutorial->start();
}

void SWIOTREFACTORPlugin::startFaultsTutorial()
{
	qInfo(CAT_SWIOT) << "Starting faults tutorial.";
	ToolMenuEntry *faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);
	this->requestTool(faultsTme->id());
	m_faultsTutorial->setTitle("FAULTS");
	m_faultsTutorial->start();
}

void SWIOTREFACTORPlugin::powerSupplyStatus(bool ps)
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

void SWIOTREFACTORPlugin::setCtxMode(QString mode)
{
	m_ctxMode = mode;
	m_switchCmd = true;
	Q_EMIT disconnectDevice();
}

void SWIOTREFACTORPlugin::onModeAttributeChanged(QString mode)
{
	if(m_ctxMode.contains(mode)) {
		m_swiotController->disconnectSwiot();
		Q_EMIT connectDevice();
	}
}

void SWIOTREFACTORPlugin::switchCtx()
{
	m_swiotController->connectSwiot();
	m_swiotController->writeModeAttribute(m_ctxMode);
}

void SWIOTREFACTORPlugin::createStatusContainer()
{
	m_statusContainer = new QWidget();
	m_statusContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0, 0, 0, 0);

	auto exclamationIcon = new QPushButton(m_statusContainer);
	StyleHelper::NoBackgroundIconButton(exclamationIcon, QIcon::fromTheme(":/swiotrefactor/warning.svg"));

	auto statusLabel = new QLabel("AD-SWIOT1L-SL: The system is powered at limited capacity.");
	statusLabel->setWordWrap(true);
	StyleHelper::WarningLabel(statusLabel, "extPsuStatusLabel");

	m_statusContainer->layout()->addWidget(exclamationIcon);
	m_statusContainer->layout()->addWidget(statusLabel);
}

void SWIOTREFACTORPlugin::setupToolList()
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
		ad74413rTme->setTool(new swiotrefactor::Ad74413r(m_param, ad74413rTme));
		max14906Tme->setTool(new swiotrefactor::Max14906(m_param, max14906Tme));
		faultsTme->setTool(new swiotrefactor::Faults(m_param, faultsTme));
	} else {
		configTme->setTool(new swiotrefactor::SwiotConfig(m_param));
	}

	connect(dynamic_cast<SwiotConfig *>(configTme->tool()), &SwiotConfig::writeModeAttribute, this,
		&SWIOTREFACTORPlugin::setCtxMode);

	connect(dynamic_cast<Ad74413r *>(ad74413rTme->tool()), &Ad74413r::configBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906 *>(max14906Tme->tool()), &Max14906::configBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults *>(faultsTme->tool()), &Faults::backBtnPressed, m_runtime,
		&SwiotRuntime::onBackBtnPressed);

	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTREFACTORPlugin::disconnectDevice);

	if(!m_isRuntime) {
		requestTool(configTme->id());
	} else {
		requestTool(ad74413rTme->id());
	}
}

void SWIOTREFACTORPlugin::abortTutorial()
{
	disconnect(m_ad74413rTutorial, &gui::TutorialBuilder::finished, this,
		   &SWIOTREFACTORPlugin::startMax14906Tutorial);
	disconnect(m_max14906Tutorial, &gui::TutorialBuilder::finished, this,
		   &SWIOTREFACTORPlugin::startFaultsTutorial);
}

QString SWIOTREFACTORPlugin::description() { return "Adds functionality specific to SWIOT1L board"; }

void SWIOTREFACTORPlugin::initMetadata()
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
