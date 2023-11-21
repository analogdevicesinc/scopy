#include "datamonitor.h"
#include "pluginbase/scopyjs.h"
#include "qloggingcategory.h"

#include <iio.h>

#include <QDebug>
#include <QLabel>
#include <QUuid>
#include <QVBoxLayout>

#include <libm2k/contextbuilder.hpp>
#include "datamonitortool.hpp"

using namespace scopy;
using namespace datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR, "DataMonitorPlugin");
Q_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL, "DataMonitorTool");

bool DataMonitorPlugin::loadPage()
{
	m_page = new QWidget();
	return true;
}

bool DataMonitorPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/gui/icons/scopy-light/icons/unlocked.svg);");
	return true;
}

void DataMonitorPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("datamonitor", "DataMonitor",
						  ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void DataMonitorPlugin::unload() {}

bool DataMonitorPlugin::compatible(QString param, QString cateogory)
{
	m_name = "DataMonitor";
	ContextProvider *cp = ContextProvider::GetInstance();

	iio_context *ctx = cp->open(param);

	if(!ctx) {
		qWarning(CAT_DATAMONITOR) << "No context available for datamonitor";
		return false;
	}

	cp->close(param);

	return true;
}

bool DataMonitorPlugin::onConnect()
{
	ContextProvider *cp = ContextProvider::GetInstance();
	iio_context *ctx = cp->open(m_param);

	// TODO get all DMM devices

	QList<iio_device *> *m_deviceList = new QList<iio_device *>();

	auto deviceCount = iio_context_get_devices_count(ctx);

	for(int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		m_deviceList->push_back(dev);
	}

	tool = new DataMonitorTool();

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setRunBtnVisible(true);

	connect(ping, &IIOPingTask::pingFailed, this, [this]() { Q_EMIT disconnectDevice(); });
	connect(ping, &IIOPingTask::pingSuccess, this, []() { qDebug(CAT_DATAMONITOR) << "Ping Success"; });
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

bool DataMonitorPlugin::onDisconnect()
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
		qDebug(CAT_DATAMONITOR) << ex.what();
	}

	auto &&cp = ContextProvider::GetInstance();
	cp->close(m_param);

	return true;
}

void DataMonitorPlugin::initMetadata()
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

void DataMonitorPlugin::saveSettings(QSettings &s) {}

void DataMonitorPlugin::loadSettings(QSettings &s) {}

QString DataMonitorPlugin::description() { return "Use IIO raw and scale attributes to plot and save data"; }
