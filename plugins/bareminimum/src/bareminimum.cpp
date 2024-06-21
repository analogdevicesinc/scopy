#include "bareminimum.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>

#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN, "BareMinimum");
using namespace scopy;

bool BareMinimum::compatible(QString m_param, QString category)
{
	qDebug(CAT_TESTPLUGIN) << "compatible";
	return true;
}

void BareMinimum::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("bareminimum_tool", "MinimumTool",
						  ":/gui/icons/home.svg"));
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
