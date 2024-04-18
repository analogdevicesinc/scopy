#include "hockeypucktempsensorplugin.h"
#include <iioutil/connectionprovider.h>

#include <QLoggingCategory>
#include <QLabel>

#include "hpts.h"

Q_LOGGING_CATEGORY(CAT_HOCKEYPUCKTEMPSENSORPLUGIN, "HockeyPuckTempSensorPlugin")
using namespace scopy::HockeyPuckTempSensor;

bool HockeyPuckTempSensorPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = false;
	qDebug(CAT_HOCKEYPUCKTEMPSENSORPLUGIN) << "compatible";

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return ret;

	iio_context *ctx = conn->context();
	QString description(iio_context_get_description(ctx));
	if(description.contains(COMPATIBLE_DESCRIPTION)) {
		ret = true;
	}

	return ret;
}

bool HockeyPuckTempSensorPlugin::loadPage()
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
			qWarning(CAT_HOCKEYPUCKTEMPSENSORPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool HockeyPuckTempSensorPlugin::loadIcon()
{
	// SCOPY_PLUGIN_ICON(":/gui/icons/scopy-default/icons/logo_analog.svg");
	// m_icon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	SCOPY_PLUGIN_ICON(":/gui/icons/scopy-default/icons/info.svg");
	return true;
}

void HockeyPuckTempSensorPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("HPTS", "Temperature Sensor",
						  ":/gui/icons/scopy-default/icons/tool_voltmeter.svg"));
}

void HockeyPuckTempSensorPlugin::unload()
{ /*delete m_infoPage;*/
}

QString HockeyPuckTempSensorPlugin::description() { return "Hockey Puck temperature sensor"; }

QString HockeyPuckTempSensorPlugin::displayName() { return "Hockey Puck"; }

bool HockeyPuckTempSensorPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();

	HockeyPuckTempSensor *hockeyPuckTempSensor = new HockeyPuckTempSensor(m_ctx);

	m_toolList[0]->setTool(hockeyPuckTempSensor);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	connect(m_toolList[0], &ToolMenuEntry::runToggled, hockeyPuckTempSensor, &HockeyPuckTempSensor::run);
	m_toolList[0]->setRunning(true);
	return true;
}

bool HockeyPuckTempSensorPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	qDebug(CAT_HOCKEYPUCKTEMPSENSORPLUGIN) << "disconnect";
	if(m_ctx)
		ConnectionProvider::GetInstance()->close(m_param);

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
	return true;
}

void HockeyPuckTempSensorPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":110,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*", "!debuggerplugin"]
	}
)plugin");
}
