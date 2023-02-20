#include "testpluginip.h"
#include "qlabel.h"
#include "qpushbutton.h"
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
	static int count = 0;
	m_uri = uri;
	m_icon = new QLabel(QString::number(count));
	count++;
	m_icon->setStyleSheet("border-image: url(:/icons/adalm.svg);");
	m_page = new QLabel("TestPageIP");
	m_toolList.append(new ToolMenuEntry(QUuid::createUuid().toString(),"IP","", this));
	return true;
}


void TestPluginIp::unload() {
	for(auto &tool : m_toolList) {
		delete tool;
	}

}

bool TestPluginIp::connectDev()
{
	qDebug(CAT_TESTPLUGINIP)<<"connect";
	qDebug(CAT_TESTPLUGINIP)<<m_toolList[0]->id()<<m_toolList[0]->name();
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setName("IP tool1");
	m_toolList[0]->setRunBtnVisible(true);
	m_toolList[0]->setTool(new QLabel("TestPage IP Renamed"));
	QString iptool2UUID = QUuid::createUuid().toString();

	m_toolList.append(new ToolMenuEntry(iptool2UUID,"IP tool2","", this));
	m_toolList[1]->setEnabled(true);
	QPushButton *btn = new QPushButton("LASTTOOOL testpage");
	m_tool = btn;

	connect(btn,&QPushButton::clicked,this,[=]() { Q_EMIT requestTool(m_toolList[0]->id());});

	Q_EMIT toolListChanged();
	m_toolList[1]->setTool(m_tool);

	return true;
}

bool TestPluginIp::disconnectDev()
{
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		if(tool->tool()) {
			delete tool->tool();
			tool->setTool(nullptr);
		}
	}
	m_toolList.removeLast();
	Q_EMIT toolListChanged();
	m_toolList[0]->setName("IP");

	qDebug(CAT_TESTPLUGINIP)<<"disconnect";
	return true;
}
