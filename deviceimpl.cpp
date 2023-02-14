#include "deviceimpl.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include <QLabel>
#include <QTextBrowser>
#include <QLoggingCategory>
#include <QDebug>
#include <QPicture>

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "DeviceImplementation")

namespace adiscope {
DeviceImpl::DeviceImpl(QString uri, QObject *parent)
	: QObject{parent},
	  m_uri(uri)
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/icons/adalm.svg);");
//	m_icon->setFixedHeight(100);
//	m_icon->setFixedWidth(100);

	m_page = new QWidget();
	connbtn = new QPushButton("connect", m_page);
	connbtn->setProperty("blue_button",true);
	discbtn = new QPushButton("disconnect", m_page);
	discbtn->setProperty("blue_button",true);
//	extraToolchkbox = new QCheckBox("extra tool",m_page);
	discbtn->setVisible(false);

	auto layout = new QHBoxLayout(m_page);
	layout->addWidget(connbtn);
	layout->addWidget(discbtn);
	connect(connbtn,SIGNAL(clicked()),this,SLOT(connectDev()));
	connect(discbtn,SIGNAL(clicked()),this,SLOT(disconnectDev()));

	// this is a little wonky but could prove useful ...
//	connect(extraToolchkbox,SIGNAL(stateChanged(int)),this,SIGNAL(toolListChanged()));


	qDebug(CAT_DEVICEIMPL)<< m_uri <<"ctor";
}

void DeviceImpl::connectDev() {
	discbtn->show();
	connbtn->hide();
	Q_EMIT connected();
}

void DeviceImpl::disconnectDev() {
	discbtn->hide();
	connbtn->show();
	Q_EMIT disconnected();
}

DeviceImpl::~DeviceImpl() {
	qDebug(CAT_DEVICEIMPL)<< m_uri <<"dtor";
}

QString DeviceImpl::name()
{
	return "name";
}

QString DeviceImpl::uri()
{
	return m_uri;
}

QWidget *DeviceImpl::icon()
{
	return m_icon;
}

QWidget *DeviceImpl::page()
{
	return m_page;
}

QList<ToolMenuEntry> DeviceImpl::toolList()
{
	static int i;
	QList<ToolMenuEntry> ret;
	ret.append({"tool1"+uri(),"tool1Name"+QString::number(i),""});
	ret.append({"tool2"+uri(),"tool2Name"+QString::number(i),""});
	ret.append({"tool3"+uri(),"tool3Name"+QString::number(i),""});
//	if(extraToolchkbox->isChecked()) {
//		ret.append({"extratool"+uri(),"extraTool"+QString::number(i),""});
//	}
	i++;


	return ret;
}
}
