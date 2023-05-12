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
	m_displayName = "SWIOT";

	m_swiotController = new SwiotController(m_param, this);
	m_runtime = new SwiotRuntime();
}

bool SWIOTPlugin::loadPage()
{
	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_infoPage = new swiot::SwiotInfoPage(m_page);
	m_page->layout()->addWidget(m_infoPage);
	connect(m_infoPage->getCtxAttrsButton(), &QPushButton::clicked, this, [this] (){
		auto &&cp = ContextProvider::GetInstance();
		iio_context* ctx = cp->open(m_param);
		QString hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
		cp->close(m_param);
		m_infoPage->setText(hw_serial);
	});
	m_page->ensurePolished();

	return true;
}

bool SWIOTPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/swiot/swiot_top_transparent.png);");
	return true;
}

void SWIOTPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(CONFIG_TME_ID, "Config", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(AD74413R_TME_ID, "AD74413R",":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(MAX14906_TME_ID, "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(FAULTS_TME_ID, "Faults", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void SWIOTPlugin::unload()
{
	delete m_infoPage;
}

bool SWIOTPlugin::compatible(QString m_param, QString category)
{
	m_name = "SWIOT";
	bool ret = false;
	auto &&cp = ContextProvider::GetInstance();

	iio_context* ctx = cp->open(m_param);

	if(!ctx) {
		qWarning(CAT_SWIOT) << "No context available for swiot";
		return false;
	}

	unsigned int devices_count = iio_context_get_devices_count(ctx);
	if (devices_count >= 2) {
		iio_device* device0 = iio_context_find_device(ctx, AD_NAME);
		iio_device* device1 = iio_context_find_device(ctx, MAX_NAME);
		if (device0 && device1) {
			ret = true;
		} else {
			ret = false;
		}
	} else {
		ret = false;
	}

	cp->close(m_param);

	return ret;
}

void SWIOTPlugin::cleanAfterLastContext()
{
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		delete tme->tool();
		tme->setTool(nullptr);
	}
	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_param);
}

void SWIOTPlugin::setupToolList()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context* ctx = cp->open(m_param);

	m_swiotController->connectSwiot(ctx);
	m_swiotController->startPingTask();
	m_runtime->setContext(ctx);

	auto configTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, CONFIG_TME_ID);
	auto ad74413rTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, AD74413R_TME_ID);
	auto max14906Tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, MAX14906_TME_ID);
	auto faultsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, FAULTS_TME_ID);

	configTme->setTool(new swiot::SwiotConfig(ctx));
	ad74413rTme->setTool(new swiot::Ad74413r(ctx, ad74413rTme, m_chnlsFunction));
	max14906Tme->setTool(new swiot::Max14906(ctx, max14906Tme));
	faultsTme->setTool(new swiot::Faults(ctx, faultsTme));

	connect(dynamic_cast<SwiotConfig*> (configTme->tool()), &SwiotConfig::configBtn, this, [=](QVector<QStringList*> funcAvailable) {
		QVector<QString> adConfigFunc = funcAvailable[0]->toVector();
		m_chnlsFunction = adConfigFunc;
		m_swiotController->stopPingTask();
		m_swiotController->startSwitchContextTask();
	});
	connect(dynamic_cast<Ad74413r*> (ad74413rTme->tool()), &Ad74413r::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906*> (max14906Tme->tool()), &Max14906::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults*> (faultsTme->tool()), &Faults::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(true);
		if (!m_runtime->isRuntimeCtx()) {
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

	if (!m_runtime->isRuntimeCtx()) {
		Q_EMIT requestTool(configTme->id());

	} else {
		Q_EMIT requestTool(ad74413rTme->id());
	}
}

bool SWIOTPlugin::onConnect()
{
	setupToolList();

	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);
	connect(m_swiotController, &SwiotController::pingSuccess, this, [](){
		qDebug(CAT_SWIOT) << "Ping success!";
	});
	connect(m_swiotController, &SwiotController::contextSwitched, this,[this](){
		m_swiotController->stopSwitchContextTask();
		cleanAfterLastContext();
		setupToolList();

	});
	connect(m_runtime, &SwiotRuntime::backBtnPressed, this, [this]() {
		m_swiotController->stopPingTask();
		m_swiotController->startSwitchContextTask();
	});
	return true;
}

bool SWIOTPlugin::onDisconnect()
{
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		delete tme->tool();
		tme->setTool(nullptr);
	}

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);

	m_swiotController->stopPingTask();
	m_swiotController->disconnectSwiot();

	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_param);

	return true;
}

void SWIOTPlugin::initMetadata()
{
	loadMetadata(
				R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*", "!debuggerplugin", "!dataloggerplugin", "!regmapplugin"]
	}
)plugin");
}



#include "moc_swiotplugin.cpp"
