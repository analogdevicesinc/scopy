#include "iiodebugplugin.h"
#include <QLabel>
#include <iioutil/connectionprovider.h>

using namespace scopy::iiodebugplugin;

bool IIODebugPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = true;
	return ret;
}

bool IIODebugPlugin::loadPage()
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
			qWarning(CAT_IIODEBUGPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

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
	return true;
}

void IIODebugPlugin::initMetadata()
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

// ----------------------------

QString IIODebugPlugin_API::debugTest() const { return m_debugTest; }

void IIODebugPlugin_API::setDebugTest(const QString &newDebugTest) { m_debugTest = "ajndjsadjba"; }
