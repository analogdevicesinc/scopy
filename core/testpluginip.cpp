#include "testpluginip.h"
#include "qlabel.h"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(CAT_TESTPLUGINIP,"TestPluginIp");
using namespace adiscope;
TestPluginIp::TestPluginIp(QObject *parent) : Plugin(parent)
{
	m_name = "TestPluginIp";
	qDebug(CAT_TESTPLUGINIP)<<"ctor";
}

TestPluginIp::~TestPluginIp()
{
	qDebug(CAT_TESTPLUGINIP)<<"dtor";
}

bool TestPluginIp::compatible(QString uri) {
	qDebug(CAT_TESTPLUGINIP)<<"compatible";
	return uri.startsWith("ip:");
}

bool TestPluginIp::load(QString uri) {
	m_uri = uri;
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/icons/adalm.svg);");
	m_page = new QLabel("TestPageIP");
	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"IP","", this));
	return true;
}

bool TestPluginIp::connectDev()
{
	qDebug(CAT_TESTPLUGINIP)<<"connect";
	for(auto &&tool:m_toolList) {
	qDebug(CAT_TESTPLUGINIP)<<tool->id()<<tool->name();
	tool->setEnabled(true);
	tool->setName("boo");
	tool->setRunBtnVisible(true);	
	}
	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"IP222","", this));
	m_toolList.last()->setEnabled(true);
	Q_EMIT toolListChanged();
	m_toolList.last()->setTool(new QLabel("LASTTOOOL"));

	return true;
}

bool TestPluginIp::disconnectDev()
{
	qDebug(CAT_TESTPLUGINIP)<<"disconnect";
	return true;
}
