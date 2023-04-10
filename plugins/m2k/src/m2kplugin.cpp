#include "m2kplugin.h"
#include "digitalio.hpp"
#include "m2kcommon.h"
#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QLoggingCategory>
#include <QUuid>
#include <QSpacerItem>
#include <QTimer>
#include <libsigrokdecode/libsigrokdecode.h>
#include <pluginbase/preferences.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferenceshelper.h>
#include <pluginbase/scopyjs.h>
#include "iioutil/contextprovider.h"
#include <iio.h>

#include "filter.hpp"
#include "dmm.hpp"
#include "manualcalibration.h"
#include "digitalchannel_manager.hpp"
#include "network_analyzer.hpp"
#include "oscilloscope.hpp"
#include "patterngenerator/pattern_generator.h"
#include "power_controller.hpp"
#include "signal_generator.hpp"
#include "spectrum_analyzer.hpp"

using namespace adiscope;
using namespace adiscope::m2k;

Q_LOGGING_CATEGORY(CAT_M2KPLUGIN,"M2KPLUGIN");

bool M2kPlugin::compatible(QString m_param) {
	qDebug(CAT_M2KPLUGIN)<<"compatible";
	bool ret = false;
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);

	if(!ctx)
		return false;

//	ret = !!iio_context_find_device(ctx,"m2k-adc");
//	ret = ret && !!iio_context_find_device(ctx,"m2k-dac-a");
//	ret = ret && !!iio_context_find_device(ctx,"m2k-dac-b");

	Filter *f = new Filter(ctx);
	ret = (f->hw_name().compare("M2K") == 0);
	delete(f);

	c->close(m_param);
	return ret;
}

void M2kPlugin::preload()
{
	m_m2kController = new M2kController(m_param,this);
}


void M2kPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kosc","Oscilloscope",":/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kspec","Spectrum Analyzer",":/icons/scopy-default/icons/tool_spectrum_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2knet","Network Analyzer",":/icons/scopy-default/icons/tool_network_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2ksiggen","Signal Generator",":/icons/scopy-default/icons/tool_signal_generator.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2klogic","Logic Analyzer",":/icons/scopy-default/icons/tool_logic_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpattern","Pattern Generator",":/icons/scopy-default/icons/tool_pattern_generator.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kdio","Digital I/O",":/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kdmm","Voltmeter",":/icons/scopy-default/icons/tool_voltmeter.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpower","Power Supply",":/icons/scopy-default/icons/tool_power_supply.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kcal","Calibration",":/icons/scopy-default/icons/tool_calibration.svg"));

}

bool M2kPlugin::loadPage()
{
	m_infoPageTimer = new QTimer(this);
	m_infoPageTimer->setInterval(infoPageTimerTimeout);	
	connect(m_m2kController,SIGNAL(newTemperature(double)),this, SLOT(updateTemperature(double)));
	m_page = new QWidget();
	m_page->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_m2kInfoPage = new InfoPage(m_page);
	m_m2kInfoPage->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);
	for(int i=0;i<iio_context_get_attrs_count(ctx);i++) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(ctx,i,&name,&value);
		if(ret != 0)
			continue;

		m_m2kInfoPage->update(name,value);

	}
	c->close(m_param);


	return true;
}

bool M2kPlugin::loadExtraButtons()
{

	m_btnIdentify = new QPushButton("Identify"); m_extraButtons.append(m_btnIdentify);
	m_btnCalibrate = new QPushButton("Calibrate"); m_extraButtons.append(m_btnCalibrate);
	m_btnRegister = new QPushButton("Register"); m_extraButtons.append(m_btnRegister);

	m_btnCalibrate->setDisabled(true);

	connect(m_btnIdentify,SIGNAL(clicked()),m_m2kController,SLOT(identify()));
	connect(m_btnCalibrate,SIGNAL(clicked()),m_m2kController,SLOT(calibrate()));
	connect(m_m2kController,&M2kController::calibrationStarted, this, [=](){m_btnCalibrate->setEnabled(false);});
	connect(m_m2kController,&M2kController::calibrationFinished, this, [=](){m_btnCalibrate->setEnabled(true);});
	connect(m_btnRegister,&QPushButton::clicked,this,[=](){
		;
//		QString versionString = QString(m_info_params["Model"].split("Rev")[1][1]);
//		QString url = "https://my.analog.com/en/app/registration/hardware/ADALM2000?sn="+QString(getSerialNumber())+"&v=Rev."+versionString;
//		QDesktopServices::openUrl(QUrl(url));
	});


	return true;
}

bool M2kPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/icons/adalm.svg");
	return true;
}

void M2kPlugin::showPageCallback()
{
	m_m2kController->startTemperatureTask();
}

void M2kPlugin::hidePageCallback()
{
	m_m2kController->stopTemperatureTask();
}

void M2kPlugin::calibrationStarted()
{
	storeToolState(calibrationToolNames);
	for(const QString &tool : calibrationToolNames) {
		auto tme = ToolMenuEntry::findToolMenuEntryByName(m_toolList,tool);
		tme->setName("Calibrating ... ");
		tme->setEnabled(false);
		tme->setRunning(false);
		tme->setRunBtnVisible(false);
	}
}

void M2kPlugin::calibrationSuccess()
{
	restoreToolState(calibrationToolNames);
}

void M2kPlugin::calibrationFinished()
{
	restoreToolState(calibrationToolNames);
}

void M2kPlugin::updateTemperature(double val)
{
	m_m2kInfoPage->update("Temperature", QString::number(val));
}

void M2kPlugin::storeToolState(QStringList tools) {
	for(const QString &tool : calibrationToolNames) {
		auto tme = ToolMenuEntry::findToolMenuEntryByName(m_toolList,tool);
		toolMenuEntryCalibrationCache[tool] = new ToolMenuEntry(*tme); // save tool state
	}
}

void M2kPlugin::restoreToolState(QStringList tools) {
	for(const QString &tool : calibrationToolNames) {
		if(!toolMenuEntryCalibrationCache.contains(tool))
			continue;
		ToolMenuEntry* cachedTme = toolMenuEntryCalibrationCache[tool];
		auto id = toolMenuEntryCalibrationCache[tool]->id();
		auto tme = ToolMenuEntry::findToolMenuEntryById(m_toolList,id);
		tme->setName(cachedTme->name());
		tme->setEnabled(cachedTme->enabled());
		tme->setRunning(cachedTme->running());
		tme->setRunBtnVisible(cachedTme->runBtnVisible());
		delete cachedTme;
	}
}

void M2kPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("m2k_instrument_notes_active",false);

	p->init("m2k_show_adc_filters",false);
	p->init("m2k_show_graticule",false);
	p->init("m2k_mini_histogram",false);
	p->init("m2k_osc_filtering",true);
	p->init("m2k_osc_label",false);
	p->init("m2k_siggen_periods",2);
	p->init("m2k_spectrum_visible_peak_search",true);
	p->init("m2k_na_show_zero",false);

	p->init("m2k_logic_separate_annotations",false);
	p->init("m2k_logic_display_sampling_points",false);
	p->init("m2k_logic_display_sample_time",true);

}

bool M2kPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);
	lay->addWidget(new QLabel("-- General --"));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_instrument_notes_active","Instrument Notes",this));
	lay->addWidget(new QLabel("-- Analog tools --"));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_show_adc_filters","Show ADC digital filter config",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_show_graticule","Enable graticule",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_mini_histogram","Enable mini histogram",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_osc_filtering","Enable sample rate filters",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_osc_label","Enable labels on the plot",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_spectrum_visible_peak_search","Only search marker peaks in visible domain",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_na_show_zero","Always display 0db value on the graph",this));
	lay->addWidget(PreferencesHelper::addPreferenceEdit(p,"m2k_siggen_periods","Number of displayed periods",this));
	lay->addWidget(new QLabel("-- Logic tools --"));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_logic_separate_annotations","Separate decoder annotaions when exporting",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_logic_display_sampling_points","Display sampling points when zoomed",this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"m2k_logic_display_sample_time","Show sample and time info in decoder table",this));

	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Minimum,QSizePolicy::Expanding));

	return true;
}

void M2kPlugin::init()
{
#if defined __APPLE__
				bool success = loadDecoders(QCoreApplication::applicationDirPath() + "/decoders");
#else
				bool success = loadDecoders("decoders");
#endif
				qInfo(CAT_M2KPLUGIN)<<"loadDecoders " << success;
}

void M2kPlugin::saveSettings()
{
	for(auto &&tool : tools) {
		ApiObject *api = tool->getApi();
		if(api) {
			api->save();
		}
	}
}

void M2kPlugin::loadSettings()
{
	for(auto &&tool : tools) {
		ApiObject *api = tool->getApi();
		if(api) {
			api->load();
		}
	}
}

bool M2kPlugin::loadDecoders(QString path)
{
	static bool srd_loaded = false;

	if (srd_loaded) {
		srd_exit();
	}

	if (srd_init(path.toStdString().c_str()) != SRD_OK) {
		qDebug(CAT_M2KPLUGIN) << "ERROR: libsigrokdecode init failed.";
		return false;
	} else {
		srd_loaded = true;
		/* Load the protocol decoders */
		srd_decoder_load_all();
		auto decoder = srd_decoder_get_by_id("parallel");

		if (decoder == nullptr) {
			return false;
		}
	}

	return true;
}

bool M2kPlugin::onConnect()
{
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);
	m2k_man = new m2k_iio_manager();
	m_btnCalibrate->setDisabled(false);

	m_m2kController->connectM2k(ctx);
	m_m2kController->startPingTask();

	Filter *f = new Filter(ctx);
	QJSEngine *js = ScopyJS::GetInstance()->engine();

	auto calib = new Calibration(ctx);
	auto diom = new DIOManager(ctx,f);

	auto dmmTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kdmm");
	auto mancalTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kcal");
	auto dioTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kdio");
	auto pwrTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kpower");
	auto siggenTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2ksiggen");
	auto specTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kspec");
	auto oscTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kosc");
	auto netTme = ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2knet");
	auto laTme =  ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2klogic");
	auto pgTme =  ToolMenuEntry::findToolMenuEntryById(m_toolList,"m2kpattern");
	m_adcBtnGrp = new QButtonGroup(this);	

	tools.insert("m2kdmm",new DMM(ctx, f, dmmTme, m2k_man)); dmmTme->setTool(tools["m2kdmm"]);
	tools.insert("m2kcal",new ManualCalibration(ctx,f,mancalTme,nullptr,calib)); mancalTme->setTool(tools["m2kcal"]);
	tools.insert("m2kdio",new DigitalIO(ctx,f,dioTme,diom,js,nullptr));dioTme->setTool(tools["m2kdio"]);
	tools.insert("m2kpower",new PowerController(ctx,pwrTme,js,nullptr));pwrTme->setTool(tools["m2kpower"]);
	tools.insert("m2ksiggen",new SignalGenerator(ctx,f,siggenTme,js,nullptr));siggenTme->setTool(tools["m2ksiggen"]);
	tools.insert("m2kspec",new SpectrumAnalyzer(ctx,f,specTme,m2k_man,js,nullptr));specTme->setTool(tools["m2kspec"]);
	tools.insert("m2kosc",new Oscilloscope(ctx,f,oscTme,m2k_man,js,nullptr));oscTme->setTool(tools["m2kosc"]);
	tools.insert("m2knet",new NetworkAnalyzer(ctx,f,netTme,m2k_man,js,nullptr));netTme->setTool(tools["m2knet"]);
	tools.insert("m2klogic",new logic::LogicAnalyzer(ctx,f,laTme,js,nullptr));laTme->setTool(tools["m2klogic"]);
	tools.insert("m2kpattern",new logic::PatternGenerator(ctx,f,pgTme,js,diom,nullptr));pgTme->setTool(tools["m2kpattern"]);

	connect(dynamic_cast<SignalGenerator*>(siggenTme->tool())->getRunButton(),&QPushButton::toggled, this,[=](bool en){
		if(en) {
			if(dynamic_cast<NetworkAnalyzer*>(netTme->tool())->getRunButton()->isChecked())
				dynamic_cast<NetworkAnalyzer*>(netTme->tool())->getRunButton()->setChecked(false);
		}
	});

	connect(dynamic_cast<NetworkAnalyzer*>(netTme->tool())->getRunButton(),&QPushButton::toggled, this,[=](bool en){
		if(en) {
			if(dynamic_cast<SignalGenerator*>(siggenTme->tool())->getRunButton()->isChecked())
				dynamic_cast<SignalGenerator*>(siggenTme->tool())->getRunButton()->setChecked(false);
		}
	});

//	m_dacBtnGrp->addButton(dynamic_cast<NetworkAnalyzer*>(netTme->tool())->getRunButton());
//	m_dacBtnGrp->addButton(dynamic_cast<SignalGenerator*>(siggenTme->tool())->getRunButton());

	connect(m_m2kController,&M2kController::pingFailed,this,&M2kPlugin::disconnectDevice);	
	connect(m_m2kController, SIGNAL(calibrationStarted()), this, SLOT(calibrationStarted()));
	connect(m_m2kController, SIGNAL(calibrationSuccess()), this, SLOT(calibrationSuccess()));
	connect(m_m2kController, SIGNAL(calibrationFailed()) , this, SLOT(calibrationFinished()));

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(true);
		tme->setRunBtnVisible(true);
		tme->setRunning(false);
	}

	m_m2kController->initialCalibration();
	return true;
}

bool M2kPlugin::onDisconnect()
{	
	delete m2k_man;
	delete calib;
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		delete tme->tool();
		tme->setTool(nullptr);
	}

	disconnect(m_m2kController,&M2kController::pingFailed,this,&M2kPlugin::disconnectDevice);
	disconnect(m_m2kController, SIGNAL(calibrationStarted()), this, SLOT(calibrationStarted()));
	disconnect(m_m2kController, SIGNAL(calibrationSuccess()), this, SLOT(calibrationSuccess()));
	disconnect(m_m2kController, SIGNAL(calibrationFailed()) , this, SLOT(calibrationFinished()));


	m_m2kController->stopPingTask();
	m_m2kController->disconnectM2k();
	m_btnCalibrate->setDisabled(true);


	ContextProvider *c = ContextProvider::GetInstance();
	c->close(m_param);
	return true;
}



void M2kPlugin::initMetadata()
{
	loadMetadata(
R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio",
		"m2k"
	   ],
	   "exclude":["*"]
	}
)plugin");
}



#include "moc_m2kplugin.cpp"
