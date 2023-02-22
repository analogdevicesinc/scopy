#include "testplugin.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"TestPlugin");
using namespace adiscope;

bool TestPlugin::compatible(QString uri) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}

bool TestPlugin::load(QString uri) {
	qDebug(CAT_TESTPLUGIN)<<"Loaded plugin";
	m_uri = uri;
	SCOPY_PLUGIN_ICON(":/icons/adalm.svg");

	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	lay->addWidget(new QLabel("TestPage",m_page));
	QPushButton* restartBtn = new QPushButton("restartPlugin",m_page);
	lay->addWidget(restartBtn);
	connect(restartBtn,SIGNAL(clicked()),this,SIGNAL(restartDevice()));

	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("FirstPlugin",":/icons/scopy-default/icons/tool_home.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("Alexandra",":/icons/scopy-default/icons/tool_io.svg"));
	return true;
}

void TestPlugin::unload() {
}


bool TestPlugin::connectDev()
{
	qDebug(CAT_TESTPLUGIN)<<"connect";
	qDebug(CAT_TESTPLUGIN)<<m_toolList[0]->id()<<m_toolList[0]->name();

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setName("TestPlugin");
	m_toolList[0]->setRunBtnVisible(true);
	QLabel *lbl = new QLabel("TestPlugin");
	m_toolList[0]->setTool(lbl);

	return true;
}

bool TestPlugin::disconnectDev()
{
	qDebug(CAT_TESTPLUGIN)<<"disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		if(tool->tool()) {
			delete tool->tool();
			tool->setTool(nullptr);
		}
	}
	return true;
}
