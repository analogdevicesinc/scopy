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

Q_SIGNALS:

	void notify();
	void reset();

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

	Preferences_API *pref_api;
	QString getPreferenceIniFile() const;
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

public:

	explicit Preferences_API(Preferences *preferencePanel) :
		ApiObject(),
		preferencePanel(preferencePanel) {}

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

private:
	Preferences *preferencePanel;

};
}

#endif // PREFERENCE_PANEL_H
