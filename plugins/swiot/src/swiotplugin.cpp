#include "swiotplugin.h"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include "ui_swiotInfoPage.h"
#include "ui_swiotconfigui.h"
#include "ui_swiotruntime.h"
#include <QUuid>

#include "iioutil/contextprovider.h"

using namespace adiscope;

Q_LOGGING_CATEGORY(CAT_SWIOT,"SWIOTPlugin");

bool SWIOTPlugin::loadPage()
{
	infoui = new Ui::SWIOTInfoPage();
	m_page = new QWidget();
	infoui->setupUi(m_page);
	connect(infoui->pushButton,&QPushButton::clicked,this,[=](){
		auto &&cp = ContextProvider::GetInstance();
		iio_context* ctx = cp->open(m_uri);
		QString hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
		cp->close(m_uri);
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("SWIOT Config",""));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("SWIOT Runtime",""));
}

void SWIOTPlugin::unload()
{

	delete infoui;
}

bool SWIOTPlugin::compatible(QString uri)
{
	m_name="SWIOT";
	m_uri = uri;
	bool ret = false;
	auto &&cp = ContextProvider::GetInstance();

	QString hw_serial;
	iio_context* ctx = cp->open(uri);

	if(!ctx)
		return false;

	hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
	if(!hw_serial.isEmpty())
		ret = true;

	cp->close(uri);

	return ret;
}

bool SWIOTPlugin::onConnect()
{
	auto &&cp = ContextProvider::GetInstance();
	iio_context* ctx = cp->open(m_uri);

	ping = new IIOPingTask(ctx);
	cs = new CyclicalTask(ping,this);
	cs->start(2000);

	connect(ping, &IIOPingTask::pingFailed, this, [=](){Q_EMIT disconnectDevice();} );
	connect(ping, &IIOPingTask::pingSuccess, this, [=](){qDebug(CAT_SWIOT)<<"Ping Success";} );

	config = new QWidget();
	configui = new Ui::SWIOTConfigTool();
	configui->setupUi(config);
	runtime = new QWidget();
	rungui = new Ui::SWIOTRuntime();
	rungui->setupUi(runtime);

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(config);

//	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setTool(runtime);

	connect(configui->pushButton,&QPushButton::clicked,this,[=](){
		m_toolList[0]->setEnabled(false);
		m_toolList[1]->setEnabled(true);
		Q_EMIT requestTool(m_toolList[1]->id());
	});
	connect(rungui->pushButton,&QPushButton::clicked,this,[=](){
		m_toolList[0]->setEnabled(true);
		m_toolList[1]->setEnabled(false);
		Q_EMIT requestTool(m_toolList[0]->id());
	});

	return false;
}

bool SWIOTPlugin::onDisconnect()
{
	cs->stop();
	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_uri);


	m_toolList[0]->setEnabled(false);
	m_toolList[0]->setTool(nullptr);

	m_toolList[1]->setEnabled(false);
	m_toolList[1]->setTool(nullptr);

	delete configui;
	delete rungui;
	delete config;
	delete runtime;

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


