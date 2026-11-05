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
#include "m2kcommon.h"
#include "manualcalibration.h"
#include "oscilloscope.hpp"
#include "power_controller.hpp"
#include "qtextbrowser.h"
#include "scopy-m2k_config.h"
#include "signal_generator.hpp"

#include <iio.h>

#include <component/attribute.h>
#include <component/context.h>
#include <component/controller.h>
#include <component/device.h>

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
	// Asked of the device-controller context Scopy already holds: no connection of our own.
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	for(const char *dev : {"m2k-adc", "m2k-dac-a", "m2k-dac-b", "m2k-fabric"}) {
		if(!ctx->findChild<component::Device *>(dev, Qt::FindDirectChildrenOnly)) {
			return false;
		}
	}
	return true;
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("m2ksiggen", tr("Signal Generator"),
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_signal_generator.svg"));
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
	// The context attributes, cached by the device controller when it built the context.
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(ctx) {
		for(component::Attribute *a :
		    ctx->findChildren<component::Attribute *>(QString(), Qt::FindDirectChildrenOnly)) {
			m_m2kInfoPage->update(a->name(), a->cachedValue());
		}
	}

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

	m_btnCalibrate->setDisabled(true);

	connect(m_btnIdentify, SIGNAL(clicked()), m_m2kController, SLOT(identify()));
	connect(m_btnCalibrate, SIGNAL(clicked()), m_m2kController, SLOT(calibrate()));
	connect(m_m2kController, &M2kController::calibrationStarted, this,
		[=]() { m_btnCalibrate->setEnabled(false); });
	connect(m_m2kController, &M2kController::calibrationFinished, this,
		[=]() { m_btnCalibrate->setEnabled(true); });

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
		// Not every tool is in every build.
		if(!tme) {
			continue;
		}
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
		if(!tme) {
			continue;
		}
		toolMenuEntryCalibrationCache[tool] = new ToolMenuEntry(*tme); // save tool state
	}
}

void M2kPlugin::restoreToolState(QStringList tools)
{
	for(const QString &tool : calibrationToolNames) {
		if(!toolMenuEntryCalibrationCache.contains(tool)) {
			continue;
		}
		ToolMenuEntry *cachedTme = toolMenuEntryCalibrationCache.take(tool);
		auto id = cachedTme->id();
		auto tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, id);
		if(!tme) {
			delete cachedTme;
			continue;
		}
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
	lay->setContentsMargins(0, 0, 0, 0);

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
	if(m_calib) {
		delete m_calib;
		m_calib = nullptr;
	}

	restoreToolState(calibrationToolNames);

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
}

void M2kPlugin::clearPingTask()
{
	if(m_cyclicalTask) {
		m_cyclicalTask->deleteLater();
		m_cyclicalTask = nullptr;
	}
}

bool M2kPlugin::onConnect()
{
	// libm2k opens its own libiio v0 context: this build's device controller is on libiio v1,
	// and the two never share a context.
	struct iio_context *ctx = nullptr;
	try {
		m_m2k = m2kOpen(m2kV0Uri(m_param).constData());
		if(!m_m2k) {
			return false;
		}
		ctx = m_m2k->getIioContext();
		m2k_man = new m2k_iio_manager();
		m_btnCalibrate->setDisabled(false);

		m_m2kController->connectM2k(m_m2k);

		Filter *f = new Filter(ctx);
		QJSEngine *js = ScopyJS::GetInstance()->engine();

		m_calib = new Calibration(m_m2k);

		auto diom = new DIOManager(m_m2k, f);
		auto dmmTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kdmm");
		auto mancalTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kcal");
		auto dioTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kdio");
		auto pwrTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kpower");
		auto siggenTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2ksiggen");
		auto oscTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "m2kosc");
		m_adcBtnGrp = new QButtonGroup(this);

		tools.insert("m2kdmm", new DMM(m_m2k, m_param, f, dmmTme, m2k_man));
		dmmTme->setTool(tools["m2kdmm"]);
		tools.insert("m2kcal", new ManualCalibration(m_m2k, f, mancalTme, nullptr, m_calib));
		mancalTme->setTool(tools["m2kcal"]);
		tools.insert("m2kdio", new DigitalIO(f, dioTme, diom, js, nullptr));
		dioTme->setTool(tools["m2kdio"]);
		tools.insert("m2kpower", new PowerController(m_m2k, pwrTme, js, nullptr));
		pwrTme->setTool(tools["m2kpower"]);
		tools.insert("m2ksiggen", new SignalGenerator(m_m2k, m_param, f, siggenTme, js, nullptr));
		siggenTme->setTool(tools["m2ksiggen"]);
		tools.insert("m2kosc", new Oscilloscope(m_m2k, m_param, f, oscTme, m2k_man, js, nullptr));
		oscTme->setTool(tools["m2kosc"]);
		// This build leaves out the spectrum, network and logic analyzers and the pattern
		// generator: they do not build against this tree yet.

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
