/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "preferences.h"
#include "ui_preferences.h"
#include "dynamicWidget.hpp"

#include <QElapsedTimer>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

using namespace adiscope;

Preferences::Preferences(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preferences),
	sig_gen_periods_nr(2),
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
	m_initialized(false)
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
	connect(ui->sigGenNrPeriods, &QLineEdit::returnPressed, [=]() {
		bool isNumber = false;
		QString text = ui->sigGenNrPeriods->text();
		int nr_of_periods = text.toInt(&isNumber);

		if (!isNumber) {
			setDynamicProperty(ui->sigGenNrPeriods, "valid", false);
			setDynamicProperty(ui->sigGenNrPeriods, "invalid", true);
		} else if (nr_of_periods > 1 && nr_of_periods < 10) {
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
	connect(ui->decodersCheckBox, &QCheckBox::stateChanged, [=](int state){
		digital_decoders_enabled = (!state ? false : true);
		Q_EMIT notify();

		if (m_initialized) {
			QMessageBox info(this);
			info.setText("This changes will be applied only after a Scopy reset.");
			info.exec();
		} else {
			m_initialized = true;
		}
	});

	QString preference_ini_file = getPreferenceIniFile();
	QSettings settings(preference_ini_file, QSettings::IniFormat);

	pref_api->setObjectName(QString("Preferences"));
	pref_api->load(settings);
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

	QWidget::showEvent(event);
}

QString Preferences::getPreferenceIniFile() const
{
	QSettings settings;
	QFileInfo fileInfo(settings.fileName());
	QString preference_ini_file = fileInfo.absolutePath() + "/Preferences.ini";

	return preference_ini_file;
}

void Preferences::resetScopy()
{
	QMessageBox msgBox;
	msgBox.setText("By resetting scopy you will lose the current configuration!");
	msgBox.setInformativeText("Do you want to reset?");
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
