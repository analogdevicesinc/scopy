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
#include "menuoption.h"
#include "detachedWindow.hpp"

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

class ToolLauncher : public QMainWindow
{
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit ToolLauncher(QWidget *parent = 0);
	~ToolLauncher();

	Q_INVOKABLE void runProgram(const QString& program, const QString& fn);
	InfoWidget *infoWidget;

Q_SIGNALS:
	void connectionDone(bool success);
	void adcCalibrationDone();
	void dacCalibrationDone();

	void adcToolsCreated();
	void dacToolsCreated();

public Q_SLOTS:
	void detachToolOnPosition(int);
	void addDebugWindow(void);

	void loadSession();
	void saveSession();

private Q_SLOTS:

	void search();
	void update();
	void ping();
	void highlightLast(bool on);

	void btnOscilloscope_clicked();
	void btnSignalGenerator_clicked();
	void btnDMM_clicked();
	void btnPowerControl_clicked();
	void btnLogicAnalyzer_clicked();
	void btnPatternGenerator_clicked();
	void btnNetworkAnalyzer_clicked();
	void btnSpectrumAnalyzer_clicked();
	void btnDebugger_clicked();
	void on_btnHome_clicked();
	void setButtonBackground(bool on);

	void on_btnConnect_clicked(bool pressed);

	void device_btn_clicked(bool pressed);
	void addRemoteContext();
	void destroyPopup();

	void enableAdcBasedTools();
	void enableDacBasedTools();

	void hasText();

	void btnDigitalIO_clicked();

	void toolDetached(bool detached);

	void swapMenuOptions(int source, int destination, bool dropAfter);
	void highlight(bool on, int position);

private:
	Ui::ToolLauncher *ui;
	struct iio_context *ctx;

	QMap<QString, MenuOption*> toolMenu;
	QList<DetachedWindow *> detachedWindows;
	QVector<int> position;
	QVector<Debugger*> debugInstances;
	QVector<DetachedWindow *> debugWindows;

	QVector<QPair<QWidget, Ui::Device> *> devices;
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
	QWidget *current;
	QSettings *settings;

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

	bool calibrating;
	bool skip_calibration;

	bool debugger_enabled;

	QString indexFile;

	void loadToolTips(bool connected);
	QVector<QString> searchDevices();
	void swapMenu(QWidget *menu);
	void destroyContext();
	bool loadDecoders(QString path);
	bool switchContext(const QString& uri);
	void resetStylesheets();
	void calibrate();
	void checkIp(const QString& ip);
	void disconnect();
	void saveSettings();
	Q_INVOKABLE QPushButton *addContext(const QString& hostname);

	QList<QString> getOrder();
	void setOrder(QList<QString> list);

	QTextBrowser *welcome;
	QTextBrowser *index;

	void updateListOfDevices(const QVector<QString>& uris);
	void generateMenu();
	QStringList tools;
	QStringList toolIcons;
	void UpdatePosition(QWidget *widget, int position);
	void insertMenuOptions();
	void closeEvent(QCloseEvent *event);
	void highlightDevice(QPushButton *btn);
	void setupHomepage();
};

class ToolLauncher_API: public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool menu_opened READ menu_opened WRITE open_menu
		   STORED false);

	Q_PROPERTY(bool hidden READ hidden WRITE hide STORED false);

	Q_PROPERTY(QString previous_ip READ getPreviousIp WRITE addIp
		   SCRIPTABLE false);

	Q_PROPERTY(bool maximized READ maximized WRITE setMaximized);

	Q_PROPERTY(QList<QString> tool_list READ order WRITE setOrder);

	Q_PROPERTY(bool skip_calibration READ calibration_skipped WRITE skip_calibration);

	Q_PROPERTY(bool debugger READ debugger_enabled WRITE enable_debugger)

	Q_PROPERTY(QString index_file READ getIndexFile WRITE setIndexFile)

public:
	explicit ToolLauncher_API(ToolLauncher *tl) : ApiObject(), tl(tl) {}
	~ToolLauncher_API() {}

	QString getIndexFile() const;
	void setIndexFile(const QString &indexFile);

	bool menu_opened() const;
	void open_menu(bool open);

	QList<QString> order();
	void setOrder(QList<QString> list);

	bool hidden() const;
	void hide(bool hide);

	bool calibration_skipped();
	void skip_calibration(bool);

	bool debugger_enabled();
	void enable_debugger(bool);

	const QString& getPreviousIp()
	{
		return tl->previousIp;
	}
	void addIp(const QString& ip);

	bool maximized()
	{
		return tl->isMaximized();
	}
	void setMaximized(bool m)
	{
		if (m) {
			tl->showMaximized();
		} else {
			tl->showNormal();
		}
	}

	Q_INVOKABLE QList<QString> usb_uri_list();
	Q_INVOKABLE bool connect(const QString& uri);
	Q_INVOKABLE void disconnect();

	Q_INVOKABLE void load(const QString& file);
	Q_INVOKABLE void save(const QString& file);

private:
	ToolLauncher *tl;
};
}

#endif // M2K_TOOL_LAUNCHER_H
