#include "swiotplugin.h"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QUuid>

#include "src/runtime/faults/faults.hpp"
#include "src/runtime/max14906/max14906.hpp"
#include "src/config/swiotconfig.hpp"
#include "src/runtime/swiotruntime.hpp"
#include <iioutil/contextprovider.h>

using namespace adiscope;
using namespace adiscope::swiot;

Q_LOGGING_CATEGORY(CAT_SWIOT,"SWIOTPlugin");

Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_CONFIG)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_RUNTIME)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_FAULTS)
Q_DECLARE_LOGGING_CATEGORY(CAT_MAX14906)

Q_LOGGING_CATEGORY(CAT_SWIOT_CONFIG, "swiotConfig")
Q_LOGGING_CATEGORY(CAT_SWIOT_RUNTIME, "swiotRuntime")
Q_LOGGING_CATEGORY(CAT_SWIOT_FAULTS, "swiotFaults")
Q_LOGGING_CATEGORY(CAT_MAX14906, "max14906tool")

void SWIOTPlugin::preload()
{
	m_swiotController = new SwiotController(m_param, this);
	m_runtime = new SwiotRuntime();
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
	m_icon->setStyleSheet("border-image: url(:/icons/scopy-light/icons/locked.svg);");
	return true;
}

void SWIOTPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("0", "Config", ":/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("1", "AD74413R",":/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("2", "MAX14906", ":/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("3", "Faults", ":/icons/scopy-default/icons/tool_debugger.svg"));
}

void SWIOTPlugin::unload()
{
	delete infoui;
}

bool SWIOTPlugin::compatible(QString m_param)
{
	m_name = "SWIOT";
	bool ret = false;
	auto &&cp = ContextProvider::GetInstance();

	QString hw_serial;
	iio_context* ctx = cp->open(m_param);

	if(!ctx) {
		qWarning(CAT_SWIOT) << "No context available for swiot";
		return false;
	}

	hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
	if(!hw_serial.isEmpty())
		ret = true;


	unsigned int devices_count = iio_context_get_devices_count(ctx);
	if (devices_count == 2) {
		iio_device* device0 = iio_context_get_device(ctx, 0);
		iio_device* device1 = iio_context_get_device(ctx, 1);

		std::string device0_name = iio_device_get_name(device0);
		std::string device1_name = iio_device_get_name(device1);

		if ((device0_name == "ad74413r" && device1_name == "max14906") || (device0_name == "max14906" && device1_name == "ad74413r")) {
			ret = true;
		}
	} else {
		ret = false;
	}


	cp->close(m_param);

	return ret;
}

bool SWIOTPlugin::onConnect()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context* ctx = cp->open(m_param);

	m_swiotController->connectSwiot(ctx);
//	m_swiotController->startPingTask();

//	ping = new IIOPingTask(ctx);
//	cs = new CyclicalTask(ping,this);
//	cs->start(2000);

//	connect(ping, &IIOPingTask::pingFailed, this, [this](){Q_EMIT disconnectDevice();} );
//	connect(ping, &IIOPingTask::pingSuccess, this, [](){qDebug(CAT_SWIOT)<<"Ping Success";} );

	m_runtime->setContext(ctx);

	config = new swiot::SwiotConfig(ctx);
	adtool = new swiot::Ad74413r(ctx, channel_function);
	faults = new swiot::Faults(ctx);
	maxtool = new swiot::Max14906(ctx);

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(config);

	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setTool(adtool);

	m_toolList[2]->setEnabled(true);
	m_toolList[2]->setTool(maxtool);

	m_toolList[3]->setEnabled(true);
	m_toolList[3]->setTool(faults);

	if (!m_runtime->isRuntimeCtx()) {
		m_toolList[1]->setEnabled(false);
		m_toolList[2]->setEnabled(false);
		m_toolList[3]->setEnabled(false);
	} else {
		m_toolList[0]->setEnabled(false);
	}

	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);
	connect(m_swiotController, &SwiotController::pingSuccess, this, [=](){
		qDebug(CAT_SWIOT_CONFIG)<<"Ping Success";
//		m_swiotController->stopPingTask();
	} );

	connect(m_swiotController, &SwiotController::contextSwitched, this,[=](){
		delete config;
		delete adtool;
		delete faults;
		delete maxtool;
		cp->close(m_param);
		onConnect();

	});
	connect(dynamic_cast<Ad74413r*> (adtool), &Ad74413r::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Max14906*> (maxtool), &Max14906::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);
	connect(dynamic_cast<Faults*> (faults), &Faults::backBtnPressed, m_runtime, &SwiotRuntime::onBackBtnPressed);

//	connect(dynamic_cast<Ad74413r*> (adtool), &Ad74413r::backBtnPressed, this, [](){
//		qDebug(CAT_SWIOT_CONFIG) << "Back from ad";
//	});
//	connect(dynamic_cast<Max14906*> (maxtool), &Max14906::backBtnPressed, this, [](){
//		qDebug(CAT_SWIOT_CONFIG) << "Back from max";
//	});
//	connect(dynamic_cast<Faults*> (faults), &Faults::backBtnPressed, this, [](){
//		qDebug(CAT_SWIOT_CONFIG) << "Back from faults";
//	});

	connect(dynamic_cast<SwiotConfig*> (config), &SwiotConfig::configBtn, this, [=](QVector<QStringList*> funcAvailable) {
		QVector<QString> adConfigFunc = funcAvailable[0]->toVector();
		for (int i = 0; i < adConfigFunc.size(); i++){
			m_chnlsFunction[i] = adConfigFunc[i];
		}
//		m_swiotController->startSwitchContextTask();
		Q_EMIT requestTool(m_toolList[1]->id());
	});
	connect(m_runtime, &SwiotRuntime::backBtnPressed, this, [=]() {
//		m_swiotController->startSwitchContextTask();
		Q_EMIT requestTool(m_toolList[0]->id());
	});
	return true;
}

bool SWIOTPlugin::onDisconnect()
{
//	cs->stop();

        for (auto & tool : m_toolList) {
                tool->setEnabled(false);
                tool->setTool(nullptr);
        }

	disconnect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);
	disconnect(m_swiotController, &SwiotController::contextSwitched, this, &SWIOTPlugin::onConnect);

//	m_swiotController->stopPingTask();
//	m_swiotController->stopSwitchContextTask();
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
