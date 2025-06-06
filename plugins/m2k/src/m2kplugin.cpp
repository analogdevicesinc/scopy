/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "m2kplugin.h"

#include "digitalchannel_manager.hpp"
#include "digitalio.hpp"
#include "dmm.hpp"
#include "filter.hpp"
#include "iioutil/connectionprovider.h"
#include "m2kcommon.h"
#include "manualcalibration.h"
#include "network_analyzer.hpp"
#include "oscilloscope.hpp"
#include "patterngenerator/pattern_generator.h"
#include "power_controller.hpp"
#include "qtextbrowser.h"
#include "scopy-m2k_config.h"
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
#include <style.h>

#include <libsigrokdecode/libsigrokdecode.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
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
	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);

	if(!conn)
		return false;

	//	ret = !!iio_context_find_device(ctx,"m2k-adc");
	//	ret = ret && !!iio_context_find_device(ctx,"m2k-dac-a");
	//	ret = ret && !!iio_context_find_device(ctx,"m2k-dac-b");

	Filter *f = new Filter(conn->context());
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
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kspec", tr("Spectrum Analyzer"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_spectrum_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2knet", tr("Network Analyzer"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_network_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2ksiggen", tr("Signal Generator"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_signal_generator.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2klogic", tr("Logic Analyzer"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_logic_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpattern", tr("Pattern Generator"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_pattern_generator.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kdio", tr("Digital I/O"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_io.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kdmm", tr("Voltmeter"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_voltmeter.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kpower", tr("Power Supply"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_power_supply.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2kcal", tr("Calibration"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_calibration.svg"));
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
	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);
	for(int i = 0; i < iio_context_get_attrs_count(conn->context()); i++) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(conn->context(), i, &name, &value);
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
	Style::setStyle(m_btnIdentify, style::properties::button::basicButton);
	m_extraButtons.append(m_btnIdentify);
	m_btnCalibrate = new QPushButton("Calibrate");
	Style::setStyle(m_btnCalibrate, style::properties::button::basicButton);
	m_extraButtons.append(m_btnCalibrate);
	m_btnRegister = new QPushButton("Register");
	Style::setStyle(m_btnRegister, style::properties::button::basicButton);
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

void M2kPlugin::startPingTask() { m_cyclicalTask->start(PING_PERIOD); }

void M2kPlugin::stopPingTask() { m_cyclicalTask->stop(); }

void M2kPlugin::onPausePingTask(bool pause)
{
	if(pause) {
		stopPingTask();
	} else {
		startPingTask();
	}
}

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
	p->init("m2k_debugger_plugin_included", false);
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
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_instrument_notes_active", "Instrument Notes",
		"Enable or disable a text section at the bottom of each ADALM2000 instrument, allowing "
		"the user to take notes. Disabled by default.",
		generalSection));

	generalSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_debugger_plugin_included", "Include debugger plugin",
				     "Include the Debugger in the current instrument list "
				     "when connected to an ADALM2000 device. Disabled by default.",
				     generalSection));

	// Analog tools preferences
	MenuSectionWidget *analogWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *analogSection = new MenuCollapseSection("Analog tools", MenuCollapseSection::MHCW_NONE,
								     MenuCollapseSection::MHW_BASEWIDGET, analogWidget);
	analogWidget->contentLayout()->setSpacing(10);
	analogWidget->contentLayout()->addWidget(analogSection);
	analogSection->contentLayout()->setSpacing(10);
	lay->addWidget(analogWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	analogSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_show_adc_filters", "Show ADC digital filter config",
				     "Enable/disable the configuration panel for ADC digital filters. "
				     "This allows manual control over the parameters of the filters.",
				     analogSection));
	analogSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_show_graticule", "Enable graticule",
		"Select whether the plot backbone (graticule) is displayed in the Oscilloscope.", analogSection));
	analogSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_mini_histogram", "Enable mini histogram",
		"Select whether the Oscilloscope plot displays a histogram on the right side.", analogSection));
	analogSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_osc_filtering", "Enable sample rate filters",
				     "Enable or disable the ADALM2000 ADC filters for all samplerates less than "
				     "100 MHz.",
				     analogSection));
	analogSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_osc_label", "Enable labels on the plot",
		"Enable or disable the plot labels in the Oscilloscope instrument.", analogSection));
	analogSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_spectrum_visible_peak_search", "Only search marker peaks in visible domain",
		"Enabled by default, this control allows the Spectrum Analyzer to search for marker "
		"peaks outside the visible frequency range.",
		analogSection));
	analogSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_na_show_zero", "Always display 0db value on the graph",
				     "Select whether the 0dB value is displayed as a reference point on the "
				     "Network Analyzer plot. This control is disabled by default.",
				     analogSection));
	analogSection->contentLayout()->addWidget(PREFERENCE_EDIT_VALIDATION(
		p, "m2k_siggen_periods", "Number of displayed periods",
		"Select the number of displayed signal periods in the Signal Generator. "
		"Default value is 2.",
		[](const QString &text) {
			// check if input is an positive integer
			bool ok;
			auto value = text.toInt(&ok);
			return ok && value > 0;
		},
		analogSection));

	// Logic tools preferences
	MenuSectionWidget *logicWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *logicSection = new MenuCollapseSection("Logic tools", MenuCollapseSection::MHCW_NONE,
								    MenuCollapseSection::MHW_BASEWIDGET, logicWidget);
	logicWidget->contentLayout()->setSpacing(10);
	logicWidget->contentLayout()->addWidget(logicSection);
	logicSection->contentLayout()->setSpacing(10);
	lay->addWidget(logicWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	logicSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_logic_separate_annotations", "Separate decoder annotations when exporting",
				     "Select whether to export decoder annotations and data separately from the "
				     "Logic Analyzer instrument. Disabled by default.",
				     logicSection));
	logicSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "m2k_logic_display_sampling_points", "Display sampling points when zoomed",
		"Select whether to display each sampling point when zoomed in on the Logic Analyzer. "
		"Disabled by default.",
		logicSection));
	logicSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "m2k_logic_display_sample_time", "Show sample and time info in decoder table",
				     "Select whether the sample and time detailed information is shown in the "
				     "decoder table of the Logic Analyzer. Enabled by default.",
				     logicSection));

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
		QWidget *tool = tme->tool();
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		tme->setTool(nullptr);
		if(tool) {
			delete tool;
		}
	}

	disconnect(m_m2kController, SIGNAL(calibrationStarted()), this, SLOT(calibrationStarted()));
	disconnect(m_m2kController, SIGNAL(calibrationSuccess()), this, SLOT(calibrationSuccess()));
	disconnect(m_m2kController, SIGNAL(calibrationFailed()), this, SLOT(calibrationFinished()));

	m_m2kController->disconnectM2k();
	m_btnCalibrate->setDisabled(true);
	clearPingTask();

	if(m_m2k) {
		try {
			contextClose(m_m2k);
		} catch(...) {
			qWarning(CAT_M2KPLUGIN) << "M2K plugin deinit on disconnect errored!";
		}
		m_m2k = nullptr;
	}

	ConnectionProvider *c = ConnectionProvider::GetInstance();
	c->close(m_param);
}

void M2kPlugin::clearPingTask()
{
	if(m_cyclicalTask) {
		m_cyclicalTask->deleteLater();
		m_cyclicalTask = nullptr;
	}
	if(m_pingTask) {
		m_pingTask->deleteLater();
		m_pingTask = nullptr;
	}
}

bool M2kPlugin::onConnect()
{
	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);

	if(!conn) {
		return false;
	}
	struct iio_context *ctx = conn->context();
	try {
		m_m2k = m2kOpen(ctx, m_param.toUtf8());
		m2k_man = new m2k_iio_manager();
		m_btnCalibrate->setDisabled(false);

		m_m2kController->connectM2k(m_m2k);
		m_pingTask = new IIOPingTask(ctx, this);
		m_cyclicalTask = new CyclicalTask(m_pingTask);

		Filter *f = new Filter(ctx);
		QJSEngine *js = ScopyJS::GetInstance()->engine();

		auto calib = new Calibration(m_m2k);
		auto diom = new DIOManager(m_m2k, f);
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

		tools.insert("m2kdmm", new DMM(m_m2k, m_param, f, dmmTme, m2k_man));
		dmmTme->setTool(tools["m2kdmm"]);
		tools.insert("m2kcal", new ManualCalibration(m_m2k, f, mancalTme, nullptr, calib));
		mancalTme->setTool(tools["m2kcal"]);
		tools.insert("m2kdio", new DigitalIO(f, dioTme, diom, js, nullptr));
		dioTme->setTool(tools["m2kdio"]);
		tools.insert("m2kpower", new PowerController(m_m2k, pwrTme, js, nullptr));
		pwrTme->setTool(tools["m2kpower"]);
		tools.insert("m2ksiggen", new SignalGenerator(m_m2k, m_param, f, siggenTme, js, nullptr));
		siggenTme->setTool(tools["m2ksiggen"]);
		tools.insert("m2kspec", new SpectrumAnalyzer(m_m2k, m_param, f, specTme, m2k_man, js, nullptr));
		specTme->setTool(tools["m2kspec"]);
		tools.insert("m2kosc", new Oscilloscope(m_m2k, m_param, f, oscTme, m2k_man, js, nullptr));
		oscTme->setTool(tools["m2kosc"]);
		tools.insert("m2knet", new NetworkAnalyzer(m_m2k, m_param, f, netTme, m2k_man, js, nullptr));
		netTme->setTool(tools["m2knet"]);
		tools.insert("m2klogic", new logic::LogicAnalyzer(m_m2k, f, laTme, js, nullptr));
		laTme->setTool(tools["m2klogic"]);

		logic::LogicAnalyzer *logic_analyzer = dynamic_cast<logic::LogicAnalyzer *>(tools["m2klogic"]);
		Oscilloscope *oscilloscope = dynamic_cast<Oscilloscope *>(tools["m2kosc"]);
		NetworkAnalyzer *network_analyzer = dynamic_cast<NetworkAnalyzer *>(tools["m2knet"]);

		oscilloscope->setLogicAnalyzer(logic_analyzer);
		network_analyzer->setOscilloscope(oscilloscope);

		tools.insert("m2kpattern", new logic::PatternGenerator(m_m2k, f, pgTme, js, diom, nullptr));
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

QString M2kPlugin::pkgName() { return M2K_PKG_NAME; }

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
	   "exclude":["*"]
	}
)plugin");

	// Add !debuggerplugin to exclude list
	if(Preferences::get("m2k_debugger_plugin_included").toBool()) {
		if(!m_metadata.contains("exclude")) {
			qWarning(CAT_M2KPLUGIN) << "Cannot find \"exclude\" key in M2kPlugin metadata";
		} else {
			m_metadata["exclude"] = QJsonArray({"*", "!debuggerplugin"});
		}
	}
}

#include "moc_m2kplugin.cpp"
