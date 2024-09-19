#include "dacplugin.h"
#include "dac_logging_categories.h"
#include "dacinstrument.h"
#include "dacutils.h"

#include <QLabel>

#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace scopy::dac;

bool DACPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DAC) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return ret;

	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn)) {
				continue;
			}
			if(iio_channel_is_scan_element(chn) || DacUtils::checkDdsChannel(chn)) {
				ret = true;
				goto finish;
			}
		}
	}
finish:

	ConnectionProvider::GetInstance()->close(m_param);
	return ret;
}

bool DACPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	m_page->setLayout(lay);
	return true;
}

bool DACPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/scopy-default/icons/tool_oscilloscope.svg");
	return true;
}

void DACPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("dac", "DAC", ":/gui/icons/scopy-default/icons/tool_signal_generator.svg"));
}

void DACPlugin::unload()
{
	if(m_page) {
		delete m_page;
	}
}

QString DACPlugin::description() { return "Tool for generic IIO DAC control."; }

QString DACPlugin::about()
{
	QString content = "DAC plugin";
	return content;
}

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
	connect(m_toolList.last(), &ToolMenuEntry::runToggled, dynamic_cast<DacInstrument *>(dac),
		&DacInstrument::runToggled);
	connect(dynamic_cast<DacInstrument *>(dac), &DacInstrument::running, m_toolList[0], &ToolMenuEntry::setRunning);
	return true;
}

bool DACPlugin::onDisconnect()
{
	qDebug(CAT_DAC) << "disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			w->deleteLater();
			tool->setTool(nullptr);
		}
	}
	if(m_ctx) {
		ConnectionProvider::GetInstance()->close(m_param);
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

QString DACPlugin::version() { return "0.1"; }

#include "moc_dacplugin.cpp"
