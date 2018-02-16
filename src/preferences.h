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

private:
	Preferences *preferencePanel;

};
}

#endif // PREFERENCE_PANEL_H
