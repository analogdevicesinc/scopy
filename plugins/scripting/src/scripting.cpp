#include "scripting.h"

#include "scopy-scripting_config.h"
#include "scriptingtool.h"
#include <QBoxLayout>
#include <QLabel>
#include <style.h>
#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_SCRIPTINGPLUGIN, "ScriptingPlugin")

using namespace scopy;
using namespace scripting;

bool Scripting::compatible(QString m_param, QString category)
{
	m_name = SCRIPTING_PLUGIN_DESCRIPTION;
	// TODO check if any api pluging available ?
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qWarning(CAT_SCRIPTINGPLUGIN) << "No context available for datalogger";
		return false;
	}

	cp->close(m_param);
	return true;
}

bool Scripting::loadPage() { return false; }

bool Scripting::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
			      "/icons/RegMap.svg);");
	return true;
}

void Scripting::loadToolList()
{
	ToolMenuEntry *toolMenuEntry = SCOPY_NEW_TOOLMENUENTRY(
		SCRIPTING_PLUGIN_SCOPY_MODULE, SCRIPTING_PLUGIN_DISPLAY_NAME,
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/tool_calibration.svg");
	m_toolList.append(toolMenuEntry);
	m_toolList.last()->setRunBtnVisible(true);
	m_toolList.last()->setRunEnabled(false);

	Q_EMIT toolListChanged();
}

void Scripting::unload() {}

void Scripting::initMetadata()
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

QString Scripting::description() { return SCRIPTING_PLUGIN_DESCRIPTION; }

QWidget *Scripting::getTool() { return m_scriptingWidget; }

bool Scripting::onConnect()
{
	m_scriptingWidget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_scriptingWidget);
	m_scriptingWidget->setLayout(layout);
	layout->addWidget(new ScriptingTool(m_scriptingWidget));

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(m_scriptingWidget);

	for(auto &tool : m_toolList) {
		tool->setEnabled(true);
		tool->setRunBtnVisible(true);
	}

	Q_EMIT toolListChanged();

	return true;
}

bool Scripting::onDisconnect()
{

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		tme->tool()->deleteLater();
		tme->setTool(nullptr);
	}

	Q_EMIT toolListChanged();

	return true;
}
