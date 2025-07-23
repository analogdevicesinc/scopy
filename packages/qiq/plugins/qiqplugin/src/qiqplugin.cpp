#include "qiqplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include "qiqinstrument.h"

#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_QIQPLUGIN, "QIQPlugin")
using namespace scopy::qiqplugin;

bool QIQPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_QIQPLUGIN) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr) {
		return ret;
	}

	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				ret = true;
				goto finish;
			}
		}
	}

finish:
	ConnectionProvider::GetInstance()->close(m_param);
	return ret;
}

bool QIQPlugin::loadPage()
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
			qWarning(CAT_QIQPLUGIN) << "Could not read attribute with index:" << i;
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

bool QIQPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void QIQPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("qiqinstrument", "QIQInstrument",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void QIQPlugin::unload()
{ /*delete m_infoPage;*/
}

QString QIQPlugin::description() { return "A new plugin"; }

bool QIQPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	m_iioManager = new IIOManager(conn->context());

	QIQInstrument *qiqInstrument = new QIQInstrument();
	m_toolList[0]->setTool(qiqInstrument);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	// tests
	QVariantMap configMap;
	configMap["sample_count"] = 128;
	configMap["channel_count"] = 0;
	configMap["sampling_frequency"] = 1024;
	configMap["frequency_offset"] = 0;
	configMap["input_file"] = "";
	configMap["input_file_format"] = "binary-interleaved";
	configMap["chnls_format"] = QStringList();

	InputConfig config;
	config.fromVariantMap(configMap);

	QVariantMap outputInfoMap = {
		{"channel_count", 3},
		{"channel_names", QStringList{"magnitude", "magnitude", "magnitude"}},
		{"channel_format", QStringList{"int16", "int16", "int16"}},
	};

	QList<QIQPlotInfo> plotInfoList;

	// Plot 0: Add
	QVariantMap plot0;
	plot0["id"] = 0;
	plot0["title"] = "Add";
	plot0["xLabel"] = "time[s]";
	plot0["yLabel"] = "magnitude[dB]";
	plot0["type"] = "time";
	plot0["xyValues"] = false;
	plot0["ch"] = QVariantList{QVariantMap{{"y", 0}}};
	plotInfoList.append(QIQPlotInfo::fromVariantMap(plot0));

	// Plot 1: Sub
	QVariantMap plot1;
	plot1["id"] = 1;
	plot1["title"] = "Sub";
	plot1["xLabel"] = "time[s]";
	plot1["yLabel"] = "magnitude[dB]";
	plot1["type"] = "time";
	plot1["xyValues"] = false;
	plot1["ch"] = QVariantList{QVariantMap{{"y", 1}}};
	plotInfoList.append(QIQPlotInfo::fromVariantMap(plot1));

	// Plot 2: Mul
	QVariantMap plot2;
	plot2["id"] = 2;
	plot2["title"] = "Mul";
	plot2["xLabel"] = "time[s]";
	plot2["yLabel"] = "magnitude[dB]";
	plot2["type"] = "time";
	plot2["xyValues"] = false;
	plot2["ch"] = QVariantList{QVariantMap{{"y", 2}}};
	plotInfoList.append(QIQPlotInfo::fromVariantMap(plot2));

	OutputInfo outInfo;
	outInfo.fromVariantMap(outputInfoMap);
	QStringList analysis{"psk", "fft", "addition"};

	qiqInstrument->setAvailableChannels(m_iioManager->getAvailableChannels());
	qiqInstrument->onAnalysisTypes(analysis);
	qiqInstrument->onInputFormatChanged(config);
	qiqInstrument->onAnalysisInfo("psk", pskMap, outInfo, plotInfoList);

	connect(m_iioManager, &IIOManager::inputFormatChanged, qiqInstrument, &QIQInstrument::onInputFormatChanged);
	connect(m_iioManager, &IIOManager::dataReady, qiqInstrument, &QIQInstrument::bufferDataReady);
	connect(qiqInstrument, &QIQInstrument::bufferParamsChanged, m_iioManager, &IIOManager::onBufferParamsChanged);
	connect(qiqInstrument, &QIQInstrument::runPressed, m_iioManager, &IIOManager::startAcq);

	connect(qiqInstrument, &QIQInstrument::requestNewData, m_iioManager, &IIOManager::onDataRequest);

	return true;
}

bool QIQPlugin::onDisconnect()
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

	m_iioManager->deleteLater();
	m_iioManager = nullptr;

	ConnectionProvider::GetInstance()->close(m_param);

	return true;
}

void QIQPlugin::initMetadata()
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
