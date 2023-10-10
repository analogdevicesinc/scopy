#include "debuggerplugin.h"

#include "debuggerinstrument.h"

#include <iio.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QLabel>
#include <QLoggingCategory>
#include <QVBoxLayout>

#include <core/detachedtoolwindow.h>
#include <core/detachedtoolwindowmanager.h>
#include <iioutil/contextprovider.h>

using namespace scopy;
using namespace scopy::debugger;

Q_LOGGING_CATEGORY(CAT_DEBUGGERPLUGIN, "DEBUGGERPLUGIN");
Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark");

bool DebuggerPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DEBUGGERPLUGIN) << " compatible";
	bool ret = true;
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);

	if(!ctx) {
		return false;
	}
	c->close(m_param);
	return ret;
}

void DebuggerPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("debugger", "Debugger", ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
	ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger")->setVisible(true);
}

QString DebuggerPlugin::description() { return "IIO context explorer tool"; }

bool DebuggerPlugin::onConnect()
{
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);
	if(!ctx) {
		return false;
	}
	auto dbgTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger");
	dbgTme->setTool(new DebuggerInstrument(ctx, nullptr, nullptr));
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

	ContextProvider *c = ContextProvider::GetInstance();
	c->close(m_param);
	return true;
}

bool DebuggerPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	lay->addWidget(new QLabel("IIO Debugger plugin", m_page));
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
