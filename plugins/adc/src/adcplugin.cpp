#include "adcplugin.h"

#include "adcinstrument.h"
#include "gui/stylehelper.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>

#include <iioutil/contextprovider.h>
#include <pluginbase/preferences.h>
#include <pluginbase/preferenceshelper.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_ADCPLUGIN, "ADCPlugin");
using namespace scopy;
using namespace scopy::grutil;

bool ADCPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_ADCPLUGIN) << "compatible";

	bool ret = false;
	iio_context *ctx = ContextProvider::GetInstance()->open(m_param);

	if(ctx == nullptr)
		return ret;

	for(int i = 0; i < iio_context_get_devices_count(ctx); i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(iio_channel_is_scan_element(chn)) {
				ret = true;
				goto finish;
			}
		}
	}

finish:

	ContextProvider::GetInstance()->close(m_param);
	return ret;
}

void ADCPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("adc_plot_xaxis_label_position", QwtAxis::XBottom);
	p->init("adc_plot_yaxis_label_position", QwtAxis::YLeft);
	p->init("adc_plot_yaxis_handle_position", QwtAxis::YLeft);
	p->init("adc_plot_xcursor_position", QwtAxis::XBottom);
	p->init("adc_plot_ycursor_position", QwtAxis::YLeft);
	p->init("adc_plot_show_buffer_previewer", true);
}

bool ADCPlugin::loadPreferencesPage()
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
	lay->addWidget(generalWidget);
	lay->setMargin(0);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	auto adc_plot_xaxis_label_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_xaxis_label_position", "Plot X-Axis scale position",
		{{"Top", QwtAxis::XTop}, {"Bottom", QwtAxis::XBottom}}, generalSection);
	auto adc_plot_yaxis_label_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_yaxis_label_position", "Plot Y-Axis scale position",
		{{"Left", QwtAxis::YLeft}, {"Right", QwtAxis::YRight}}, generalSection);
	auto adc_plot_yaxis_handle_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_yaxis_handle_position", "Plot channel Y-handle position",
		{{"Left", QwtAxis::YLeft}, {"Right", QwtAxis::YRight}}, generalSection);
	auto adc_plot_xcursor_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_xcursor_position", "Plot X-Cursor position",
		{{"Top", QwtAxis::XTop}, {"Bottom", QwtAxis::XBottom}}, generalSection);
	auto adc_plot_ycursor_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_ycursor_position", "Plot Y-Curosr position",
		{{"Left", QwtAxis::YLeft}, {"Right", QwtAxis::YRight}}, generalSection);
	auto adc_plot_show_buffer_previewer = PreferencesHelper::addPreferenceCheckBox(
		p, "adc_plot_show_buffer_previewer", "Show buffer previewer", m_preferencesPage);

	generalSection->contentLayout()->addWidget(adc_plot_xaxis_label_position);
	generalSection->contentLayout()->addWidget(adc_plot_yaxis_label_position);
	generalSection->contentLayout()->addWidget(adc_plot_yaxis_handle_position);
	generalSection->contentLayout()->addWidget(adc_plot_xcursor_position);
	generalSection->contentLayout()->addWidget(adc_plot_ycursor_position);
	generalSection->contentLayout()->addWidget(adc_plot_show_buffer_previewer);

	//	connect(p, &Preferences::preferenceChanged, )

	return true;
}

bool ADCPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

bool ADCPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	return true;
}

void ADCPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("time", "Time", ":/gui/icons/scopy-default/icons/tool_home.svg"));
}

PlotProxy *ADCPlugin::createRecipe(iio_context *ctx)
{
	QStringList deviceList;
	QMap<QString, QStringList> devChannelMap;
	int devCount = iio_context_get_devices_count(ctx);
	qDebug(CAT_ADCPLUGIN) << " Found " << devCount << "devices";
	for(int i = 0; i < devCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		QString dev_name = QString::fromLocal8Bit(iio_device_get_name(dev));

		qDebug(CAT_ADCPLUGIN) << "Looking for scanelements in " << dev_name;
		if(dev_name == "m2k-logic-analyzer-rx")
			continue;
		QStringList channelList;
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {

			struct iio_channel *chn = iio_device_get_channel(dev, j);
			QString chn_name = QString::fromLocal8Bit(iio_channel_get_id(chn));
			qDebug(CAT_ADCPLUGIN) << "Verify if " << chn_name << "is scan element";
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

	int i = 0;
	for(const QString &iio_dev : deviceList) {
		GRIIODeviceSource *gr_dev = new GRIIODeviceSource(m_ctx, iio_dev, iio_dev, 0x400, this);
		top->registerIIODeviceSource(gr_dev);

		GRDeviceAddon *d = new GRDeviceAddon(gr_dev, this);
		connect(s, &GRTimePlotAddonSettings::bufferSizeChanged, d, &GRDeviceAddon::updateBufferSize);
		recipe->addDeviceAddon(d);

		for(const QString &ch : devChannelMap.value(iio_dev, {})) {
			GRTimeChannelAddon *t = new GRTimeChannelAddon(
				ch, d, p, QPen(StyleHelper::getColor("CH" + QString::number(i))), this);
			top->registerSignalPath(t->signalPath());
			recipe->addChannelAddon(t);
			i++;
		}
	}
	recipe->setTopBlock(top);

	qDebug(CAT_ADCPLUGIN) << deviceList;
	qDebug(CAT_ADCPLUGIN) << devChannelMap;

	return recipe;
}

bool ADCPlugin::onConnect()
{
	m_ctx = ContextProvider::GetInstance()->open(m_param);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	// create gnuradio flow out of channels
	// pass channels to ADC instrument - figure out channel model (sample rate/ size/ etc)

	auto recipe = createRecipe(m_ctx);

	time = new AdcInstrument(recipe);
	m_toolList[0]->setTool(time);

	return true;
}

bool ADCPlugin::onDisconnect()
{
	qDebug(CAT_ADCPLUGIN) << "disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}
	return true;
}

void ADCPlugin::saveSettings(QSettings &s) {}

void ADCPlugin::loadSettings(QSettings &s) {}

QString ADCPlugin::about()
{
	QString content = "ADC plugin";
	return content;
}

void ADCPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":2,
	   "category":[
		  "iio",
		  "adc"
	   ]
	}
)plugin");
}

QString ADCPlugin::version() { return "0.1"; }
