#include "dataloggerplugin.hpp"
#include "qloggingcategory.h"
#include "src/tool/datalogger.hpp"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QUuid>
#include <ui_dataloggerInfoPage.h>


using namespace adiscope;

Q_LOGGING_CATEGORY(CAT_DATALOGGER,"DataLoggerPlugin");
Q_LOGGING_CATEGORY(CAT_DATALOGGER_TOOL,"dataloggerTool");

bool DataLoggerPlugin::loadPage()
{
	infoui = new Ui::DataLoggerInfoPage();
	m_page = new QWidget();
	infoui->setupUi(m_page);
	connect(infoui->pushButton,&QPushButton::clicked, this, [=, this] (){
		auto &&cp = ContextProvider::GetInstance();
		iio_context* ctx = cp->open(m_param);
		QString hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
		cp->close(m_param);
		infoui->textBrowser->setText(hw_serial);
	});
	return true;
}

bool DataLoggerPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/icons/scopy-light/icons/unlocked.svg);");
	return true;
}

void DataLoggerPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("datalogger", "Datalogger", ":/icons/scopy-default/icons/tool_debugger.svg"));
}

void DataLoggerPlugin::unload()
{
	delete infoui;
}

bool DataLoggerPlugin::compatible(QString m_param)
{
	m_name = "Datalogger";
	auto &&cp = ContextProvider::GetInstance();

	iio_context* ctx = cp->open(m_param);

	if(!ctx) {
		qWarning(CAT_DATALOGGER) << "No context available for datalogger";
		return false;
	}

	cp->close(m_param);

	return true;
}

bool DataLoggerPlugin::onConnect()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context* ctx = cp->open(m_param);

	ping = new IIOPingTask(ctx);
	cs = new CyclicalTask(ping,this);
	cs->start(2000);

	connect(ping, &IIOPingTask::pingFailed, this, [this](){Q_EMIT disconnectDevice();} );
	connect(ping, &IIOPingTask::pingSuccess, this, [](){qDebug(CAT_DATALOGGER)<<"Ping Success";} );

	tool = new DataLogger(ctx);

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(tool);

	return true;
}

bool DataLoggerPlugin::onDisconnect()
{
	cs->stop();
	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_param);

	for (auto & tool : m_toolList) {
		tool->setEnabled(false);
		tool->setTool(nullptr);
	}

//	delete tool;

	return true;
}

void DataLoggerPlugin::initMetadata()
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



#include "moc_dataloggerplugin.cpp"
#include <iioutil/contextprovider.h>
#include <iioutil/iiopingtask.h>
#include <src/tool/datalogger.hpp>
