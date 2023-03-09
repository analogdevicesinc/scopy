/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "preferences.h"
#include "ui_preferences.h"
#include "gui/dynamicWidget.hpp"

#include <QElapsedTimer>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <scopyApplication.hpp>
#include "application_restarter.h"
#include "utils.h"


using namespace adiscope;

Preferences* adiscope::pref_ptr;
Preferences* adiscope::getScopyPreferences() {
	return pref_ptr;
}

Preferences::Preferences(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preferences),
	sig_gen_periods_nr(1),
	save_session_on_exit(true),
	double_click_to_detach(false),
	pref_api(new Preferences_API(this)),
	spectrum_visible_peak_search(true),
	osc_labels_enabled(false),
	na_show_zero(false),
	advanced_device_info(false),
	user_notes_active(false),
	external_script_enabled(false),
	manual_calib_script_enabled(false),
	debugger_enabled(false),
	manual_calib_enabled(false),
	graticule_enabled(false),
	animations_enabled(true),
	osc_filtering_enabled(true),
	mini_hist_enabled(false),
	digital_decoders_enabled(true),
	m_initialized(false),
	show_ADC_digital_filters(false),
	m_useNativeDialogs(true),
	language("auto"),
	m_displaySamplingPoints(false),
	m_separateAnnotations(false),
	m_tableInfo(true),
	m_instrument_notes_active(false),
	m_debug_messages_active(false),
	m_attemptTempLutCalib(false),
	m_skipCalIfCalibrated(true),
	automatical_version_checking_enabled(false),
	first_application_run(true),
	check_updates_url("http://swdownloads.analog.com/cse/sw_versions.json"),
	m_colorEditor(nullptr),
	m_logging_enabled(false),
	m_show_plot_fps(false),
	m_use_open_gl(
// Android/macOS/Windows use OpenGL by default
#if defined __ANDROID__  || defined __APPLE__ || defined __MINGW32__
			true
#else
			false
#endif
			),
	m_target_fps(30),
	m_docking_enabled(false)
{
	ui->setupUi(this);

	/** Will still be available to use for development purposes.
	 *  For now, no longer exposed to the users.**/
	ui->debugMessagesCheckbox->setVisible(false);
	ui->debugMessagesLbl->setVisible(false);

#ifdef __ANDROID__
	ui->doubleClickToDetachWidget->setDisabled(true);
#endif
	connect(ui->doubleClickCheckBox, &QCheckBox::stateChanged, [=](int state){
		double_click_to_detach = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->oscLabelsCheckBox, &QCheckBox::stateChanged, [=](int state) {
		osc_labels_enabled = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->oscADCFiltersCheckBox, &QCheckBox::stateChanged, [=](int state) {
		show_ADC_digital_filters = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->sigGenNrPeriods, &QLineEdit::returnPressed, [=]() {
		bool isNumber = false;
		QString text = ui->sigGenNrPeriods->text();
		int nr_of_periods = text.toInt(&isNumber);

		if (!isNumber) {
			setDynamicProperty(ui->sigGenNrPeriods, "valid", false);
			setDynamicProperty(ui->sigGenNrPeriods, "invalid", true);
		} else if (nr_of_periods >= 1 && nr_of_periods < 10) {
			setDynamicProperty(ui->sigGenNrPeriods, "invalid", false);
			setDynamicProperty(ui->sigGenNrPeriods, "valid", true);
			sig_gen_periods_nr = nr_of_periods;
			Q_EMIT notify();
		} else {
			setDynamicProperty(ui->sigGenNrPeriods, "valid", false);
			setDynamicProperty(ui->sigGenNrPeriods, "invalid", true);
		}

	});
	connect(ui->saveSessionCheckBox, &QCheckBox::stateChanged, [=](int state) {
		save_session_on_exit = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->resetBtn, &QPushButton::clicked, this,
	        &Preferences::resetScopy);

	connect(ui->na_zeroCheckBox, &QCheckBox::stateChanged, [=](int state) {
		na_show_zero = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->spectrumVisiblePeakSearch, &QCheckBox::stateChanged, [=](int state) {
		spectrum_visible_peak_search = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->advancedInfoCheckBox, &QCheckBox::stateChanged, [=](int state) {
		advanced_device_info = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->userNotesCheckBox, &QCheckBox::stateChanged, [=](int state) {
		user_notes_active = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->oscGraticuleCheckBox, &QCheckBox::stateChanged, [=](int state) {
		graticule_enabled = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->extScriptCheckBox, &QCheckBox::stateChanged, [=](int state) {
		external_script_enabled = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->manualCalibCheckBox, &QCheckBox::stateChanged, [=](int state) {
		manual_calib_script_enabled = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->skipCalCheckbox, &QCheckBox::stateChanged, [=](int state) {
		m_skipCalIfCalibrated = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->enableLoggingCheckBox, &QCheckBox::stateChanged, [=](int state) {
		m_logging_enabled = !!state;
		Q_EMIT notify();
	});

	connect(ui->enableAnimCheckBox, &QCheckBox::stateChanged, [=](int state) {
		animations_enabled = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->oscFilteringCheckBox, &QCheckBox::stateChanged, [=](int state) {
		osc_filtering_enabled = (!state ? false : true);
		Q_EMIT notify();
	});
	connect(ui->histCheckBox, &QCheckBox::stateChanged, [=](int state){
		mini_hist_enabled = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->debugMessagesCheckbox, &QCheckBox::stateChanged, [=](int state){
		setDebugMessagesActive(state);
		Q_EMIT notify();
	});

	connect(ui->debugInstrumentCheckbox, &QCheckBox::stateChanged, [=](int state){
		debugger_enabled = (!state ? false : true);
		Q_EMIT notify();
		requestRestart();
	});

	connect(ui->decodersCheckBox, &QCheckBox::stateChanged, [=](int state){
		digital_decoders_enabled = (!state ? false : true);
		Q_EMIT notify();

		if (m_initialized) {
#ifdef __ANDROID__
			forceSavePreferences();
#endif
			requestRestart();
		}
	});
	connect(ui->autoUpdatesCheckBox, &QCheckBox::stateChanged, [=](int state) {
		automatical_version_checking_enabled = (!state ? false : true);
		if(automatical_version_checking_enabled) {
			Q_EMIT requestUpdateCheck();
		}
		Q_EMIT notify();
	});

	connect(ui->instrumentNotesCheckbox, &QCheckBox::stateChanged, [=](int state) {
		m_instrument_notes_active = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->tempLutCalibCheckbox, &QCheckBox::stateChanged, [=](int state) {
		m_attemptTempLutCalib = state;
		Q_EMIT notify();
	});

	connect(ui->showPlotFps, &QCheckBox::stateChanged, [=](int state) {
		m_show_plot_fps = state;
		Q_EMIT notify();
	});

	connect(ui->enableDockableWidgetsCheckBox, &QCheckBox::stateChanged, [=](int state){
		m_docking_enabled = (!state ? false : true);

		requestRestart();
	});

	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);

	pref_api->setObjectName(QString("Preferences"));
	pref_api->load(settings);

	ui->languageCombo->addItems(getOptionsList());


	connect(ui->languageCombo, &QComboBox::currentTextChanged, [=](QString lang) {
		if(lang == "browse"){
			QString langtemp = loadLanguage();
			if(!langtemp.isEmpty()) {
				language=langtemp;
				ui->languageCombo->addItem(language);
				ui->languageCombo->setCurrentText(language);
				if (m_initialized) {
					requestRestart();
				}
			} else {
				if(!getLanguageList().contains(language)){
					QFileInfo info(language);
					language = info.fileName().remove(".qm");
				}
				ui->languageCombo->setCurrentText(language);
			}
		} else {
			language = lang;
			if (m_initialized) {
#ifdef __ANDROID__
				forceSavePreferences();
#endif
				requestRestart();
			}

			Q_EMIT notify();
		}
	});

	connect(ui->logicAnalyzerDisplaySamplingPoints, &QCheckBox::stateChanged, [=](int state){
		m_displaySamplingPoints = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->logicAnalyzerSeparateAnnotations, &QCheckBox::stateChanged, [=](int state){
		m_separateAnnotations = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->logicAnalyzerTableInfo, &QCheckBox::stateChanged, [=](int state){
		m_tableInfo = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->useOpenGl, &QCheckBox::stateChanged, [=](int state){
		m_use_open_gl = state;
		qputenv("SCOPY_USE_OPENGL",QByteArray::number(state));

		if (m_initialized) {
			requestRestart();
		}
		Q_EMIT notify();
	});

	connect(ui->cmbPlotTargetFps, &QComboBox::currentTextChanged, [=](QString fps){
		m_target_fps = fps.toDouble();
		Q_EMIT notify();
	});

	ui->comboBoxTheme->addItem("default");
	ui->comboBoxTheme->addItem("light");
	ui->comboBoxTheme->addItem("browse");
	connect(ui->comboBoxTheme, &QComboBox::currentTextChanged, [=](const QString &stylesheet) {
		if (stylesheet == "browse") {
			QString filePath = QFileDialog::getOpenFileName(this,
					tr("Load Theme"), "", tr("Stylesheet files (*.qss)"),
					nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

			if (filePath.isEmpty()) {
				QSignalBlocker blocker(ui->comboBoxTheme);
				ui->comboBoxTheme->setCurrentText(m_colorEditor->getCurrentStylesheet());
				return;
			}

			m_colorEditor->setUserStylesheets({filePath});
			m_colorEditor->setCurrentStylesheet(filePath);
		} else {
			m_colorEditor->setCurrentStylesheet(stylesheet);
			forceSavePreferences();
		}
		requestRestart();
	});
	pref_ptr = this;
	initializePreferences();
}

void Preferences::save() {
	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);
	pref_api->save(settings);
}

void Preferences::requestRestart()
{
	QMessageBox msgBox;
	msgBox.setText(tr("An application restart is required for these settings to take effect .. "));
	msgBox.setInformativeText(tr("Do you want to restart now ?"));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	auto buttonOk = msgBox.button(QMessageBox::Ok);
	auto buttonCancel = msgBox.button(QMessageBox::Cancel);
	buttonOk->setText("Now");
	buttonCancel->setText("Later");
	int ret = msgBox.exec();

	if (ret == QMessageBox::Ok) {
		save(); // save before restarting
		adiscope::ApplicationRestarter::triggerRestart();
	}
}

QStringList Preferences::getOptionsList()
{   
	QString currentLanguage = pref_api->getLanguage();
	QStringList options; 
	if(!getLanguageList().contains(currentLanguage) && currentLanguage.endsWith(".qm")) {
		QFileInfo info(currentLanguage);
		options<<info.fileName().remove(".qm")<<getLanguageList()<<"auto"<<"browse";
		ui->languageCombo->setCurrentText(info.fileName().remove(".qm"));
	}
	else
		options<<getLanguageList()<<"auto"<<"browse";
	return options;
}

QStringList Preferences::getLanguageList()
{
	QDir directory(":/translations");
	QStringList languages = directory.entryList(QStringList() << "*.qm" << "*.QM", QDir::Files);
	for(auto &s : languages) {
		s.remove(".qm");
	}
	return languages;
}

bool Preferences::getDisplaySamplingPoints() const
{
	return m_displaySamplingPoints;
}

void Preferences::setDisplaySamplingPoints(bool display)
{
	m_displaySamplingPoints = display;
}

bool Preferences::getTableInfo() const
{
	return m_tableInfo;
}

void Preferences::setTableInfo(bool flag)
{
	m_tableInfo = flag;
}

bool Preferences::getSeparateAnnotations() const
{
	return m_separateAnnotations;
}

void Preferences::setSeparateAnnotations(bool flag)
{
	m_separateAnnotations = flag;
}

bool Preferences::getInstrumentNotesActive() const
{
	return m_instrument_notes_active;
}
void Preferences::setInstrumentNotesActive(bool en)
{
	m_instrument_notes_active = en;
}

Preferences::~Preferences()
{
	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);
	pref_api->save(settings);
	delete pref_api;
	delete ui;
}

void Preferences::notifyChange()
{
	Q_EMIT notify();
}

void Preferences::initializePreferences()
{

	setDynamicProperty(ui->sigGenNrPeriods, "invalid", false);
	setDynamicProperty(ui->sigGenNrPeriods, "valid", true);

	ui->sigGenNrPeriods->setText(QString::number(sig_gen_periods_nr));
	ui->oscLabelsCheckBox->setChecked(osc_labels_enabled);
	ui->saveSessionCheckBox->setChecked(save_session_on_exit);
	ui->doubleClickCheckBox->setChecked(double_click_to_detach);
	ui->na_zeroCheckBox->setChecked(na_show_zero);
	ui->advancedInfoCheckBox->setChecked(advanced_device_info);
	ui->userNotesCheckBox->setChecked(user_notes_active);
	ui->oscGraticuleCheckBox->setChecked(graticule_enabled);
	ui->extScriptCheckBox->setChecked(external_script_enabled);
	ui->manualCalibCheckBox->setChecked(manual_calib_script_enabled);
	ui->enableAnimCheckBox->setChecked(animations_enabled);
	ui->oscFilteringCheckBox->setChecked(osc_filtering_enabled);
	ui->histCheckBox->setChecked(mini_hist_enabled);
	ui->decodersCheckBox->setChecked(digital_decoders_enabled);
	ui->oscADCFiltersCheckBox->setChecked(show_ADC_digital_filters);
	ui->languageCombo->setCurrentText(language);
	ui->logicAnalyzerDisplaySamplingPoints->setChecked(m_displaySamplingPoints);
	ui->logicAnalyzerSeparateAnnotations->setChecked(m_separateAnnotations);
	ui->logicAnalyzerTableInfo->setChecked(m_tableInfo);
	ui->instrumentNotesCheckbox->setChecked(m_instrument_notes_active);
	ui->debugMessagesCheckbox->setChecked(m_debug_messages_active);
	ui->debugInstrumentCheckbox->setChecked(debugger_enabled);
	ui->tempLutCalibCheckbox->setChecked(m_attemptTempLutCalib);
	ui->skipCalCheckbox->setChecked(m_skipCalIfCalibrated);
	ui->showPlotFps->setChecked(m_show_plot_fps);
	ui->useOpenGl->setChecked(m_use_open_gl);
	ui->cmbPlotTargetFps->setCurrentText(QString::number(m_target_fps));

	// requires restart after stateChanged, we avoid that here
	ui->enableDockableWidgetsCheckBox->blockSignals(true);
	ui->enableDockableWidgetsCheckBox->setChecked(m_docking_enabled);
	ui->enableDockableWidgetsCheckBox->blockSignals(false);

	// by this point the preferences menu is initialized
	m_initialized = true;
	ui->autoUpdatesCheckBox->setChecked(automatical_version_checking_enabled);
#ifdef LIBM2K_ENABLE_LOG
	ui->enableLoggingCheckBox->setChecked(m_logging_enabled);
	ui->loggingUnavailableLabel->setVisible(false);
#else
	ui->enableLoggingCheckBox->setChecked(false);
	ui->enableLoggingCheckBox->setCheckable(false);
	ui->loggingUnavailableLabel->setVisible(true);
	m_logging_enabled = false;
#endif
}

void Preferences::showEvent(QShowEvent *event)
{
	initializePreferences();
	QWidget::showEvent(event);
}

QString Preferences::getPreferenceIniFile()
{
	QSettings settings;
	QFileInfo fileInfo(settings.fileName());
	QString preference_ini_file = fileInfo.absolutePath() + "/Preferences.ini";
	return preference_ini_file;
}

void Preferences::resetScopy()
{
	QMessageBox msgBox;
	msgBox.setText(tr("By resetting scopy you will lose the current configuration!"));
	msgBox.setInformativeText(tr("Do you want to reset?"));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	int ret = msgBox.exec();

	if (ret == QMessageBox::Ok) {
		Q_EMIT reset();
	}
}

double Preferences::getTarget_fps() const
{
	return m_target_fps;
}

void Preferences::setTarget_fps(double newTarget_fps)
{
	m_target_fps = newTarget_fps;
}

bool Preferences::getUse_open_gl() const
{
	return m_use_open_gl;
}

void Preferences::setUse_open_gl(bool newUse_open_gl)
{
	m_use_open_gl = newUse_open_gl;
}

bool Preferences::getShow_plot_fps() const
{
	return m_show_plot_fps;
}

void Preferences::setShow_plot_fps(bool newShow_plot_fps)
{
	m_show_plot_fps = newShow_plot_fps;
}

void Preferences::forceSavePreferences()
{
	// force saving of the ini file as the new Scopy process
	// when restarted will start before scopy closes. A race condition
	// will appear on who gets to read/write to the .ini file first
	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);
	pref_api->save(settings);
}

bool Preferences::getDigital_decoders_enabled() const
{
	return digital_decoders_enabled;
}

void Preferences::setDigital_decoders_enabled(bool value)
{
	digital_decoders_enabled = value;
}

bool Preferences::getOsc_filtering_enabled() const
{
    return osc_filtering_enabled;
}

void Preferences::setOsc_filtering_enabled(bool value)
{
    osc_filtering_enabled = value;
}

bool Preferences::getAnimations_enabled() const
{
	return animations_enabled;
}

void Preferences::setAnimations_enabled(bool value)
{
    animations_enabled = value;
}


bool Preferences::getShowADCFilters() const
{
	return show_ADC_digital_filters;
}

void Preferences::setShowADCFilters(bool value)
{
    show_ADC_digital_filters = value;
}

bool Preferences::getMini_hist_enabled() const
{
	return mini_hist_enabled;
}

void Preferences::setMini_hist_enabled(bool value)
{
	mini_hist_enabled = value;
}

bool Preferences::getAdvanced_device_info() const
{
    return advanced_device_info;
}

void Preferences::setAdvanced_device_info(bool value)
{
    advanced_device_info = value;
}

bool Preferences::getUser_notes_active() const
{
	return user_notes_active;
}

void Preferences::setUser_notes_active(bool value)
{
	user_notes_active = value;
}

bool Preferences::getSpectrum_visible_peak_search() const
{
	return spectrum_visible_peak_search;
}

void Preferences::setSpectrum_visible_peak_search(bool value)
{
	spectrum_visible_peak_search = value;
}

bool Preferences::getDouble_click_to_detach() const
{
	return double_click_to_detach;
}

bool Preferences::getDebugMessagesActive() const
{
	return m_debug_messages_active;
}
void Preferences::setDebugMessagesActive(bool val)
{
	m_debug_messages_active = val;
	GetScopyApplicationInstance()->setDebugMode(val);
}

void Preferences::setDouble_click_to_detach(bool value)
{
	double_click_to_detach = value;
}

bool Preferences::getSave_session_on_exit() const
{
	return save_session_on_exit;
}

void Preferences::setSave_session_on_exit(bool value)
{
	save_session_on_exit = value;
}

int Preferences::getSig_gen_periods_nr() const
{
	return sig_gen_periods_nr;
}

void Preferences::setSig_gen_periods_nr(int value)
{
	sig_gen_periods_nr = value;
}

bool Preferences::getOsc_labels_enabled() const
{
	return osc_labels_enabled;
}

void Preferences::setOsc_labels_enabled(bool value)
{
	osc_labels_enabled = value;
}

bool Preferences::getExternal_script_enabled() const
{
	return external_script_enabled;
}

void Preferences::setExternal_script_enabled(bool value)
{
	external_script_enabled = value;
	ui->extScriptCheckBox->setChecked(external_script_enabled);
	Q_EMIT notify();
}

bool Preferences::getManual_calib_script_enabled() const
{
	return manual_calib_script_enabled;
}

void Preferences::setManual_calib_script_enabled(bool value)
{
	manual_calib_script_enabled = value;
	ui->manualCalibCheckBox->setChecked(manual_calib_script_enabled);
	Q_EMIT notify();
}


bool Preferences::getDebugger_enabled() const
{
	return debugger_enabled;
}

void Preferences::setDebugger_enabled(bool value)
{
	debugger_enabled = value;
	if (debugger_enabled) {
		ui->extScriptWidget->show();
	} else {
		ui->extScriptWidget->hide();
	}
	setExternal_script_enabled(external_script_enabled);
}

void Preferences::setManual_calib_enabled(bool value)
{
	manual_calib_enabled = value;
	if (manual_calib_enabled) {
		ui->manualCalibWidget->show();
	} else {
		ui->manualCalibWidget->hide();
	}
	setManual_calib_script_enabled(manual_calib_script_enabled);
}

bool Preferences::getNa_show_zero() const
{
	return na_show_zero;
}

void Preferences::setNa_show_zero(bool value)
{
	na_show_zero = value;
}

bool Preferences::getOsc_graticule_enabled() const
{
	return graticule_enabled;
}

void Preferences::setOsc_graticule_enabled(bool value)
{
	graticule_enabled = value;
}

bool Preferences::getAttemptTempLutCalib() const
{
	return m_attemptTempLutCalib;
}
void Preferences::setAttemptTempLutCalib(bool val)
{
	m_attemptTempLutCalib = val;
}

bool Preferences::getSkipCalIfCalibrated() const
{
	return m_skipCalIfCalibrated;
}
void Preferences::setSkipCalIfCalibrated(bool val)
{
	m_skipCalIfCalibrated = val;
}
bool Preferences::getAutomatical_version_checking_enabled() const
{
	return automatical_version_checking_enabled;
}

void Preferences::setAutomatical_version_checking_enabled(bool value)
{
	automatical_version_checking_enabled = value;
}

QString Preferences::getCheck_updates_url() const
{
	return check_updates_url;
}

void Preferences::setCheck_update_url(const QString& link)
{
	check_updates_url = link;
}

bool Preferences::getFirst_application_run() const
{
	return first_application_run;
}

void Preferences::setFirst_application_run(bool value)
{
	first_application_run = value;
}

void Preferences::setColorEditor(ScopyColorEditor *colorEditor)
{
	m_colorEditor = colorEditor;
	QSignalBlocker blocker(ui->comboBoxTheme);
	if (m_colorEditor->getUserStylesheets().size()) {
		ui->comboBoxTheme->insertItem(2, m_colorEditor->getUserStylesheets().back());
	}
	ui->comboBoxTheme->setCurrentText(m_colorEditor->getCurrentStylesheet());
	QIcon::setThemeName("scopy-" + m_colorEditor->getCurrentStylesheet());
}

bool Preferences::getLogging_enabled() const
{
	return m_logging_enabled;
}

void Preferences::setLogging_enabled(bool value)
{
	m_logging_enabled = value;
}

bool Preferences::getDocking_enabled() const
{
	return m_docking_enabled;
}

void Preferences::setDocking_enabled(bool value)
{
	m_docking_enabled = value;
}

bool Preferences::getCurrent_docking_enabled() const
{
	return m_current_docking_state;
}

bool Preferences_API::getAnimationsEnabled() const
{
	return preferencePanel->animations_enabled;
}

void Preferences_API::setAnimationsEnabled(const bool &enabled)
{
	preferencePanel->animations_enabled = enabled;
}

bool Preferences_API::getOscLabelsEnabled() const
{
	return preferencePanel->osc_labels_enabled;
}

void Preferences_API::setOscLabelsEnabled(const bool& enabled)
{
	preferencePanel->osc_labels_enabled = enabled;
}

int Preferences_API::getSigGenNrPeriods() const
{
	return preferencePanel->sig_gen_periods_nr;
}

void Preferences_API::setSigGenNrPeriods(const int& periods)
{
	preferencePanel->sig_gen_periods_nr = periods;
}

bool Preferences_API::getSaveSession() const
{
	return preferencePanel->save_session_on_exit;
}

void Preferences_API::setSaveSession(const bool& enabled)
{
	preferencePanel->save_session_on_exit = enabled;
}

bool Preferences_API::getDoubleClickToDetach() const
{
	return preferencePanel->double_click_to_detach;
}

void Preferences_API::setDoubleClickToDetach(const bool &enabled)
{
	preferencePanel->double_click_to_detach = enabled;
}

bool Preferences_API::getNaShowZero() const
{
	return preferencePanel->na_show_zero;
}

void Preferences_API::setNaShowZero(const bool& enabled)
{
	preferencePanel->na_show_zero = enabled;
}

bool Preferences_API::getSpectrumVisiblePeakSearch() const
{
	return preferencePanel->spectrum_visible_peak_search;
}

void Preferences_API::setSpectrumVisiblePeakSearch(const bool &enabled)
{
	preferencePanel->spectrum_visible_peak_search = enabled;
}

bool Preferences_API::getAdvancedDeviceInfo() const
{
	return preferencePanel->advanced_device_info;
}

void Preferences_API::setAdvancedDeviceInfo(const bool& enabled)
{
	preferencePanel->advanced_device_info = enabled;
}

bool Preferences_API::getUserNotesActive() const
{
	return preferencePanel->user_notes_active;
}

void Preferences_API::setUserNotesActive(const bool& enabled)
{
	preferencePanel->user_notes_active = enabled;
}

bool Preferences_API::getGraticuleEnabled() const
{
	return preferencePanel->graticule_enabled;
}
void Preferences_API::setGraticuleEnabled(const bool& enabled)
{
	preferencePanel->graticule_enabled = enabled;
}

bool Preferences_API::getInstrumentNotesActive() const
{
	return preferencePanel->m_instrument_notes_active;
}
void Preferences_API::setInstrumentNotesActive(bool en)
{
	preferencePanel->m_instrument_notes_active = en;
}

bool Preferences_API::getExternalScript() const
{
	return preferencePanel->external_script_enabled;
}
void Preferences_API::setExternalScript(const bool& enabled)
{
	preferencePanel->external_script_enabled = enabled;
}

bool Preferences_API::getManualCalibScript() const
{
	return preferencePanel->manual_calib_script_enabled;
}

void Preferences_API::setManualCalibScript(const bool &enabled)
{
	preferencePanel->manual_calib_script_enabled = enabled;
}

bool Preferences_API::getOscFilteringEnabled() const
{
	return preferencePanel->osc_filtering_enabled;
}

void Preferences_API::setOscFilteringEnabled(const bool &enabled)
{
	preferencePanel->osc_filtering_enabled = enabled;
}

bool Preferences_API::getShowADCDigitalFilters() const
{
	return preferencePanel->show_ADC_digital_filters;
}

void Preferences_API::setShowADCDigitalFilters(const bool &enabled)
{
	preferencePanel->show_ADC_digital_filters = enabled;
}


bool Preferences_API::getMiniHist() const
{
	return preferencePanel->mini_hist_enabled;
}

void Preferences_API::setMiniHist(const bool &enabled)
{
	preferencePanel->mini_hist_enabled = enabled;
}

bool Preferences_API::getDigitalDecoders() const
{
	return preferencePanel->digital_decoders_enabled;
}

void Preferences_API::setDigitalDecoders(bool enabled)
{
	preferencePanel->digital_decoders_enabled = enabled;
}

bool Preferences_API::getAttemptTempLutCalib() const
{
	return preferencePanel->m_attemptTempLutCalib;
}
void Preferences_API::setAttemptTempLutCalib(bool val)
{
	preferencePanel->m_attemptTempLutCalib = val;
}

bool Preferences_API::getSkipCalIfCalibrated() const
{
	return preferencePanel->m_skipCalIfCalibrated;
}
void Preferences_API::setSkipCalIfCalibrated(bool val)
{
	preferencePanel->m_skipCalIfCalibrated = val;
}

bool Preferences_API::getSeparateAnnotations() const
{
	return preferencePanel->m_separateAnnotations;
}
void Preferences_API::setSeparateAnnotations(bool val)
{
	preferencePanel->m_separateAnnotations = val;
}

bool Preferences_API::getTableInfo() const
{
	return preferencePanel->m_tableInfo;
}
void Preferences_API::setTableInfo(bool val)
{
	preferencePanel->m_tableInfo = val;
}


QString Preferences_API::getCurrentStylesheet() const
{
	if (!preferencePanel->m_colorEditor) {
		return "";
	}

	return preferencePanel->m_colorEditor->getCurrentStylesheet();
}

void Preferences_API::setCurrentStylesheet(const QString &currentStylesheet)
{
//	preferencePanel->m_colorEditor->setCurrentStylesheet(currentStylesheet);
}

QStringList Preferences_API::getUserStylesheets() const
{
	if (!preferencePanel->m_colorEditor) {
		return QStringList();
	}

	return preferencePanel->m_colorEditor->getUserStylesheets();
}

void Preferences_API::setUserStylesheets(const QStringList &userStylesheets)
{
	//	preferencePanel->m_colorEditor->setUserStylesheets(userStylesheets);
}

bool Preferences_API::getDockingEnabled() const
{
	return preferencePanel->m_docking_enabled;
}

void Preferences_API::setDockingEnabled(const bool &first)
{
	preferencePanel->m_docking_enabled = first;
	preferencePanel->m_current_docking_state = first;
}

bool Preferences::hasNativeDialogs() const
{
    return m_useNativeDialogs;
}

void Preferences::setNativeDialogs(bool nativeDialogs)
{
    m_useNativeDialogs = nativeDialogs;
}

QString Preferences::loadLanguage()
{
	QString validFile;
	QString filePath = QFileDialog::getOpenFileName(this,
			tr("Load language"), "", tr("Language files (*.qm)"),
			nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	if(!filePath.isEmpty())
		validFile = filePath;
	return validFile;

}

QString Preferences_API::getLanguage() const
{
	return preferencePanel->language;
}
void Preferences_API::setLanguage(QString lang)
{
	preferencePanel->language=lang;
}

bool Preferences_API::getDisplaySampling() const
{
	return preferencePanel->m_displaySamplingPoints;
}

void Preferences_API::setDisplaySampling(bool display)
{
	preferencePanel->m_displaySamplingPoints = display;
}

bool Preferences_API::getDebugMessagesActive() const
{
	return preferencePanel->m_debug_messages_active;
}
void Preferences_API::setDebugMessagesActive(bool val)
{
	preferencePanel->setDebugMessagesActive(val);
}
bool Preferences_API::getAutomaticalVersionCheckingEnabled() const
{
	return preferencePanel->automatical_version_checking_enabled;
}

void Preferences_API::setAutomaticalVersionCheckingEnabled(const bool &enabled)
{
	preferencePanel->automatical_version_checking_enabled = enabled;
}

QString Preferences_API::getCheckUpdatesUrl() const
{
	return preferencePanel->check_updates_url;
}

void Preferences_API::setCheckUpdatesUrl(const QString &link)
{
	preferencePanel->check_updates_url = link;
}

bool Preferences_API::getFirstApplicationRun() const
{
	return preferencePanel->first_application_run;
}

void Preferences_API::setFirstApplicationRun(const bool &first)
{
	preferencePanel->first_application_run = first;
}

bool Preferences_API::getShowPlotFps() const
{
	return preferencePanel->m_show_plot_fps;
}

void Preferences_API::setShowPlotFps(const bool& fps)
{
	preferencePanel->m_show_plot_fps = fps;
}

bool Preferences_API::getUseOpenGl() const
{
	return preferencePanel->m_use_open_gl;
}

void Preferences_API::setUseOpenGl(const bool& val)
{
	preferencePanel->m_use_open_gl = val;
	qputenv("SCOPY_USE_OPENGL",QByteArray::number(val));
}

double Preferences_API::getTargetFps() const
{
	return preferencePanel->m_target_fps;
}

void Preferences_API::setTargetFps(const double &val)
{
	preferencePanel->m_target_fps = val;
}
