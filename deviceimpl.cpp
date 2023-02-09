#include "deviceimpl.h"
#include <QLabel>
#include <QTextBrowser>
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_DEVICEIMPL, "DeviceImplementation")

DeviceImpl::DeviceImpl(QString uri, QObject *parent)
	: QObject{parent},
	  m_uri(uri)
{
	m_icon = new QLabel("ICON");
	m_page = new QTextEdit("Edit text " + uri);
	qDebug()<< m_uri <<"ctor";
}

DeviceImpl::~DeviceImpl() {
	qDebug()<< m_uri <<"dtor";
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

QStringList DeviceImpl::toolList()
{
	return QStringList() << "tool1" << "tool2" << "tool3";
}
