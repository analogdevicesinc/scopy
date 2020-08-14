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

#ifndef PREFERENCE_PANEL_H
#define PREFERENCE_PANEL_H

#include <QWidget>
#include <QString>
#include <QSettings>

#include "apiObject.hpp"


namespace Ui {
class Preferences;
}

namespace adiscope {
class Preferences_API;

class Preferences : public QWidget
{
	friend class Preferences_API;

	Q_OBJECT

public:
	explicit Preferences(QWidget *parent = 0);
	~Preferences();

	static QString getPreferenceIniFile();

	void notifyChange();
	void showEvent(QShowEvent *event);

	bool getOsc_labels_enabled() const;
	void setOsc_labels_enabled(bool value);

	int getSig_gen_periods_nr() const;
	void setSig_gen_periods_nr(int value);

	bool getSave_session_on_exit() const;
	void setSave_session_on_exit(bool value);

	bool getDouble_click_to_detach() const;
	void setDouble_click_to_detach(bool value);

	bool getNa_show_zero() const;
	void setNa_show_zero(bool value);

	bool getSpectrum_visible_peak_search() const;
	void setSpectrum_visible_peak_search(bool value);

	bool getAdvanced_device_info() const;
	void setAdvanced_device_info(bool value);

	bool getUser_notes_active() const;
	void setUser_notes_active(bool value);

	bool getOsc_graticule_enabled() const;
	void setOsc_graticule_enabled(bool value);

	bool getExternal_script_enabled() const;
	void setExternal_script_enabled(bool value);

	bool getManual_calib_script_enabled() const;
	void setManual_calib_script_enabled(bool value);

	void setDebugger_enabled(bool value);
	void setManual_calib_enabled(bool value);

	bool getAnimations_enabled() const;
	void setAnimations_enabled(bool value);

	bool getOsc_filtering_enabled() const;
	void setOsc_filtering_enabled(bool value);

	bool getMini_hist_enabled() const;
	void setMini_hist_enabled(bool value);

	bool getDigital_decoders_enabled() const;
	void setDigital_decoders_enabled(bool value);

	bool getShowADCFilters() const ;
	void setShowADCFilters(bool value);
 
	QStringList getLanguageList();
	QStringList getOptionsList();
	bool hasNativeDialogs() const ;
	void setNativeDialogs(bool value);

	bool getDisplaySamplingPoints() const;
	void setDisplaySamplingPoints(bool display);

	bool getInstrumentNotesActive() const;
	void setInstrumentNotesActive(bool display);


Q_SIGNALS:

	void notify();
	void reset();

public Q_SLOTS:
	
	QString loadLanguage();


private Q_SLOTS:

	void resetScopy();

private:
	Ui::Preferences *ui;
	QString preferencesFileName;

	bool osc_labels_enabled;
	int sig_gen_periods_nr;
	bool save_session_on_exit;
	bool double_click_to_detach;
	bool na_show_zero;
	bool spectrum_visible_peak_search;
	bool advanced_device_info;
	bool user_notes_active;
	bool graticule_enabled;
	bool external_script_enabled;
	bool debugger_enabled;
	bool manual_calib_script_enabled;
	bool manual_calib_enabled;
	bool animations_enabled;
	bool osc_filtering_enabled;
	bool show_ADC_digital_filters;
	bool mini_hist_enabled;
	bool digital_decoders_enabled;
	bool m_initialized;
	bool m_useNativeDialogs;
	QString language;
	bool m_instrument_notes_active;
	bool m_displaySamplingPoints;

	Preferences_API *pref_api;
};

class Preferences_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool osc_labels READ getOscLabelsEnabled WRITE setOscLabelsEnabled);
	Q_PROPERTY(int sig_gen_nr_periods READ getSigGenNrPeriods WRITE
	           setSigGenNrPeriods);
	Q_PROPERTY(bool save_session_on_exit READ getSaveSession WRITE setSaveSession);
	Q_PROPERTY(bool double_click_to_detach READ getDoubleClickToDetach WRITE setDoubleClickToDetach);
	Q_PROPERTY(bool na_show_zero READ getNaShowZero WRITE setNaShowZero)
	Q_PROPERTY(bool spectrum_visible_peak_search READ getSpectrumVisiblePeakSearch WRITE setSpectrumVisiblePeakSearch)
	Q_PROPERTY(bool advanced_device_info READ getAdvancedDeviceInfo WRITE setAdvancedDeviceInfo)
	Q_PROPERTY(bool user_notes_active READ getUserNotesActive WRITE setUserNotesActive)
	Q_PROPERTY(bool graticule_enabled READ getGraticuleEnabled WRITE setGraticuleEnabled)
	Q_PROPERTY(bool external_script_enabled READ getExternalScript WRITE setExternalScript)
	Q_PROPERTY(bool manual_calib_script_enabled READ getManualCalibScript WRITE setManualCalibScript)
	Q_PROPERTY(bool animations_enabled READ getAnimationsEnabled WRITE setAnimationsEnabled)
	Q_PROPERTY(bool osc_filtering_enabled READ getOscFilteringEnabled WRITE setOscFilteringEnabled)
	Q_PROPERTY(bool mini_hist_enabled READ getMiniHist WRITE setMiniHist)
	Q_PROPERTY(bool digital_decoders READ getDigitalDecoders WRITE setDigitalDecoders)
	Q_PROPERTY(bool show_ADC_digital_filters READ getShowADCDigitalFilters WRITE setShowADCDigitalFilters)
	Q_PROPERTY(QString language READ getLanguage WRITE setLanguage);
	Q_PROPERTY(bool displaySamplingPoints READ getDisplaySampling WRITE setDisplaySampling)
	Q_PROPERTY(bool instrument_notes_active READ getInstrumentNotesActive WRITE setInstrumentNotesActive)


public:

	explicit Preferences_API(Preferences *preferencePanel) :
		ApiObject(),
		preferencePanel(preferencePanel) {}

	bool getAnimationsEnabled() const;
	void setAnimationsEnabled(const bool& enabled);

	bool getOscLabelsEnabled() const;
	void setOscLabelsEnabled(const bool& enabled);

	int getSigGenNrPeriods() const;
	void setSigGenNrPeriods(const int& periods);

	bool getSaveSession() const;
	void setSaveSession(const bool& enabled);

	bool getDoubleClickToDetach() const;
	void setDoubleClickToDetach(const bool& enabled);

	bool getNaShowZero() const;
	void setNaShowZero(const bool& enabled);

	bool getSpectrumVisiblePeakSearch() const;
	void setSpectrumVisiblePeakSearch(const bool& enabled);

	bool getAdvancedDeviceInfo() const;
	void setAdvancedDeviceInfo(const bool& enabled);

	bool getUserNotesActive() const;
	void setUserNotesActive(const bool& enabled);

	bool getGraticuleEnabled() const;
	void setGraticuleEnabled(const bool& enabled);

	bool getExternalScript() const;
	void setExternalScript(const bool& enabled);

	bool getManualCalibScript() const;
	void setManualCalibScript(const bool& enabled);

	bool getOscFilteringEnabled() const;
	void setOscFilteringEnabled(const bool& enabled);

	bool getShowADCDigitalFilters() const;
	void setShowADCDigitalFilters(const bool& enabled);

	bool getMiniHist() const;
	void setMiniHist(const bool& enabled);

	bool getDigitalDecoders() const;
	void setDigitalDecoders(bool enabled);

	QString getLanguage() const;
	void setLanguage(QString lang);

	bool getDisplaySampling() const;
	void setDisplaySampling(bool display);


	bool getInstrumentNotesActive() const;
	void setInstrumentNotesActive(bool display);

private:
	Preferences *preferencePanel;

};
}

#endif // PREFERENCE_PANEL_H
