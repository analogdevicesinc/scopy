/*
 * Copyright 2016 Analog Devices, Inc.
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

#ifndef M2K_TOOL_LAUNCHER_H
#define M2K_TOOL_LAUNCHER_H

#include <QFuture>
#include <QFutureWatcher>
#include <QJSEngine>
#include <QMainWindow>
#include <QPair>
#include <QSocketNotifier>
#include <QVector>
#include <QButtonGroup>
#include <QMap>
#include <QStringList>
#include <info_widget.h>
#include <QTextBrowser>

#include "apiObject.hpp"
#include "dmm.hpp"
#include "filter.hpp"
#include "calibration.hpp"
#include "oscilloscope.hpp"
#include "pulseview/pv/widgets/popup.hpp"
#include "power_controller.hpp"
#include "signal_generator.hpp"
#include "logic_analyzer.hpp"
#include "pattern_generator.hpp"
#include "network_analyzer.hpp"
#include "digitalio.hpp"
#include "detachedWindow.hpp"
#include "preferences.h"
#include "info_page.hpp"
#include "device_widget.hpp"
#include "connectDialog.hpp"
#include "toolmenu.h"

extern "C" {
	struct iio_context;
}

namespace Ui {
class ToolLauncher;
class Device;
class GenericAdc;
class GenericDac;
}

namespace adiscope {
class ToolLauncher_API;
class SpectrumAnalyzer;
class Debugger;
class ManualCalibration;
class UserNotes;

class ToolLauncher : public QMainWindow
{
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit ToolLauncher(QString prevCrashDump = "", QWidget *parent = 0);
	~ToolLauncher();

	Q_INVOKABLE void runProgram(const QString& program, const QString& fn);
	InfoWidget *infoWidget;

	Preferences *getPrefPanel() const;
	Calibration *getCalibration() const;
	bool eventFilter(QObject *watched, QEvent *event);

	bool getUseDecoders() const;
	void setUseDecoders(bool use_decoders);

Q_SIGNALS:
	void connectionDone(bool success);
	void adcCalibrationDone();
	void dacCalibrationDone();
	void calibrationDone();
	void calibrationFailed();

	void adcToolsCreated();
	void dacToolsCreated();

	void launcherClosed();

public Q_SLOTS:
	void addDebugWindow(void);

	void loadSession();
	void saveSession();
	void requestCalibration();
	void requestCalibrationCancel();

	void toolDetached(bool detached);

private Q_SLOTS:
	void search();
	void update();
	void ping();

	void btnOscilloscope_clicked();
	void btnSignalGenerator_clicked();
	void btnDMM_clicked();
	void btnPowerControl_clicked();
	void btnLogicAnalyzer_clicked();
	void btnPatternGenerator_clicked();
	void btnNetworkAnalyzer_clicked();
	void btnSpectrumAnalyzer_clicked();
	void btnDebugger_clicked();
	void btnCalibration_clicked();
	void on_btnHome_clicked();
	void setButtonBackground(bool on);

	void connectBtn_clicked(bool);
	void deviceBtn_clicked(bool);
	void btnAdd_toggled(bool);
	void btnHomepage_toggled(bool);
	void forgetDeviceBtn_clicked(QString);
	void destroyPopup();

	void enableAdcBasedTools();
	void enableDacBasedTools();

	void hasText();

	void btnDigitalIO_clicked();

	void resetSession();
	DeviceWidget* getConnectedDevice();
	DeviceWidget* getSelectedDevice();
	int getDeviceIndex(DeviceWidget*);
	void pageMoved(int);
	void stopSearching(bool);
	void _toolSelected(tool tool);
	void restartToolsAfterCalibration();
	void calibrationFailedCallback();
private:
	QList<Tool*> calibration_saved_tools;
	void loadToolTips(bool connected);
	QVector<QString> searchDevices();
	void swapMenu(QWidget *menu);
	void destroyContext();
	bool loadDecoders(QString path);
	bool switchContext(const QString& uri);
	void resetStylesheets();
	void initialCalibration();
	bool calibrate();
	void checkIp(const QString& ip);
	void disconnect();
	void saveSettings();
	Q_INVOKABLE QPushButton *addContext(const QString& hostname);

	void updateListOfDevices(const QVector<QString>& uris);
	QStringList tools;
	QStringList toolIcons;
	void closeEvent(QCloseEvent *event);
	void highlightDevice(QPushButton *btn);
	void setupHomepage();
	void updateHomepage();
	void readPreferences();
	void loadIndexPageFromContent(QString fileLocation);
	DeviceWidget* getDevice(QString uri);
	void setupAddPage();
	void allowExternalScript(bool);

private:
	Ui::ToolLauncher *ui;
	struct iio_context *ctx;

	ToolMenu *menu;

	QVector<int> position;
	QVector<Debugger*> debugInstances;
	QVector<DetachedWindow *> debugWindows;

	std::vector<DeviceWidget *> devices;
	QVector<Tool*> toolList;

	QTimer *search_timer, *alive_timer;
	QFutureWatcher<QVector<QString>> watcher;
	QFuture<QVector<QString>> future;
	QFuture<void> calibration_thread;

	DMM *dmm;
	PowerController *power_control;
	SignalGenerator *signal_generator;
	Oscilloscope *oscilloscope;
	LogicAnalyzer *logic_analyzer;
	DigitalIO *dio;
	DIOManager *dioManager;
	PatternGenerator *pattern_generator;
	NetworkAnalyzer *network_analyzer;
	SpectrumAnalyzer *spectrum_analyzer;
	Debugger *debugger;
	ManualCalibration *manual_calibration;
	QWidget *current;
	QSettings *settings;
	Preferences *prefPanel;
	UserNotes *notesPanel;

	QButtonGroup adc_users_group;

	Calibration *calib;
	std::shared_ptr<GenericAdc> adc;
	QList<std::shared_ptr<GenericDac>> dacs;

	Filter *filter;
	ToolLauncher_API *tl_api;

	QJSEngine js_engine;
	QString js_cmd;
	QSocketNotifier notifier;
	QString previousIp;

	ConnectDialog *connectWidget;
	QTextBrowser *welcome;
	QTextBrowser *index;

	bool calibrating;
	bool skip_calibration;

	bool debugger_enabled;
	bool manual_calibration_enabled;

	QString indexFile;
	QString deviceInfo;
	QString pathToFile;

	QButtonGroup *devices_btn_group;

	DeviceWidget* selectedDev;
	bool m_use_decoders;

	void _setupToolMenu();
	void saveRunningToolsBeforeCalibration();
	void stopToolsBeforeCalibration();
};
}
#endif // M2K_TOOL_LAUNCHER_H
