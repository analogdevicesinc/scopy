#include "swiotplugin.h"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

#include "src/runtime/faults/faults.h"
#include "src/runtime/max14906/max14906.h"
#include "src/config/swiotconfig.h"
#include "src/runtime/swiotruntime.h"
#include "src/swiot_logging_categories.h"
#include "swiotinfopage.h"
#include <iioutil/contextprovider.h>


using namespace scopy::swiot;

void SWIOTPlugin::preload()
{
	m_displayName = "SWIOT1L";
	m_swiotController = new SwiotController(m_param, this);
	m_runtime = new SwiotRuntime();
	connect(m_swiotController, &SwiotController::contextSwitched, this, &SWIOTPlugin::onCtxSwitched, Qt::QueuedConnection);
	connect(m_runtime, &SwiotRuntime::backBtnPressed, this, &SWIOTPlugin::startCtxSwitch);
}

bool SWIOTPlugin::loadPage()
{
	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new swiot::SwiotInfoPage(m_page);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(m_swiotController, &SwiotController::readTemperature, this, [this] (double temperature) {
		m_infoPage->update("Temperature", QString::number(temperature));
	});

	auto cp = ContextProvider::GetInstance();
	struct iio_context* context = cp->open(m_param);

	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for (int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if (ret < 0) {
			qWarning(CAT_SWIOT) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}

	cp->close(m_param);
	m_page->ensurePolished();

	return true;
}

bool SWIOTPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/swiot/swiot_icon.svg);");
	return true;
}

void SWIOTPlugin::showPageCallback() {
	m_swiotController->startTemperatureTask();
}

void SWIOTPlugin::hidePageCallback() {
	m_swiotController->stopTemperatureTask();
}

void SWIOTPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(CONFIG_TME_ID, "Config", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(AD74413R_TME_ID, "AD74413R",":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(MAX14906_TME_ID, "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(FAULTS_TME_ID, "Faults", ":/swiot/tool_faults.svg"));
}

void SWIOTPlugin::unload()
{
	disconnect(m_swiotController, &SwiotController::contextSwitched, this, &SWIOTPlugin::onCtxSwitched);
	disconnect(m_runtime, &SwiotRuntime::backBtnPressed, this, &SWIOTPlugin::startCtxSwitch);
	delete m_infoPage;
}

bool SWIOTPlugin::compatible(QString m_param, QString category)
{
	m_name = "SWIOT1L";
	bool ret = false;
	auto &&cp = ContextProvider::GetInstance();

	iio_context* ctx = cp->open(m_param);

	if(!ctx) {
		qWarning(CAT_SWIOT) << "No context available for swiot";
		return false;
	}

	iio_device* swiotDevice = iio_context_find_device(ctx, "swiot");
	if (swiotDevice) {
		ret = true;
	}

	cp->close(m_param);

	return ret;
}

void SWIOTPlugin::setupToolList()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context* ctx = cp->open(m_param);

	m_swiotController->connectSwiot(ctx);
	m_swiotController->startPingTask();
	m_swiotController->startPowerSupplyTask("ext_psu");

	m_runtime->setContext(ctx);

	m_isRuntime = m_runtime->isRuntimeCtx();

	auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);
	auto ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	auto max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	auto faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

	if (m_isRuntime) {
		ad74413rTme->setTool(new swiot::Ad74413r(ctx, ad74413rTme));
		max14906Tme->setTool(new swiot::Max14906(ctx, max14906Tme));
		faultsTme->setTool(new swiot::Faults(ctx, faultsTme));
	} else {
		configTme->setTool(new swiot::SwiotConfig(ctx));
	}

	connect(dynamic_cast<SwiotConfig*> (configTme->tool()), &SwiotConfig::configBtn, this, &SWIOTPlugin::startCtxSwitch);
	connect(dynamic_cast<Ad74413r*> (ad74413rTme->tool()), &Ad74413r::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906*> (max14906Tme->tool()), &Max14906::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults*> (faultsTme->tool()), &Faults::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);


	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Faults*> (faultsTme->tool()), &Faults::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Max14906*> (max14906Tme->tool()), &Max14906::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Ad74413r*> (ad74413rTme->tool()), &Ad74413r::externalPowerSupply);
	connect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<SwiotConfig*> (configTme->tool()), &SwiotConfig::externalPowerSupply);
	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);


	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(true);
		tme->setVisible(true);
		if (!m_isRuntime) {
			if (tme->id().compare(CONFIG_TME_ID)) {
				tme->setVisible(false);
			}
		} else {
			if (tme->id().compare(CONFIG_TME_ID)) {
				tme->setRunBtnVisible(true);
				tme->setRunning(false);
			} else {
				tme->setVisible(false);
			}
		}
	}

	if (!m_isRuntime) {
		Q_EMIT requestTool(configTme->id());
	} else {
		Q_EMIT requestTool(ad74413rTme->id());
	}
}

bool SWIOTPlugin::onConnect()
{
	setupToolList();
	return true;
}

bool SWIOTPlugin::onDisconnect()
{
	auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);
	auto ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	auto max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	auto faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		delete tme->tool();
		tme->setTool(nullptr);
	}

	disconnect(dynamic_cast<SwiotConfig*> (configTme->tool()), &SwiotConfig::configBtn, this, &SWIOTPlugin::startCtxSwitch);
	disconnect(dynamic_cast<Ad74413r*> (ad74413rTme->tool()), &Ad74413r::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Max14906*> (max14906Tme->tool()), &Max14906::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	disconnect(dynamic_cast<Faults*> (faultsTme->tool()), &Faults::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);

	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Faults*> (faultsTme->tool()), &Faults::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Max14906*> (max14906Tme->tool()), &Max14906::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<Ad74413r*> (ad74413rTme->tool()), &Ad74413r::externalPowerSupply);
	disconnect(m_swiotController, &SwiotController::hasConnectedPowerSupply, dynamic_cast<SwiotConfig*> (configTme->tool()), &SwiotConfig::externalPowerSupply);

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);

	m_swiotController->stopPingTask();
	m_swiotController->stopPowerSupplyTask();
	m_swiotController->stopTemperatureTask();
	m_swiotController->disconnectSwiot();

	ContextProvider *cp = ContextProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void SWIOTPlugin::startCtxSwitch()
{
	m_swiotController->stopPingTask();
	m_swiotController->stopPowerSupplyTask();
	m_swiotController->stopTemperatureTask();
	m_swiotController->disconnectSwiot();
    Q_EMIT disconnectDevice();
	m_swiotController->startSwitchContextTask(m_isRuntime);
}

void SWIOTPlugin::onCtxSwitched()
{
	m_swiotController->stopSwitchContextTask();
	Q_EMIT connectDevice();
}

QString SWIOTPlugin::description()
{
	return "Adds functionality specific to SWIOT1L board";
}

void SWIOTPlugin::initMetadata()
{
	loadMetadata(
				R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*"]
	}
)plugin");
}



#include "moc_swiotplugin.cpp"
