#include "guitestplugin.h"
#include <QBoxLayout>
#include <QLoggingCategory>
#include <QSpacerItem>
#include <QLabel>

Q_LOGGING_CATEGORY(CAT_GUITESTPLUGIN,"GUITestPlugin");
using namespace scopy;

bool TestPlugin::compatible(QString m_param) {
	return (m_param=="gui");
}

bool TestPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/guitestplugin/icon.png");
	return true;
}

bool TestPlugin::loadPage()
{
	m_page = new QWidget();
	return true;

}

void TestPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("Buttons","Buttons",":/gui/icons/scopy-default/icons/tool_home.svg"));
}

bool TestPlugin::onConnect()
{
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	tool = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(tool);
	QLabel *lbl = new QLabel("TestPlugin", tool);
	lay->addWidget(lbl);
	lay->addSpacerItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Expanding));



	m_toolList[0]->setTool(tool);

	return true;
}


bool TestPlugin::onDisconnect()
{
	qDebug(CAT_GUITESTPLUGIN)<<"disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}
	return true;
}

void TestPlugin::initMetadata()
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

#include "moc_guitestplugin.cpp"
