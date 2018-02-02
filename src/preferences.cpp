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
	pref_api(new Preferences_API(this))
{
	ui->setupUi(this);

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
