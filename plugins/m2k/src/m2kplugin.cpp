#include "m2kplugin.h"
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

bool M2kPlugin::compatible(QString m_param) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}


void M2kPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("MinimumTool",":/icons/scopy-default/icons/tool_home.svg"));
}

bool M2kPlugin::onConnect()
{
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(new QLabel("Minimum"));
	return true;
}

bool M2kPlugin::onDisconnect()
{
	m_toolList[0]->setEnabled(false);
	m_toolList[0]->setTool(nullptr);
	return true;
}



void M2kPlugin::initMetadata() // not actually needed - putting it here to set priority
{
	loadMetadata(
R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}



#include "moc_m2kplugin.cpp"
