#include "m2kplugin.h"

#include "digitalchannel_manager.hpp"
#include "digitalio.hpp"
#include "dmm.hpp"
#include "filter.hpp"
#include "iioutil/contextprovider.h"
#include "m2kcommon.h"
#include "manualcalibration.h"
#include "network_analyzer.hpp"
#include "oscilloscope.hpp"
#include "patterngenerator/pattern_generator.h"
#include "power_controller.hpp"
#include "qtextbrowser.h"
#include "signal_generator.hpp"
#include "spectrum_analyzer.hpp"

#include <iio.h>

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>
#include <QTimer>

#include <libsigrokdecode/libsigrokdecode.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>
#include <pluginbase/preferenceshelper.h>
#include <pluginbase/scopyjs.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>

using namespace scopy;
using namespace scopy::m2k;

Q_LOGGING_CATEGORY(CAT_M2KPLUGIN, "M2KPLUGIN");
Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark")

bool M2kPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_M2KPLUGIN) << "compatible";
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
	m_m2kController = new M2kController(m_param, this);
	m_displayName = "M2k";
}

void M2kPlugin::loadToolList()
{
	Preferences *p = Preferences::GetInstance();
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kosc", tr("Oscilloscope"),
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kspec", tr("Spectrum Analyzer"),
						  ":/gui/icons/scopy-default/icons/tool_spectrum_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2knet", tr("Network Analyzer"),
						  ":/gui/icons/scopy-default/icons/tool_network_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2ksiggen", tr("Signal Generator"),
						  ":/gui/icons/scopy-default/icons/tool_signal_generator.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2klogic", tr("Logic Analyzer"),
						  ":/gui/icons/scopy-default/icons/tool_logic_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpattern", tr("Pattern Generator"),
						  ":/gui/icons/scopy-default/icons/tool_pattern_generator.svg"));
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("m2kdio", tr("Digital I/O"), ":/gui/icons/scopy-default/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kdmm", tr("Voltmeter"),
						  ":/gui/icons/scopy-default/icons/tool_voltmeter.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpower", tr("Power Supply"),
						  ":/gui/icons/scopy-default/icons/tool_power_supply.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kcal", tr("Calibration"),
						  ":/gui/icons/scopy-default/icons/tool_calibration.svg"));
	ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kcal")
		->setVisible(p->get("m2k_manual_calibration_enable").toBool());
}

bool M2kPlugin::loadPage()
{
	m_infoPageTimer = new QTimer(this);
	m_infoPageTimer->setInterval(infoPageTimerTimeout);
	connect(m_m2kController, SIGNAL(newTemperature(double)), this, SLOT(updateTemperature(double)));
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_m2kInfoPage = new InfoPage(m_page);
	lay->addWidget(m_m2kInfoPage);

	QTextBrowser *textBrowser = new QTextBrowser(m_page);
	QFile f(":/m2k/m2k.html");
	f.open(QFile::ReadOnly);
	textBrowser->setText(f.readAll());
	textBrowser->setFixedHeight(800);
	lay->addWidget(textBrowser);

	m_m2kInfoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);
	for(int i = 0; i < iio_context_get_attrs_count(ctx); i++) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(ctx, i, &name, &value);
		if(ret != 0)
			continue;

		m_m2kInfoPage->update(name, value);
	}
	c->close(m_param);

	return true;
}

bool M2kPlugin::loadExtraButtons()
{

	m_btnIdentify = new QPushButton("Identify");
	m_extraButtons.append(m_btnIdentify);
	m_btnCalibrate = new QPushButton("Calibrate");
	m_extraButtons.append(m_btnCalibrate);
	m_btnRegister = new QPushButton("Register");
	m_extraButtons.append(m_btnRegister);

	m_btnCalibrate->setDisabled(true);

	connect(m_btnIdentify, SIGNAL(clicked()), m_m2kController, SLOT(identify()));
	connect(m_btnCalibrate, SIGNAL(clicked()), m_m2kController, SLOT(calibrate()));
	connect(m_m2kController, &M2kController::calibrationStarted, this,
		[=]() { m_btnCalibrate->setEnabled(false); });
	connect(m_m2kController, &M2kController::calibrationFinished, this,
		[=]() { m_btnCalibrate->setEnabled(true); });
	connect(m_btnRegister, &QPushButton::clicked, this, [=]() {
		;
		//		QString versionString = QString(m_info_params["Model"].split("Rev")[1][1]);
		//		QString url =
		//"https://my.analog.com/en/app/registration/hardware/ADALM2000?sn="+QString(getSerialNumber())+"&v=Rev."+versionString;
		//		QDesktopServices::openUrl(QUrl(url));
	});

	return true;
}

bool M2kPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void M2kPlugin::showPageCallback() { m_m2kController->startTemperatureTask(); }

void M2kPlugin::hidePageCallback() { m_m2kController->stopTemperatureTask(); }

void M2kPlugin::calibrationStarted()
{
	storeToolState(calibrationToolNames);
	for(const QString &tool : calibrationToolNames) {
		auto tme = ToolMenuEntry::findToolMenuEntryByName(m_toolList, tool);
		tme->setName("Calibrating ... ");
		tme->setEnabled(false);
		tme->setRunning(false);
		tme->setRunBtnVisible(false);
	}
}

void M2kPlugin::calibrationSuccess() { restoreToolState(calibrationToolNames); }

void M2kPlugin::calibrationFinished() { restoreToolState(calibrationToolNames); }

void M2kPlugin::updateTemperature(double val) { m_m2kInfoPage->update("Temperature", QString::number(val)); }

void M2kPlugin::storeToolState(QStringList tools)
{
	for(const QString &tool : calibrationToolNames) {
		auto tme = ToolMenuEntry::findToolMenuEntryByName(m_toolList, tool);
		toolMenuEntryCalibrationCache[tool] = new ToolMenuEntry(*tme); // save tool state
	}
}

void M2kPlugin::restoreToolState(QStringList tools)
{
	for(const QString &tool : calibrationToolNames) {
		if(!toolMenuEntryCalibrationCache.contains(tool))
			continue;
		ToolMenuEntry *cachedTme = toolMenuEntryCalibrationCache[tool];
		auto id = toolMenuEntryCalibrationCache[tool]->id();
		auto tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, id);
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
	p->init("m2k_instrument_notes_active", false);
	p->init("m2k_manual_calibration_enable", false);

	p->init("m2k_show_adc_filters", false);
	p->init("m2k_show_graticule", false);
	p->init("m2k_mini_histogram", false);
	p->init("m2k_osc_filtering", true);
	p->init("m2k_osc_label", false);
	p->init("m2k_siggen_periods", 2);
	p->init("m2k_spectrum_visible_peak_search", true);
	p->init("m2k_na_show_zero", false);

	p->init("m2k_logic_separate_annotations", false);
	p->init("m2k_logic_display_sampling_points", false);
	p->init("m2k_logic_display_sample_time", true);
}

bool M2kPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);
	lay->setSpacing(10);
	lay->setMargin(0);

	// General preferences
	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection =
		new MenuCollapseSection("General", MenuCollapseSection::MHCW_NONE, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_instrument_notes_active", "Instrument Notes", generalSection));

	// Analog tools preferences
	MenuSectionWidget *analogWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *analogSection =
		new MenuCollapseSection("Analog tools", MenuCollapseSection::MHCW_NONE, analogWidget);
	analogWidget->contentLayout()->setSpacing(10);
	analogWidget->contentLayout()->addWidget(analogSection);
	analogSection->contentLayout()->setSpacing(10);
	lay->addWidget(analogWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_show_adc_filters", "Show ADC digital filter config", analogSection));
	analogSection->contentLayout()->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "m2k_show_graticule", "Enable graticule", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_mini_histogram", "Enable mini histogram", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_osc_filtering", "Enable sample rate filters", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_osc_label", "Enable labels on the plot", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_spectrum_visible_peak_search", "Only search marker peaks in visible domain", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_na_show_zero", "Always display 0db value on the graph", analogSection));
	analogSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceEdit(
		p, "m2k_siggen_periods", "Number of displayed periods", analogSection));

	// Logic tools preferences
	MenuSectionWidget *logicWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *logicSection =
		new MenuCollapseSection("Logic tools", MenuCollapseSection::MHCW_NONE, logicWidget);
	logicWidget->contentLayout()->setSpacing(10);
	logicWidget->contentLayout()->addWidget(logicSection);
	logicSection->contentLayout()->setSpacing(10);
	lay->addWidget(logicWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	logicSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_logic_separate_annotations", "Separate decoder annotaions when exporting", logicSection));
	logicSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_logic_display_sampling_points", "Display sampling points when zoomed", logicSection));
	logicSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "m2k_logic_display_sample_time", "Show sample and time info in decoder table", logicSection));

	return true;
}

void M2kPlugin::init() {}

void M2kPlugin::saveSettings(QSettings &s)
{
	for(auto &&tool : tools) {
		ApiObject *api = tool->getApi();
		if(api) {
			api->save(s);
		}
	}
}

void M2kPlugin::loadSettings(QSettings &s)
{
	for(auto &&tool : tools) {
		ApiObject *api = tool->getApi();
		if(api) {
			api->load(s);
		}
	}
}

void M2kPlugin::cleanup()
{
	if(m2k_man) {
		delete m2k_man;
		m2k_man = nullptr;
	}
	if(calib) {
		delete calib;
		calib = nullptr;
	}
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		delete tme->tool();
		tme->setTool(nullptr);
	}

	disconnect(m_m2kController, &M2kController::pingFailed, this, &M2kPlugin::disconnectDevice);
	disconnect(m_m2kController, SIGNAL(calibrationStarted()), this, SLOT(calibrationStarted()));
	disconnect(m_m2kController, SIGNAL(calibrationSuccess()), this, SLOT(calibrationSuccess()));
	disconnect(m_m2kController, SIGNAL(calibrationFailed()), this, SLOT(calibrationFinished()));

	m_m2kController->stopPingTask();
	m_m2kController->disconnectM2k();
	m_btnCalibrate->setDisabled(true);

	ContextProvider *c = ContextProvider::GetInstance();
	c->close(m_param);
}

bool M2kPlugin::onConnect()
{
	ContextProvider *c = ContextProvider::GetInstance();
	iio_context *ctx = c->open(m_param);

	if(!ctx) {
		return false;
	}
	try {
		m2k_man = new m2k_iio_manager();
		m_btnCalibrate->setDisabled(false);

		m_m2kController->connectM2k(ctx);
		m_m2kController->startPingTask();
		connect(m_m2kController, &M2kController::pingFailed, this, &M2kPlugin::disconnectDevice);

		Filter *f = new Filter(ctx);
		QJSEngine *js = ScopyJS::GetInstance()->engine();

		auto calib = new Calibration(ctx);
		auto diom = new DIOManager(ctx, f);
		auto dmmTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kdmm");
		auto mancalTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kcal");
		auto dioTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kdio");
		auto pwrTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kpower");
		auto siggenTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2ksiggen");
		auto specTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kspec");
		auto oscTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kosc");
		auto netTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2knet");
		auto laTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2klogic");
		auto pgTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kpattern");
		m_adcBtnGrp = new QButtonGroup(this);

		tools.insert("m2kdmm", new DMM(ctx, f, dmmTme, m2k_man));
		dmmTme->setTool(tools["m2kdmm"]);
		tools.insert("m2kcal", new ManualCalibration(ctx, f, mancalTme, nullptr, calib));
		mancalTme->setTool(tools["m2kcal"]);
		tools.insert("m2kdio", new DigitalIO(ctx, f, dioTme, diom, js, nullptr));
		dioTme->setTool(tools["m2kdio"]);
		tools.insert("m2kpower", new PowerController(ctx, pwrTme, js, nullptr));
		pwrTme->setTool(tools["m2kpower"]);
		tools.insert("m2ksiggen", new SignalGenerator(ctx, f, siggenTme, js, nullptr));
		siggenTme->setTool(tools["m2ksiggen"]);
		tools.insert("m2kspec", new SpectrumAnalyzer(ctx, f, specTme, m2k_man, js, nullptr));
		specTme->setTool(tools["m2kspec"]);
		tools.insert("m2kosc", new Oscilloscope(ctx, f, oscTme, m2k_man, js, nullptr));
		oscTme->setTool(tools["m2kosc"]);
		tools.insert("m2knet", new NetworkAnalyzer(ctx, f, netTme, m2k_man, js, nullptr));
		netTme->setTool(tools["m2knet"]);
		tools.insert("m2klogic", new logic::LogicAnalyzer(ctx, f, laTme, js, nullptr));
		laTme->setTool(tools["m2klogic"]);

		logic::LogicAnalyzer *logic_analyzer = dynamic_cast<logic::LogicAnalyzer *>(tools["m2klogic"]);
		Oscilloscope *oscilloscope = dynamic_cast<Oscilloscope *>(tools["m2kosc"]);
		NetworkAnalyzer *network_analyzer = dynamic_cast<NetworkAnalyzer *>(tools["m2knet"]);

		oscilloscope->setLogicAnalyzer(logic_analyzer);
		network_analyzer->setOscilloscope(oscilloscope);

		tools.insert("m2kpattern", new logic::PatternGenerator(ctx, f, pgTme, js, diom, nullptr));
		pgTme->setTool(tools["m2kpattern"]);
		connect(dynamic_cast<SignalGenerator *>(siggenTme->tool())->getRunButton(), &QPushButton::toggled, this,
			[=](bool en) {
				if(en) {
					if(dynamic_cast<NetworkAnalyzer *>(netTme->tool())->getRunButton()->isChecked())
						dynamic_cast<NetworkAnalyzer *>(netTme->tool())
							->getRunButton()
							->setChecked(false);
				}
			});

		connect(dynamic_cast<NetworkAnalyzer *>(netTme->tool())->getRunButton(), &QPushButton::toggled, this,
			[=](bool en) {
				if(en) {
					if(dynamic_cast<SignalGenerator *>(siggenTme->tool())
						   ->getRunButton()
						   ->isChecked())
						dynamic_cast<SignalGenerator *>(siggenTme->tool())
							->getRunButton()
							->setChecked(false);
				}
			});

		//	m_dacBtnGrp->addButton(dynamic_cast<NetworkAnalyzer*>(netTme->tool())->getRunButton());
		//	m_dacBtnGrp->addButton(dynamic_cast<SignalGenerator*>(siggenTme->tool())->getRunButton());

		connect(m_m2kController, SIGNAL(calibrationStarted()), this, SLOT(calibrationStarted()));
		connect(m_m2kController, SIGNAL(calibrationSuccess()), this, SLOT(calibrationSuccess()));
		connect(m_m2kController, SIGNAL(calibrationFailed()), this, SLOT(calibrationFinished()));

		for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
			tme->setEnabled(true);
			tme->setRunBtnVisible(true);
			tme->setRunning(false);
		}
		m_m2kController->initialCalibration();
	} catch(...) {
		qWarning(CAT_M2KPLUGIN) << "M2K plugin connection failed!";
		cleanup();
		return false;
	}
	return true;
}

bool M2kPlugin::onDisconnect()
{
	cleanup();
	return true;
}

QString M2kPlugin::description()
{
	return "Implements ADALM2000 instruments - Oscilloscope, Spectrum Analyzer, Voltmeter, Network Analyzer, "
	       "Signal Generator, Logic Analyzer, Pattern Generator, Digital I/O";
}

void M2kPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "disconnectDevOnConnectFailure":true,
	   "priority":100,
	   "category":[
	      "iio",
	      "m2k"
	   ],
	   "exclude":["*", "!debuggerplugin"]
	}
)plugin");
}

#include "moc_m2kplugin.cpp"
