#include "bareminimum.h"
#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QLoggingCategory>
#include <QUuid>
#include <QSpacerItem>
#include <pluginbase/preferences.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferenceshelper.h>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"BareMinimum");
using namespace adiscope;

bool BareMinimum::compatible(QString m_param) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}


void BareMinimum::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("bareminimum_tool","MinimumTool",":/icons/scopy-default/icons/tool_home.svg"));
}

bool BareMinimum::onConnect()
{
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(new QLabel("Minimum"));
	return true;
}

bool BareMinimum::onDisconnect()
{
	m_toolList[0]->setEnabled(false);
	m_toolList[0]->setTool(nullptr);
	return true;
}



void BareMinimum::initMetadata() // not actually needed - putting it here to set priority
{
	loadMetadata(
R"plugin(
	{
	   "priority":-255,
	   "category":[
	      "test"
	   ]
	}
)plugin");
}



#include "moc_bareminimum.cpp"
