#include "plutoplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include "ad963x.h"

#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_PLUTOPLUGIN, "PlutoPlugin")
using namespace scopy::pluto;

bool PlutoPlugin::compatible(QString m_param, QString category)
{

	qDebug(CAT_PLUTOPLUGIN) << "Check Pluto compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_PLUTOPLUGIN) << "No context available for Pluto";
		return false;
	}

	// TODO Find better connection condition
	iio_device *plutoDevice = iio_context_find_device(conn->context(), "ad9361-phy");
	if(plutoDevice) {
		ret = true;
	}

	ConnectionProvider::close(m_param);

	return ret;
}

bool PlutoPlugin::loadPage()
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
			qWarning(CAT_PLUTOPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();
	return true;
	*/
	return false;
}

bool PlutoPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void PlutoPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("ad963xTool", "AD936X", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void PlutoPlugin::unload() { /*delete m_infoPage;*/ }

QString PlutoPlugin::description() { return "This is a plugin for AD936X"; }

bool PlutoPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is 
	// compatible to that device 
	// In case of success the function must return true and false otherwise 
    	
	AD936X *aD936X = new AD936X();
	m_toolList[0]->setTool(aD936X);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
    	return true;
}

bool PlutoPlugin::onDisconnect()
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
	return true;
}

void PlutoPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["m2kplugin"]
	}
)plugin");
}
