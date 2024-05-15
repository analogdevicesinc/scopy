#include "iiodebugplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include "IIODebugInstrument.h"

Q_LOGGING_CATEGORY(CAT_IIODEBUGPLUGIN, "IIODebugPlugin");
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

QString IIODebugPlugin::description() { return "Write the plugin description here"; }

bool IIODebugPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	IIODebugInstrument *iio_debugger = new IIODebugInstrument();
	m_toolList[0]->setTool(iio_debugger);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	return true;
}

bool IIODebugPlugin::onDisconnect()
{
	// This method is called when the discconect button is pressed
	// It must remove all connections that were established om the connection
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
