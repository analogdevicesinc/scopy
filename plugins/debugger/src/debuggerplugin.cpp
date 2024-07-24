#include "debuggerplugin.h"

#include "debugger/debuggerinstrument.h"
#include "iioexplorerinstrument.h"

#include <iio.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QLabel>
#include <QLoggingCategory>
#include <QVBoxLayout>

#include <core/detachedtoolwindow.h>
#include <core/detachedtoolwindowmanager.h>
#include <iioutil/connectionprovider.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::debugger;

Q_LOGGING_CATEGORY(CAT_DEBUGGERPLUGIN, "DEBUGGERPLUGIN");
Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark");

bool DebuggerPlugin::compatible(QString m_param, QString category)
{
	bool ret = true;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		return false;
	}
	ConnectionProvider::close(m_param);
	return ret;
}

void DebuggerPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("debugger", "Debugger", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger")->setVisible(true);
}

void DebuggerPlugin::unload() {}

QString DebuggerPlugin::description() { return "IIO context explorer tool"; }

QString DebuggerPlugin::version() { return "0.1"; }

void DebuggerPlugin::saveSettings(QSettings &s)
{
	if(m_useDebuggerV2) {
		m_iioDebugger->saveSettings(s);
	}
}

void DebuggerPlugin::loadSettings(QSettings &s)
{
	if(m_useDebuggerV2) {
		m_iioDebugger->loadSettings(s);
	}
}

bool DebuggerPlugin::onConnect()
{
	m_conn = ConnectionProvider::open(m_param);
	if(!m_conn) {
		return false;
	}

	ToolMenuEntry *dbgTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger");
	m_useDebuggerV2 = Preferences::get("plugins_use_debugger_v2").toBool();
	if(m_useDebuggerV2) {
		m_iioDebugger = new IIOExplorerInstrument(m_conn->context(), nullptr, nullptr);
		dbgTme->setTool(m_iioDebugger);
	} else {
		dbgTme->setTool(new DebuggerInstrument(m_conn->context(), nullptr, nullptr));
	}
	dbgTme->setEnabled(true);
	dbgTme->setRunBtnVisible(true);

	return true;
}

bool DebuggerPlugin::onDisconnect()
{
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		tme->tool()->deleteLater();
		tme->setTool(nullptr);
	}

	if(m_conn) {
		ConnectionProvider::close(m_param);
	}
	return true;
}

bool DebuggerPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	lay->addWidget(new QLabel("IIO Debugger plugin", m_page));
	return true;
}

bool DebuggerPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void DebuggerPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}

#include "moc_debuggerplugin.cpp"
