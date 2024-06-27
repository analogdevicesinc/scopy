#include "dacplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include <iioutil/connectionprovider.h>

#include "dacinstrument.h"

Q_LOGGING_CATEGORY(CAT_DACPLUGIN, "DACPlugin")
using namespace scopy::dacplugin;

bool DACPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DACPLUGIN) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if (conn == nullptr)
		return ret;

	for (int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);

	}
	return true;//rn ret;
}

bool DACPlugin::loadPage()
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
			qWarning(CAT_DAC) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool DACPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void DACPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("Dac", "Dac", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void DACPlugin::unload() { /*delete m_infoPage;*/ }

QString DACPlugin::description() { return "Write the plugin description here"; }

bool DACPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();
	dac = new DacInstrument(conn);
	m_toolList[0]->setTool(dac);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
    	return true;
}

bool DACPlugin::onDisconnect()
{
	qDebug(CAT_DACPLUGIN) << "disconnect";
	if(m_ctx)
		ConnectionProvider::GetInstance()->close(m_param);

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

void DACPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
		"iio",
		"dac"
	   ]
	}
)plugin");
}
