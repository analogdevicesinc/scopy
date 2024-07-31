#include "iiodebugplugin.h"
#include "debuggerloggingcategories.h"
#include <QLabel>
#include <iioutil/connectionprovider.h>
#include <pluginbase/preferences.h>

using namespace scopy::iiodebugplugin;

bool IIODebugPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	qDebug(CAT_IIODEBUGGER) << "Checking if IIODebugPlugin is compatible.";

	// Check weather this version (V2) should be used
	bool useThisDebugger = Preferences::GetInstance()->get("plugins_use_debugger_v2").toBool();
	if(!useThisDebugger) {
		return false;
	}

	bool ret = true;
	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);

	if(!conn) {
		return false;
	}
	c->close(m_param);
	qDebug(CAT_IIODEBUGGER) << "IIODebugPlugin is compatible!";
	return ret;
}

bool IIODebugPlugin::loadPage() { return false; }

bool IIODebugPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void IIODebugPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("iiodebugplugin", "IIO Debugger",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void IIODebugPlugin::unload()
{ /*delete m_infoPage;*/
}

QString IIODebugPlugin::description() { return "Tool for interacting with IIO attributes."; }

QString IIODebugPlugin::version() { return "0.1"; }

void IIODebugPlugin::saveSettings(QSettings &s)
{
	m_pluginApi->save(s);
	m_iioDebugger->saveSettings(s);
}

void IIODebugPlugin::loadSettings(QSettings &s)
{
	m_pluginApi->load(s);
	m_iioDebugger->loadSettings(s);
}

bool IIODebugPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	auto *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	m_iioDebugger = new IIODebugInstrument(conn->context(), m_param);
	m_toolList[0]->setTool(m_iioDebugger);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	m_pluginApi = new IIODebugPlugin_API(this);
	m_pluginApi->setObjectName(m_name);

	return true;
}

bool IIODebugPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}

	ConnectionProvider::GetInstance()->close(m_param);
	return true;
}

void IIODebugPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

#include "moc_iiodebugplugin.cpp"
