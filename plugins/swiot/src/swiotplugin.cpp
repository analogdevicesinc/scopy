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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("0", "SWIOT Config", ":/icons/scopy-default/icons/tool_debugger.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("1", "SWIOT Runtime",""));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("2", "SWIOT MAX14906", ""));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("3", "SWIOT Faults", ""));
}

void SWIOTPlugin::unload()
{
	delete infoui;
}

bool SWIOTPlugin::compatible(QString m_param)
{
	m_name = "SWIOT";
	bool ret = true;
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

	config = new swiot::SwiotConfig(ctx);
	runtime = new SwiotRuntime(ctx);
	faults = new swiot::Faults(ctx);
	maxtool = new swiot::Max14906(ctx);

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(config);

	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setTool(runtime);

	m_toolList[2]->setEnabled(true);
	m_toolList[2]->setTool(maxtool);

	m_toolList[3]->setEnabled(true);
	m_toolList[3]->setTool(faults);



	connect(m_swiotController, &SwiotController::pingFailed, this, &SWIOTPlugin::disconnectDevice);
	connect(m_swiotController, &SwiotController::pingSuccess, this, [=](){
		qDebug(CAT_SWIOT_CONFIG)<<"Ping Success";
//		m_swiotController->stopPingTask();
	} );

	connect(m_swiotController, &SwiotController::contextSwitched, this,[=](){
		delete config;
		delete runtime;
		delete faults;
		delete maxtool;
		cp->close(m_param);
		onConnect();

	});
	connect(dynamic_cast<SwiotConfig*> (config), &SwiotConfig::configBtn, this, [=](QVector<QStringList*> funcAvailable) {
		for(int i = 0; i < funcAvailable.size(); i++) {
			qDebug(CAT_SWIOT_CONFIG) << funcAvailable[i]->toVector();
		}
//		m_swiotController->startSwitchContextTask();
		Q_EMIT requestTool(m_toolList[1]->id());
	});
	connect(dynamic_cast<SwiotRuntime*> (runtime), &SwiotRuntime::backBtnPressed, this, [=]() {
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
	m_swiotController->disconnectSwiot();

	delete config;
	delete runtime;
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
