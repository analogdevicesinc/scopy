#include "datamonitorplugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <dmm.hpp>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <timemanager.hpp>
#include <datamonitorutils.hpp>

#include <libm2k/analog/dmm.hpp>

#include <iioutil/connectionprovider.h>

#include <pluginbase/preferences.h>
#include <pluginbase/preferenceshelper.h>

Q_LOGGING_CATEGORY(CAT_DATAMONITORPLUGIN, "DataMonitorPlugin")
using namespace scopy::datamonitor;

bool DataMonitorPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	m_name = "DataMonitor";
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qWarning(CAT_DATAMONITOR) << "No context available for datalogger";
		return false;
	}

	cp->close(m_param);

	return true;
}

bool DataMonitorPlugin::loadPage()
{
	// Here you must write the code for the plugin info page
	// Below is an example for an iio device
	/*m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_DATAMONITORPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool DataMonitorPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/scopy-light/icons/unlocked.svg");
	return true;
}

void DataMonitorPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("DataMonitorPreview", "DataMonitor",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void DataMonitorPlugin::unload()
{ /*delete m_infoPage;*/
}

bool DataMonitorPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	iio_context *ctx = conn->context();

	DMM dmm;
	QList<DmmDataMonitorModel *> availableDmmList = dmm.getDmmMonitors(ctx);

	foreach(auto dmmModel, availableDmmList) {
		dmmList.push_back(dmmModel);
	}

	m_dataAcquisitionManager = new DataAcquisitionManager(this);

	foreach(ReadableDataMonitorModel *monitor, dmmList) {
		m_dataAcquisitionManager->getDataMonitorMap()->insert(monitor->getName(), monitor);
	}

	Preferences *p = Preferences::GetInstance();

	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setTimerInterval(p->get("datamonitorplugin_read_interval").toDouble());

	connect(timeTracker, &TimeManager::timeout, m_dataAcquisitionManager, &DataAcquisitionManager::readData);

	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("datamonitorplugin_read_interval")) {
			timeTracker->setTimerInterval(p->get("datamonitorplugin_read_interval").toDouble());
		}
	});

	removeTool("DataMonitorPreview");
	addNewTool();

	return true;
}

bool DataMonitorPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		removeTool(tool->id());
	}

	// add proxy tool to represent the plugin
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("DataMonitorPreview", "DataMonitor",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));

	Q_EMIT toolListChanged();
	return true;
}

void DataMonitorPlugin::addNewTool()
{
	static int i = 0;
	QString tool_name = (QString("DataMonitor ") + QString::number(i));

	ToolMenuEntry *toolMenuEntry =
		SCOPY_NEW_TOOLMENUENTRY(tool_name, tool_name, ":/gui/icons/scopy-default/icons/gear_wheel.svg");
	m_toolList.append(toolMenuEntry);
	m_toolList.last()->setEnabled(true);
	m_toolList.last()->setRunBtnVisible(true);

	bool isDeletable = m_toolList.length() > 1;
	DatamonitorTool *datamonitorTool = new DatamonitorTool(m_dataAcquisitionManager, isDeletable);

	connect(datamonitorTool, &DatamonitorTool::requestNewTool, this, &DataMonitorPlugin::addNewTool);
	connect(datamonitorTool, &DatamonitorTool::runToggled, this, &DataMonitorPlugin::toggleRunState);

	connect(datamonitorTool, &DatamonitorTool::requestDeleteTool, this, [=, this]() {
		// make sure at least one tool exists
		if(m_toolList.length() > 1) {
			removeTool(tool_name);
			requestTool(m_toolList.first()->id());
		}
	});
	connect(datamonitorTool, &DatamonitorTool::settingsTitleChanged, this,
		[=, this](QString newTitle) { toolMenuEntry->setName(newTitle); });

	datamonitorTool->getRunButton()->setChecked(isRunning);

	// one for each
	connect(toolMenuEntry, &ToolMenuEntry::runToggled, this, [=, this](bool en) {
		if(datamonitorTool->getRunButton()->isChecked() != en) {
			datamonitorTool->getRunButton()->toggle();
		}
	});

	Q_EMIT toolListChanged();
	m_toolList.last()->setTool(datamonitorTool);
	if(m_toolList.length() > 1) {
		requestTool(tool_name);
	}

	i++;
}

void DataMonitorPlugin::removeTool(QString toolId)
{

	auto *tool = ToolMenuEntry::findToolMenuEntryById(m_toolList, toolId);
	m_toolList.removeOne(tool);
	QWidget *datamonitorTool = tool->tool();
	if(datamonitorTool) {
		delete datamonitorTool;
	}

	Q_EMIT toolListChanged();

	delete tool;
}

void DataMonitorPlugin::toggleRunState(bool toggled)
{
	for(int i = 0; i < m_toolList.length(); i++) {
		m_toolList[i]->setRunning(toggled);
	}
	isRunning = toggled;
}

void DataMonitorPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
		  "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

void DataMonitorPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("datamonitorplugin_data_storage_size", "10 Kb");
	p->init("datamonitorplugin_read_interval", "1");
	p->init("datamonitorplugin_date_time_format", "hh:mm:ss");
}

bool DataMonitorPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection =
		new MenuCollapseSection("General", MenuCollapseSection::MHCW_NONE, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->setMargin(0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "datamonitorplugin_data_storage_size", "Maximum data stored for each monitor", {"10 Kb", "1 Mb"},
		generalSection));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceEdit(
		p, "datamonitorplugin_read_interval", "Read interval (seconds) ", generalSection));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceEdit(
		p, "datamonitorplugin_date_time_format", "DateTime format :", generalSection));
	return true;
}

QString DataMonitorPlugin::description() { return "Use IIO raw and scale attributes to plot and save data"; }
