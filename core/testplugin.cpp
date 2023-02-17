#include "testplugin.h"
#include "qlabel.h"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"TestPlugin");
using namespace adiscope;
TestPlugin::TestPlugin(QObject *parent) : Plugin(parent)
{
	m_name = "TestPlugin";
	qDebug(CAT_TESTPLUGIN)<<"ctor";
}

TestPlugin::~TestPlugin()
{
	qDebug(CAT_TESTPLUGIN)<<"dtor";
}

bool TestPlugin::compatible(QString uri) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}

bool TestPlugin::load(QString uri) {
	m_uri = uri;
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/icons/adalm.svg);");
	m_page = new QLabel("TestPage");	
	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"2","",this));
	return true;
}

bool TestPlugin::connectDev()
{
	qDebug(CAT_TESTPLUGIN)<<"connect";
	for(auto &&tool:m_toolList) {
	qDebug(CAT_TESTPLUGIN)<<tool->id()<<tool->name();
	}
	return true;
}

bool TestPlugin::disconnectDev()
{
	qDebug(CAT_TESTPLUGIN)<<"disconnect";
	return true;
}
