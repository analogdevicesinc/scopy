#include "swiotplugin.h"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include "ui_swiotInfoPage.h"
#include "ui_swiotconfigui.h"
#include "ui_swiotruntime.h"
#include <QUuid>

using namespace adiscope;

bool SWIOTPlugin::load(QString uri)
{
	m_uri = uri;
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/icons/scopy-light/icons/locked.svg);");


	infoui = new Ui::SWIOTInfoPage();

	m_page = new QWidget();
	infoui->setupUi(m_page);
	connect(infoui->pushButton,&QPushButton::clicked,this,[=](){
		auto &&cp = ContextProvider::GetInstance();
		iio_context* ctx = cp->open(uri);
		QString hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));

		cp->close(uri);
		infoui->textBrowser->setText(hw_serial);
	});
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	return true;
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

	hw_serial = QString(iio_context_get_attr_value(ctx,"hw_serial"));
	if(hw_serial == "104473b04a060009f8ff2000da6084d36f")
		ret = true;

	cp->close(uri);

	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"SWIOT Config","",this));
	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"SWIOT Runtime","",this));
	return ret;
}

bool SWIOTPlugin::connectDev()
{

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

bool SWIOTPlugin::disconnectDev()
{
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


