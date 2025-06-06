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

#ifndef M2KPLUGIN_H
#define M2KPLUGIN_H

#include "m2ktool.hpp"
#define SCOPY_PLUGIN_NAME M2kPlugin

#include <QObject>
//#include <pluginbase/plugin.h>
#include "iio_manager.hpp"
#include "m2kcontroller.h"
#include "scopy-m2k_export.h"

#include <QLineEdit>
#include <QPushButton>

#include <calibration.hpp>
#include <gui/infopage.h>
#include <pluginbase/pluginbase.h>

namespace scopy {

namespace m2k {
class SCOPY_M2K_EXPORT M2kPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;

	void preload() override;
	void loadToolList() override;
	bool loadPage() override;
	bool loadExtraButtons() override;

	bool onConnect() override;
	bool onDisconnect() override;
	bool loadIcon() override;

	void initPreferences() override;
	bool loadPreferencesPage() override;
	void init() override;
	void saveSettings(QSettings &s) override;
	void loadSettings(QSettings &s) override;

	QString description() override;
	QString pkgName() override;

public Q_SLOTS:
	void showPageCallback() override;
	void hidePageCallback() override;
	void startPingTask() override;
	void stopPingTask() override;
	void onPausePingTask(bool pause) override;

	void calibrationStarted();
	void calibrationSuccess();
	void calibrationFinished();
	void updateTemperature(double);

private:
	QTimer *m_infoPageTimer;

	QPushButton *m_btnIdentify;
	QPushButton *m_btnCalibrate;
	QPushButton *m_btnRegister;
	QButtonGroup *m_adcBtnGrp;
	QButtonGroup *m_dacBtnGrp;

	Calibration *calib;

	M2kController *m_m2kController;
	m2k_iio_manager *m2k_man;
	InfoPage *m_m2kInfoPage;

	libm2k::context::M2k *m_m2k;

	const int PING_PERIOD = 5000;
	const int infoPageTimerTimeout = 1000;
	const QStringList calibrationToolNames = {"Oscilloscope",     "Spectrum Analyzer", "Network Analyzer",
						  "Signal Generator", "Voltmeter",	   "Calibration"};

	QMap<QString, M2kTool *> tools;
	QMap<QString, ToolMenuEntry *> toolMenuEntryCalibrationCache;
	void storeToolState(QStringList tools);
	void restoreToolState(QStringList tools);
	void cleanup();
	void clearPingTask();
};
} // namespace m2k
} // namespace scopy

#endif // M2KPLUGIN_H
