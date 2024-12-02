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
//#include "coloreditor.h"
#include "scopy_color_editor.h"


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

	bool getDebugger_enabled() const;
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

	bool getSeparateAnnotations() const;
	void setSeparateAnnotations(bool display);

	bool getTableInfo() const;
	void setTableInfo(bool display);

	bool getInstrumentNotesActive() const;
	void setInstrumentNotesActive(bool display);

	bool getDebugMessagesActive() const;
	void setDebugMessagesActive(bool display);

	bool getAttemptTempLutCalib() const;
	void setAttemptTempLutCalib(bool display);

	bool getSkipCalIfCalibrated() const;
	void setSkipCalIfCalibrated(bool val);

	bool getAutomatical_version_checking_enabled() const;
	void setAutomatical_version_checking_enabled(bool value);

	QString getCheck_updates_url() const;
	void setCheck_update_url(const QString &link);

	bool getFirst_application_run() const;
	void setFirst_application_run(bool value);

	void requestRestart();
	void setColorEditor(ScopyColorEditor *colorEditor);

	bool getLogging_enabled() const;
	void setLogging_enabled(bool value);

	bool getShow_plot_fps() const;
	void setShow_plot_fps(bool newShow_plot_fps);

	bool getUse_open_gl() const;
	void setUse_open_gl(bool newUse_open_gl);

	double getTarget_fps() const;
	void setTarget_fps(double newTarget_fps);

	bool getDocking_enabled() const;
	void setDocking_enabled(bool value);
	bool getCurrent_docking_enabled() const;

Q_SIGNALS:

	void notify();
	void reset();
	void requestUpdateCheck();

public Q_SLOTS:
	
	void save();
	QString loadLanguage();


private Q_SLOTS:

	void resetScopy();

private:
	void forceSavePreferences();

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
	bool automatical_version_checking_enabled;
	QString check_updates_url;
	bool first_application_run;
	bool m_initialized;
	bool m_useNativeDialogs;
	QString language;
	bool m_instrument_notes_active;
	bool m_displaySamplingPoints;
	bool m_separateAnnotations;
	bool m_tableInfo;
	bool m_debug_messages_active;
	bool m_attemptTempLutCalib;
	bool m_skipCalIfCalibrated;
	bool m_logging_enabled;
	bool m_show_plot_fps;
	bool m_use_open_gl;
	int m_target_fps;
	bool m_docking_enabled;
	bool m_current_docking_state;

	Preferences_API *pref_api;

	ScopyColorEditor *m_colorEditor;
	void initializePreferences();
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
	Q_PROPERTY(bool debug_messages_active READ getDebugMessagesActive WRITE setDebugMessagesActive)
	Q_PROPERTY(bool attemptTempLutCalib READ getAttemptTempLutCalib WRITE setAttemptTempLutCalib)
	Q_PROPERTY(bool skipCalIfCalibrated READ getSkipCalIfCalibrated WRITE setSkipCalIfCalibrated)
	Q_PROPERTY(bool separateAnnotations READ getSeparateAnnotations WRITE setSeparateAnnotations)
	Q_PROPERTY(bool TableInfo READ getTableInfo WRITE setTableInfo)
	Q_PROPERTY(bool automatical_version_checking_enabled READ getAutomaticalVersionCheckingEnabled WRITE setAutomaticalVersionCheckingEnabled)
	Q_PROPERTY(QString check_updates_url READ getCheckUpdatesUrl WRITE setCheckUpdatesUrl)
	Q_PROPERTY(bool first_application_run READ getFirstApplicationRun WRITE setFirstApplicationRun)
	Q_PROPERTY(QString currentStylesheet READ getCurrentStylesheet WRITE setCurrentStylesheet)
	Q_PROPERTY(QStringList userStylesheets READ getUserStylesheets WRITE setUserStylesheets)
	Q_PROPERTY(bool showPlotFps READ getShowPlotFps WRITE setShowPlotFps)
	Q_PROPERTY(bool useOpenGl READ getUseOpenGl WRITE setUseOpenGl)
	Q_PROPERTY(double targetFps READ getTargetFps WRITE setTargetFps)
	Q_PROPERTY(bool docking_enabled READ getDockingEnabled WRITE setDockingEnabled)


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

	bool getDebugMessagesActive() const;
	void setDebugMessagesActive(bool display);

	bool getAttemptTempLutCalib() const;
	void setAttemptTempLutCalib(bool val);

	bool getSkipCalIfCalibrated() const;
	void setSkipCalIfCalibrated(bool val);

	bool getSeparateAnnotations() const;
	void setSeparateAnnotations(bool val);

	bool getTableInfo() const;
	void setTableInfo(bool val);

	bool getAutomaticalVersionCheckingEnabled() const;
	void setAutomaticalVersionCheckingEnabled(const bool& enabled);

	QString getCheckUpdatesUrl() const;
	void setCheckUpdatesUrl(const QString& link);

	bool getFirstApplicationRun() const;
	void setFirstApplicationRun(const bool& first);

	bool getShowPlotFps() const;
	void setShowPlotFps(const bool& first);

	bool getUseOpenGl() const;
	void setUseOpenGl(const bool& first);

	double getTargetFps() const;
	void setTargetFps(const double& val);

	QString getCurrentStylesheet() const;
	void setCurrentStylesheet(const QString &currentStylesheet);

	QStringList getUserStylesheets() const;
	void setUserStylesheets(const QStringList &userStylesheets);

	bool getDockingEnabled() const;
	void setDockingEnabled(const bool& first);

private:
	Preferences *preferencePanel;

};

extern Preferences* pref_ptr;
Preferences* getScopyPreferences();

}

#endif // PREFERENCE_PANEL_H
