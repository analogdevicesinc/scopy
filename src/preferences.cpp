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
#include "dynamicWidget.hpp"

#include <QElapsedTimer>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <scopyApplication.hpp>


using namespace adiscope;


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
	m_instrument_notes_active(false),
	m_debug_messages_active(false),
	m_attemptTempLutCalib(false),
	m_skipCalIfCalibrated(true)
{
	ui->setupUi(this);

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
	});

	connect(ui->decodersCheckBox, &QCheckBox::stateChanged, [=](int state){
		digital_decoders_enabled = (!state ? false : true);
		Q_EMIT notify();

		if (m_initialized) {
			QMessageBox info(this);
			info.setText(tr("This change will be applied only after a Scopy reset."));
			info.exec();
		}
	});

	connect(ui->instrumentNotesCheckbox, &QCheckBox::stateChanged, [=](int state) {
		m_instrument_notes_active = (!state ? false : true);
		Q_EMIT notify();
	});

	connect(ui->tempLutCalibCheckbox, &QCheckBox::stateChanged, [=](int state) {
		m_attemptTempLutCalib = state;
		Q_EMIT notify();
	});

	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);

	pref_api->setObjectName(QString("Preferences"));
	pref_api->load(settings);

	ui->label_restart->setVisible(false);
	//////////////////////
	// TEMPORARY UNTIL ACTUAL IMPLEMENTATION
	ui->tempLutCalibCheckbox->setVisible(false);
	ui->label_28->setVisible(false);
	//////////////////////////

	ui->languageCombo->addItems(getOptionsList());


	connect(ui->languageCombo, &QComboBox::currentTextChanged, [=](QString lang) {
		if(lang == "browse"){
			QString langtemp = loadLanguage();
			if(!langtemp.isEmpty()) {
				language=langtemp;
				ui->languageCombo->addItem(language);
				ui->languageCombo->setCurrentText(language);
				if (m_initialized) {
					ui->label_restart->setVisible(true);
				}
			} else {
				if(!getLanguageList().contains(language)){
					QFileInfo info(language);
					language = info.fileName().remove(".qm");
				}
				ui->languageCombo->setCurrentText(language);
				ui->label_restart->setVisible(false);
			}
		} else {
			language = lang;
			if (m_initialized) {
				ui->label_restart->setVisible(true);
			}

			Q_EMIT notify();
		}
	});

	connect(ui->logicAnalyzerDisplaySamplingPoints, &QCheckBox::stateChanged, [=](int state){
		m_displaySamplingPoints = (!state ? false : true);
		Q_EMIT notify();
	});
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

void Preferences::showEvent(QShowEvent *event)
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
	ui->instrumentNotesCheckbox->setChecked(m_instrument_notes_active);
	ui->debugMessagesCheckbox->setChecked(m_debug_messages_active);
	ui->debugInstrumentCheckbox->setChecked(debugger_enabled);
	ui->tempLutCalibCheckbox->setChecked(m_attemptTempLutCalib);
	ui->skipCalCheckbox->setChecked(m_skipCalIfCalibrated);
	// by this point the preferences menu is initialized
	m_initialized = true;

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
