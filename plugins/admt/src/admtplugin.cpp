#include "admtplugin.h"
#include "harmoniccalibration.h"

#include <QLoggingCategory>
#include <QLabel>

#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_ADMTPLUGIN, "ADMTPlugin")
using namespace scopy;
using namespace scopy::grutil;

bool ADMTPlugin::compatible(QString m_param, QString category)
{
	m_name = "ADMT4000";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_ADMTPLUGIN) << "No context available for admt";
		return false;
	}

	iio_device *admtDevice = iio_context_find_device(conn->context(), "admt4000");
	if(admtDevice) {
		ret = true;
	}

	ConnectionProvider::close(m_param);
	
	return ret;
}

bool ADMTPlugin::loadPage()
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
			qWarning(CAT_ADMTPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool ADMTPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void ADMTPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("harmoniccalibration", "Harmonic Calibration", ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
}

void ADMTPlugin::unload() { /*delete m_infoPage;*/ }

QString ADMTPlugin::description() { return "Plugin for ADMT Harmonic Calibration"; }

PlotProxy *ADMTPlugin::createRecipe(iio_context *ctx)
{
	QStringList deviceList;
	QMap<QString, QStringList> devChannelMap;
	int devCount = iio_context_get_devices_count(ctx);
	qDebug(CAT_ADMTPLUGIN) << " Found " << devCount << "devices";
	for(int i = 0; i < devCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		QString dev_name = QString::fromLocal8Bit(iio_device_get_name(dev));

		qDebug(CAT_ADMTPLUGIN) << "Looking for scanelements in " << dev_name;
		if(dev_name == "m2k-logic-analyzer-rx")
			continue;
		QStringList channelList;
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {

			struct iio_channel *chn = iio_device_get_channel(dev, j);
			QString chn_name = QString::fromLocal8Bit(iio_channel_get_id(chn));
			qDebug(CAT_ADMTPLUGIN) << "Verify if " << chn_name << "is scan element";
			if(chn_name == "timestamp" /*|| chn_name == "accel_z" || chn_name =="accel_y"*/)
				continue;
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				channelList.append(chn_name);
			}
		}
		if(channelList.isEmpty())
			continue;
		deviceList.append(dev_name);
		devChannelMap.insert(dev_name, channelList);
	}

	// should this be wrapped to a register function (?)
	GRTopBlock *top = new grutil::GRTopBlock("Time", this);

	recipe = new GRTimePlotProxy(this);
	QString plotRecipePrefix = "time_";
	recipe->setPrefix(plotRecipePrefix);

	GRTimePlotAddon *p = new GRTimePlotAddon(plotRecipePrefix, top, this);
	GRTimePlotAddonSettings *s = new GRTimePlotAddonSettings(p, this);

	recipe->setPlotAddon(p, s);

	ChannelIdProvider *chIdProvider = recipe->getChannelIdProvider();
	for(const QString &iio_dev : deviceList) {
		GRIIODeviceSource *gr_dev = new GRIIODeviceSource(m_ctx, iio_dev, iio_dev, 0x400, this);

		top->registerIIODeviceSource(gr_dev);

		GRDeviceAddon *d = new GRDeviceAddon(gr_dev, this);
		connect(s, &GRTimePlotAddonSettings::bufferSizeChanged, d, &GRDeviceAddon::updateBufferSize);
		recipe->addDeviceAddon(d);

		for(const QString &ch : devChannelMap.value(iio_dev, {})) {
			int idx = chIdProvider->next();
			GRTimeChannelAddon *t = new GRTimeChannelAddon(ch, d, p, chIdProvider->pen(idx), this);
			top->registerSignalPath(t->signalPath());
			recipe->addChannelAddon(t);
		}
	}
	recipe->setTopBlock(top);

	qDebug(CAT_ADMTPLUGIN) << deviceList;
	qDebug(CAT_ADMTPLUGIN) << devChannelMap;

	return recipe;
}

bool ADMTPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is 
	// compatible to that device 
	// In case of success the function must return true and false otherwise 
    	
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	auto recipe = createRecipe(m_ctx);

	harmonicCalibration = new HarmonicCalibration(recipe);
	m_toolList[0]->setTool(harmonicCalibration);
	
	return true;
}

bool ADMTPlugin::onDisconnect()
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

void ADMTPlugin::initMetadata()
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
