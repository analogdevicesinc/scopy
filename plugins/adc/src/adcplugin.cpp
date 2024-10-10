#include "adcplugin.h"

#include "src/adcinstrument.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>

#include <iioutil/connectionprovider.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>

#include "adctimeinstrumentcontroller.h"
#include "adcfftinstrumentcontroller.h"

Q_LOGGING_CATEGORY(CAT_ADCPLUGIN, "ADCPlugin");
using namespace scopy;
using namespace scopy::grutil;
using namespace scopy::adc;

bool ADCPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_ADCPLUGIN) << "compatible";

	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(conn == nullptr)
		return ret;

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

void ADCPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("adc_plot_xaxis_label_position", HandlePos::SOUTH);
	p->init("adc_plot_yaxis_label_position", HandlePos::WEST);
	p->init("adc_plot_yaxis_handle_position", HandlePos::WEST);
	p->init("adc_plot_xcursor_position", HandlePos::SOUTH);
	p->init("adc_plot_ycursor_position", HandlePos::WEST);
	p->init("adc_plot_show_buffer_previewer", true);
	p->init("adc_default_y_mode", 0);
	p->init("adc_add_remove_plot", false);
	p->init("adc_add_remove_instrument", false);
	p->init("adc_acquisition_timeout", 10000);
}

bool ADCPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionCollapseWidget *generalSection =
		new MenuSectionCollapseWidget("General", MenuCollapseSection::MHCW_NONE);
	generalSection->contentLayout()->setSpacing(10);
	lay->addWidget(generalSection);
	lay->setMargin(0);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	auto adc_plot_xaxis_label_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_xaxis_label_position", "Plot X-Axis scale position",
		{{"Top", HandlePos::NORTH}, {"Bottom", HandlePos::SOUTH}}, generalSection);
	auto adc_plot_yaxis_label_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_yaxis_label_position", "Plot Y-Axis scale position",
		{{"Left", HandlePos::WEST}, {"Right", HandlePos::EAST}}, generalSection);
	auto adc_plot_yaxis_handle_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_yaxis_handle_position", "Plot channel Y-handle position",
		{{"Left", HandlePos::WEST}, {"Right", HandlePos::EAST}}, generalSection);
	auto adc_plot_xcursor_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_xcursor_position", "Plot X-Cursor position",
		{{"Top", HandlePos::NORTH}, {"Bottom", HandlePos::SOUTH}}, generalSection);
	auto adc_plot_ycursor_position = PreferencesHelper::addPreferenceComboList(
		p, "adc_plot_ycursor_position", "Plot Y-Curosr position",
		{{"Left", HandlePos::WEST}, {"Right", HandlePos::EAST}}, generalSection);
	auto adc_plot_show_buffer_previewer = PreferencesHelper::addPreferenceCheckBox(
		p, "adc_plot_show_buffer_previewer", "Show buffer previewer", m_preferencesPage);

	auto adc_default_y_mode = PreferencesHelper::addPreferenceComboList(
		p, "adc_default_y_mode", "ADC Default Y-Mode", {{"ADC Count", 0}, {"% Full Scale", 1}}, generalSection);
	auto adc_acquisition_timeout = PreferencesHelper::addPreferenceEdit(
		p, "adc_acquisition_timeout", "ADC Acquisition timeout", m_preferencesPage);
	auto adc_add_remove_plot = PreferencesHelper::addPreferenceCheckBox(
		p, "adc_add_remove_plot", "Add/Remove plot feature (EXPERIMENTAL)", m_preferencesPage);
	auto adc_add_remove_instrument = PreferencesHelper::addPreferenceCheckBox(
		p, "adc_add_remove_instrument", "Add/Remove instrument feature (EXPERIMENTAL)", m_preferencesPage);

	generalSection->contentLayout()->addWidget(adc_plot_xaxis_label_position);
	generalSection->contentLayout()->addWidget(adc_plot_yaxis_label_position);
	generalSection->contentLayout()->addWidget(adc_plot_yaxis_handle_position);
	generalSection->contentLayout()->addWidget(adc_plot_xcursor_position);
	generalSection->contentLayout()->addWidget(adc_plot_ycursor_position);
	generalSection->contentLayout()->addWidget(adc_plot_show_buffer_previewer);
	generalSection->contentLayout()->addWidget(adc_default_y_mode);
	generalSection->contentLayout()->addWidget(adc_acquisition_timeout);
	generalSection->contentLayout()->addWidget(adc_add_remove_plot);
	generalSection->contentLayout()->addWidget(adc_add_remove_instrument);

	return true;
}

bool ADCPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/scopy-default/icons/tool_oscilloscope.svg");
	return true;
}

bool ADCPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	m_page->setLayout(lay);
	m_page->hide(); // This does not implement a legitimate page, it just adds an empty space on the page
	return true;
}

void ADCPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("time", "ADC - Time", ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("freq", "ADC - Frequency",
						  ":/gui/icons/scopy-default/icons/tool_spectrum_analyzer.svg"));
}

bool iio_is_buffer_capable(struct iio_device *dev)
{
	for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
		struct iio_channel *chn = iio_device_get_channel(dev, j);
		if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
			return true;
		}
	}
	return false;
}

void ADCPlugin::createGRIIOTreeNode(GRTopBlockNode *ctxNode, iio_context *ctx)
{
	int devCount = iio_context_get_devices_count(ctx);
	qDebug(CAT_ADCPLUGIN) << " Found " << devCount << "devices";
	ctxNode->setCtx(ctx);
	for(int i = 0; i < devCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		QString dev_name = QString::fromLocal8Bit(iio_device_get_name(dev));

		if(dev_name.isEmpty())
			continue;
		/*if(dev_name == "m2k-logic-analyzer-rx")
			continue;*/

		qDebug(CAT_ADCPLUGIN) << "Looking for scanelements in " << dev_name;
		QStringList channelList;

		GRIIODeviceSource *gr_dev = new GRIIODeviceSource(ctx, dev_name, dev_name, 0x400, ctxNode);
		GRIIODeviceSourceNode *d = new GRIIODeviceSourceNode(ctxNode, gr_dev, gr_dev);

		if(iio_is_buffer_capable(dev)) { // at least one scan element
			ctxNode->addTreeChild(d);
			ctxNode->src()->registerIIODeviceSource(gr_dev);
		} else {
			continue;
		}

		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			QString chn_name = QString::fromLocal8Bit(iio_channel_get_id(chn));
			qDebug(CAT_ADCPLUGIN) << "Verify if " << chn_name << "is scan element";
			if(chn_name == "timestamp" /*|| chn_name == "accel_z" || chn_name =="accel_y"*/)
				continue;
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {

				GRIIOFloatChannelSrc *ch = new GRIIOFloatChannelSrc(gr_dev, chn_name, d);
				GRIIOFloatChannelNode *c = new GRIIOFloatChannelNode(ctxNode, ch, d);
				d->addTreeChild(c);
			}
		}
	}
}

bool ADCPlugin::onConnect()
{
	Preferences *p = Preferences::GetInstance();
	connect(p, &Preferences::preferenceChanged, this, &ADCPlugin::preferenceChanged);
	for(auto &tool : m_toolList) {
		deleteInstrument(tool);
	}

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();

	// create gnuradio flow out of channels
	// pass channels to ADC instrument - figure out channel model (sample rate/ size/ etc)
	AcqTreeNode *root = new AcqTreeNode("root", this);
	GRTopBlock *top = new GRTopBlock("ctx", this);
	GRTopBlockNode *ctxNode = new GRTopBlockNode(top, nullptr);
	QMetaObject::invokeMethod(top, &GRTopBlock::suspendBuild, Qt::DirectConnection);
	root->addTreeChild(ctxNode);
	createGRIIOTreeNode(ctxNode, m_ctx);

	newInstrument(TIME, root, top);
	newInstrument(FREQUENCY, root, top);
	QMetaObject::invokeMethod(top, &GRTopBlock::unsuspendBuild, Qt::QueuedConnection);
	return true;
}

void ADCPlugin::newInstrument(ADCInstrumentType t, AcqTreeNode *root, GRTopBlock *grtp)
{

	static int idx = 0;
	ADCInstrumentController *adc;
	ADCInstrument *ui;

	if(t == TIME) {
		m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("time", "ADC - Time",
							  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
		auto tme = m_toolList.last();
		tme->setEnabled(true);
		tme->setRunBtnVisible(true);

		adc = new ADCTimeInstrumentController(tme, "adc" + QString::number(idx), root, this);
		adc->init();

		ui = adc->ui();
		idx++;

		connect(root, &AcqTreeNode::newChild, dynamic_cast<ADCTimeInstrumentController *>(adc),
			&ADCTimeInstrumentController::addChannel, Qt::QueuedConnection);
		connect(root, &AcqTreeNode::deletedChild, dynamic_cast<ADCTimeInstrumentController *>(adc),
			&ADCTimeInstrumentController::removeChannel, Qt::QueuedConnection);

		connect(ui, &ADCInstrument::requestNewInstrument, this, [=]() {
			QMetaObject::invokeMethod(grtp, &GRTopBlock::suspendBuild, Qt::DirectConnection);
			newInstrument(t, root, grtp);
			QMetaObject::invokeMethod(grtp, &GRTopBlock::unsuspendBuild, Qt::QueuedConnection);
		});

		connect(ui, &ADCInstrument::requestDeleteInstrument, this, [=]() {
			ToolMenuEntry *t = nullptr;
			for(auto tool : qAsConst(m_toolList)) {
				if(tool->tool() == ui) {
					t = tool;
				}
			}
			deleteInstrument(t);
		});

		connect(adc, &ADCInstrumentController::requestDisconnect, this, &ADCPlugin::disconnectDevice,
			Qt::QueuedConnection);
		m_ctrls.append(adc);
	} else if(t == FREQUENCY) {

		m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(
			"freq", "ADC - Frequency", ":/gui/icons/scopy-default/icons/tool_spectrum_analyzer.svg"));
		auto tme = m_toolList.last();
		tme->setEnabled(true);
		tme->setRunBtnVisible(true);

		adc = new ADCFFTInstrumentController(tme, "adc" + QString::number(idx), root, this);
		adc->init();
		ui = adc->ui();
		idx++;

		connect(root, &AcqTreeNode::newChild, dynamic_cast<ADCFFTInstrumentController *>(adc),
			&ADCFFTInstrumentController::addChannel, Qt::QueuedConnection);
		connect(root, &AcqTreeNode::deletedChild, dynamic_cast<ADCFFTInstrumentController *>(adc),
			&ADCFFTInstrumentController::removeChannel, Qt::QueuedConnection);

		connect(ui, &ADCInstrument::requestNewInstrument, this, [=]() {
			QMetaObject::invokeMethod(grtp, &GRTopBlock::suspendBuild, Qt::DirectConnection);
			newInstrument(t, root, grtp);
			QMetaObject::invokeMethod(grtp, &GRTopBlock::unsuspendBuild, Qt::QueuedConnection);
		});

		connect(ui, &ADCInstrument::requestDeleteInstrument, this, [=]() {
			ToolMenuEntry *t = nullptr;
			for(auto tool : qAsConst(m_toolList)) {
				if(tool->tool() == ui) {
					t = tool;
				}
			}
			deleteInstrument(t);
		});

		connect(adc, &ADCInstrumentController::requestDisconnect, this, &ADCPlugin::disconnectDevice,
			Qt::QueuedConnection);
		m_ctrls.append(adc);
	} else {
		return;
	}

	auto tme = m_toolList.last();
	Q_EMIT toolListChanged();
	tme->setTool(ui);

	adc->setEnableAddRemovePlot(Preferences::get("adc_add_remove_plot").toBool());
	adc->setEnableAddRemoveInstrument(Preferences::get("adc_add_remove_instrument").toBool());

	// prevent user from deleting first time and fft tool
	if(idx <= 2)
		adc->setIsMainInstrument(true);
}

void ADCPlugin::deleteInstrument(ToolMenuEntry *tool)
{
	tool->setEnabled(false);
	tool->setRunning(false);
	tool->setRunBtnVisible(false);
	QWidget *w = tool->tool();
	if(w) {
		ADCInstrumentController *found = nullptr;
		for(ADCInstrumentController *ctrl : m_ctrls) {
			if(ctrl->ui() == tool->tool()) {
				found = ctrl;
				break;
			}
		}
		found->stop();
		m_ctrls.removeAll(found);
		tool->setTool(nullptr);
		delete(w);
	}
	m_toolList.removeAll(tool);
	Q_EMIT toolListChanged();
}

void ADCPlugin::preferenceChanged(QString s, QVariant t1)
{
	if(s == "adc_add_remove_plot") {
		for(ADCInstrumentController *ctrl : m_ctrls) {
			ctrl->setEnableAddRemovePlot(t1.toBool());
		}
	}
	if(s == "adc_add_remove_instrument") {
		for(ADCInstrumentController *ctrl : m_ctrls) {
			ctrl->setEnableAddRemoveInstrument(t1.toBool());
		}
	}
	if(s == "adc_acquisition_timeout") {
		bool ok;
		Preferences::get("adc_acquisition_timeout").toDouble(&ok);
		if(!ok) {
			Preferences::set("adc_acquisition_timeout", 1000);
		}
	}
}

bool ADCPlugin::onDisconnect()
{
	Preferences *p = Preferences::GetInstance();
	disconnect(p, &Preferences::preferenceChanged, this, &ADCPlugin::preferenceChanged);
	qDebug(CAT_ADCPLUGIN) << "disconnect";
	if(m_ctx)
		ConnectionProvider::GetInstance()->close(m_param);
	for(auto &tool : m_toolList) {
		deleteInstrument(tool);
	}

	loadToolList();
	Q_EMIT toolListChanged();

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
	   "priority":10,
	   "category":[
		  "iio",
		  "adc"
	   ]
	}
)plugin");
}

QString ADCPlugin::version() { return "0.1"; }

#include "moc_adcplugin.cpp"
