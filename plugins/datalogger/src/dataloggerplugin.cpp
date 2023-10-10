#include "dataloggerplugin.h"

#include "datalogger_api.h"
#include "pluginbase/scopyjs.h"
#include "qloggingcategory.h"

#include <iio.h>

#include <QDebug>
#include <QLabel>
#include <QUuid>
#include <QVBoxLayout>

#include <libm2k/contextbuilder.hpp>

using namespace scopy;
using namespace datalogger;

Q_LOGGING_CATEGORY(CAT_DATALOGGER, "DataLoggerPlugin");
Q_LOGGING_CATEGORY(CAT_DATALOGGER_TOOL, "dataloggerTool");

bool DataLoggerPlugin::loadPage()
{
	m_page = new QWidget();
	return true;
}

bool DataLoggerPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/gui/icons/scopy-light/icons/unlocked.svg);");
	return true;
}

void DataLoggerPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("datalogger", "Datalogger",
						  ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void DataLoggerPlugin::unload() {}

bool DataLoggerPlugin::compatible(QString param, QString cateogory)
{
	m_name = "Datalogger";
	ContextProvider *cp = ContextProvider::GetInstance();

	iio_context *ctx = cp->open(param);

	if(!ctx) {
		qWarning(CAT_DATALOGGER) << "No context available for datalogger";
		return false;
	}

	cp->close(param);

	return true;
}

bool DataLoggerPlugin::onConnect()
{
	ContextProvider *cp = ContextProvider::GetInstance();
	iio_context *ctx = cp->open(m_param);
	libm2k_context = libm2k::context::contextOpen(ctx, "");
	ping = new IIOPingTask(ctx);
	cs = new CyclicalTask(ping, this);
	cs->start(2000);

	tool = new DataLogger(libm2k_context);
	api = new DataLogger_API(tool);
	ScopyJS::GetInstance()->registerApi(api);

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setRunBtnVisible(true);

	connect(ping, &IIOPingTask::pingFailed, this, [this]() { Q_EMIT disconnectDevice(); });
	connect(ping, &IIOPingTask::pingSuccess, this, []() { qDebug(CAT_DATALOGGER) << "Ping Success"; });
	connect(tool->getRunButton(), &QPushButton::toggled, this, [=](bool en) {
		if(m_toolList[0]->running() != en) {
			m_toolList[0]->setRunning(en);
		}
	});
	connect(m_toolList[0], &ToolMenuEntry::runToggled, this, [=](bool en) {
		if(tool->getRunButton()->isChecked() != en) {
			tool->getRunButton()->setChecked(en);
		}
	});

	return true;
}

bool DataLoggerPlugin::onDisconnect()
{
	delete tool;

	cs->stop();
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setTool(nullptr);
		m_toolList[0]->setRunBtnVisible(false);
	}

	try {
		contextClose(libm2k_context, true);
	} catch(std::exception &ex) {
		qDebug(CAT_DATALOGGER) << ex.what();
	}

	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_param);

	return true;
}

void DataLoggerPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":16,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}

void DataLoggerPlugin::saveSettings(QSettings &s) { api->save(s); }

void DataLoggerPlugin::loadSettings(QSettings &s) { api->load(s); }

QString DataLoggerPlugin::description() { return "Use IIO raw and scale attributes to plot and save data"; }

#include "moc_dataloggerplugin.cpp"
