#include "testpluginip.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(CAT_TESTPLUGINIP,"TestPluginIp");
using namespace adiscope;

bool TestPluginIp::compatible(QString uri) {
	qDebug(CAT_TESTPLUGINIP)<<"compatible";
	return uri.startsWith("ip:");
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

void TestPluginIp::postload()
{

}

bool TestPluginIp::loadIcon()
{
	static int count = 0;
	m_icon = new QLabel(QString::number(count));
	count++;
	m_icon->setStyleSheet("border-image: url(:/icons/adalm.svg);");
	return true;
}

bool TestPluginIp::loadPage()
{
	m_page = new QLabel("TestPageIP");
	return true;
}

void TestPluginIp::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("SecondPlugin",""));
}

void TestPluginIp::initMetadata()
{
	loadMetadata(
R"plugin(
	{
	   "priority":2,
	   "category":[
	      "test"
	   ]
	}
)plugin");
}
