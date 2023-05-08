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
#include <iioutil/contextprovider.h>

using namespace scopy::swiot;

void SWIOTPlugin::preload()
{
	m_swiotController = new SwiotController(m_param, this);
	m_runtime = new SwiotRuntime();
	adtool = nullptr;
	maxtool = nullptr;
	faults = nullptr;
	config = nullptr;
}

bool SWIOTPlugin::loadPage()
{
	infoui = new Ui::SWIOTInfoPage();
	m_page = new QWidget();
	infoui->setupUi(m_page);
	connect(infoui->pushButton,&QPushButton::clicked, this, [this] (){
		auto &&cp = ContextProvider::GetInstance();
		iio_context* ctx = cp->open(m_param);
		QString hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
		cp->close(m_param);
		infoui->textBrowser->setText(hw_serial);
	});
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("swiotconfig", "Config", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("swiotad74413r", "AD74413R",":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("swiotmax14906", "MAX14906", ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("swiotfaults", "Faults", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void SWIOTPlugin::unload()
{
	delete infoui;
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
		iio_device* device0 = iio_context_get_device(ctx, 0);
		iio_device* device1 = iio_context_get_device(ctx, 1);

		std::string device0_name = iio_device_get_name(device0);
		std::string device1_name = iio_device_get_name(device1);

		if ((device0_name == "ad74413r" && device1_name == "max14906") || (device0_name == "max14906" && device1_name == "ad74413r")) {
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
	for (auto & tool : m_toolList) {
		tool->setEnabled(false);
		tool->setTool(nullptr);
	}

	if (config) {
		delete config;
		config = nullptr;
	}
	if (adtool) {
		delete adtool;
		adtool = nullptr;
	}
	if (faults) {
		delete faults;
		faults = nullptr;
	}
	if (maxtool) {
		delete maxtool;
		maxtool = nullptr;
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

	if (!m_runtime->isRuntimeCtx()) {
		config = new swiot::SwiotConfig(ctx);
		m_toolList[0]->setEnabled(true);
		m_toolList[0]->setTool(config);
		m_toolList[1]->setEnabled(false);
		m_toolList[2]->setEnabled(false);
		m_toolList[3]->setEnabled(false);
		Q_EMIT requestTool(m_toolList[0]->id());

		connect(dynamic_cast<SwiotConfig*> (config), &SwiotConfig::configBtn, this, [=](QVector<QStringList*> funcAvailable) {
			QVector<QString> adConfigFunc = funcAvailable[0]->toVector();
			for (int i = 0; i < adConfigFunc.size(); i++){
				m_chnlsFunction[i] = adConfigFunc[i];
			}
			m_swiotController->stopPingTask();
			m_swiotController->startSwitchContextTask();
		});
	} else {
		adtool = new swiot::Ad74413r(ctx, channel_function);
		faults = new swiot::Faults(ctx);
		maxtool = new swiot::Max14906(ctx);
		m_toolList[1]->setEnabled(true);
		m_toolList[1]->setTool(adtool);

		m_toolList[2]->setEnabled(true);
		m_toolList[2]->setTool(maxtool);

		m_toolList[3]->setEnabled(true);
		m_toolList[3]->setTool(faults);

		m_toolList[0]->setEnabled(false);
		Q_EMIT requestTool(m_toolList[1]->id());

		connect(dynamic_cast<Ad74413r*> (adtool), &Ad74413r::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
		connect(dynamic_cast<Max14906*> (maxtool), &Max14906::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
		connect(dynamic_cast<Faults*> (faults), &Faults::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	}
}

bool SWIOTPlugin::onConnect()
{
	setupToolList();

	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);
	connect(m_swiotController, &SwiotController::pingSuccess, this, [](){
		qDebug(CAT_SWIOT) << "Ping success!";
	});
	connect(m_swiotController, &SwiotController::contextSwitched, this,[=](){
		m_swiotController->stopSwitchContextTask();
		cleanAfterLastContext();
		setupToolList();

	});
	connect(m_runtime, &SwiotRuntime::backBtnPressed, this, [=]() {
		m_swiotController->stopPingTask();
		m_swiotController->startSwitchContextTask();
	});
	return true;
}

bool SWIOTPlugin::onDisconnect()
{
        for (auto & tool : m_toolList) {
                tool->setEnabled(false);
                tool->setTool(nullptr);
        }

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);

	m_swiotController->stopPingTask();
	m_swiotController->disconnectSwiot();

	delete config;
	delete adtool;
        delete faults;
	delete maxtool;

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
	   ]
	}
)plugin");
}



#include "moc_swiotplugin.cpp"
