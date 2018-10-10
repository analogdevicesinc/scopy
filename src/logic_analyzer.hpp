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

#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

/* Qt includes */
#include <QWidget>
#include <QQueue>
#include <QPair>

/* Local includes */
#include "apiObject.hpp"
#include "pulseview/pv/widgets/sweeptimingwidget.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/devices/binarystream.hpp"
#include "la_channel_manager.hpp"
#include "la_capture_params.hpp"
#include "plot_utils.hpp"
#include "tool.hpp"
#include "customPushButton.hpp"

using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;

namespace pv {
namespace view {
class Viewport;
class Ruler;
}
class Session;
}

namespace Ui {
class LogicAnalyzer;
class Channel;
class LChannelSettings;
class DigitalTriggerSettings;
}

class QJSEngine;
class QPushButton;
class QTimer;

class HorizHandlesArea;
class FreePlotLineHandleH;
class PlotLineHandleH;

namespace adiscope {
class LogicAnalyzer_API;
class ChannelGroup_API;
class LogicChannel_API;
class MouseWheelWidgetGuard;
class DigitalBufferPreviewer;
class Filter;
class ScaleSpinButton;
class PositionSpinButton;
class StateUpdater;
class ExportSettings;

class LogicAnalyzer : public Tool
{
	friend class LogicAnalyzer_API;
	friend class ToolLauncher_API;
	friend class ChannelGroup_API;
	friend class LogicChannel_API;

	Q_OBJECT

public:
	enum AcquisitionMode {
                REPEATED,
                STREAM,
                SCREEN
	};

	enum TriggerState {
		Stop,
		Waiting,
		Triggered,
		Auto,
		Stream,
	};

	explicit LogicAnalyzer(struct iio_context *ctx,
	                       Filter *filt,
	                       QPushButton *runButton,
			       QJSEngine *engine,
	                       ToolLauncher *parent,
	                       bool offline_mode_ = 0);
	~LogicAnalyzer();
	void updateAreaTimeTrigger();

	void setHWTrigger(int chid, std::string trigger_val);
	std::string get_trigger_from_device(int chid);
	void startTimeout();
	void stopTimeout();
	void setTriggerCache(int chid, const std::string &trigger_value);
	void get_channel_groups_api();
	void installWheelEventGuard();
	void bufferSentSignal(bool lastBuffer);
	int getCurrent_acquisition_mode() const;
	void setCurrent_acquisition_mode(int value);
	QString saveToFile();
	std::vector<std::string> get_iio_trigger_options();

private Q_SLOTS:
	void startStop(bool start);
	void toggleRightMenu(bool);
	void rightMenuFinished(bool opened);
	void toggleLeftMenu(bool val);
	void leftMenuFinished(bool opened);
	void on_btnShowChannelsClicked(bool check);
	void onHorizScaleValueChanged(double value);
	void setTimebaseLabel(double value);
	void singleRun(bool);
	void onRulerChanged(double, bool);
	void setHWTriggerLogic(const QString value);
	void setupTriggerSettingsUI(bool enabled = false);
	void cleanTrigger();
	void setExternalTrigger(int);
	void cursorValueChanged_1(int);
	void cursorValueChanged_2(int);
	void setCursorsActive(bool);
	void resizeEvent();
	void resetInstrumentToDefault();
	void setTimeout(bool);
	void triggerTimeout();
	void startTimer();
	void stopTimer();
	void btnExportPressed();
	void runModeChanged(bool);
	void validateSamplingFrequency(double);
	void setTriggerState(int);
	void onDataReceived();
	void onFrameEnded();
	void onTriggerModeChanged(bool);
	void checkEnabledChannels();
	void toolDetached(bool);
public Q_SLOTS:
	void onTimeTriggerHandlePosChanged(int);
	void onTimePositionSpinboxChanged(double value);
	void refreshTriggerPos(int);
	void onChmWidthChanged(int);
	void triggerChanged(int);
	void updateBufferPreviewer();
	void requestUpdateBufferPreviewer();
Q_SIGNALS:
	void starttimeout();
	void stoptimeout();
	void activateExportButton();
	void showTool();

private:
	std::shared_ptr<sigrok::Context> context;
	pv::DeviceManager *device_manager;
	Ui::LogicAnalyzer *ui;
	Ui::DigitalTriggerSettings *trigger_settings_ui;
	QButtonGroup *settings_group;
	QPushButton *active_settings_btn;
	QPushButton *triggerBtn;
	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

	bool reset_horiz_offset;
	double horiz_offset_after_drop;
	double scrolling_offset;
	double trigger_offset;

	static std::vector<std::string> trigger_mapping;
	std::vector<std::string> trigger_cache;

	ScaleSpinButton *timeBase;
	PositionSpinButton *timePosition;
	ScaleSpinButton *frequencySpinButton;

	std::string dev_name;

	struct iio_device *dev;
	unsigned int no_channels;
	unsigned int itemsize;
	pv::MainWindow *main_win;

	QList<ChannelGroup_API *> channel_groups_api;
	LogicAnalyzerChannelManager chm;
	LogicAnalyzerChannelManagerUI *chm_ui;
	bool apiLoading;

	void disconnectAll();
	static unsigned int get_no_channels(struct iio_device *dev);

	static const unsigned long maxBuffersize;
	long long maxSamplingFrequency;
	void configureMaxSampleRate();
	static const unsigned long maxTriggerBufferSize;
	double active_sampleRate;
	double active_hw_sampleRate;
	unsigned long active_sampleCount;
	unsigned long last_set_sample_count;
	unsigned long custom_sampleCount;
	long long active_triggerSampleCount;
	long long active_hw_trigger_sample_count;
        double active_timePos;

	void settings_panel_update(int id);
	void toggleRightMenu(QPushButton *btn, bool checked);

	bool menuOpened;

	std::map<std::string, Glib::VariantBase> options;
	std::shared_ptr<pv::devices::BinaryStream> logic_analyzer_ptr;

	Ui::LChannelSettings *lachannelsettings;

	void clearLayout(QLayout *layout);

	HorizHandlesArea *d_bottomHandlesArea;
	FreePlotLineHandleH *d_timeTriggerHandle;
	QWidget* bottomHandlesArea();

	MetricPrefixFormatter d_cursorMetricFormatter;
	TimePrefixFormatter d_cursorTimeFormatter;

	PlotLineHandleH *d_hCursorHandle1;
	PlotLineHandleH *d_hCursorHandle2;

	void updateBuffersizeSamplerateLabel(int samples, double samplerate);
	void setBuffersizeLabelValue(double value);
	void setSamplerateLabelValue(double value);
	int d_bufferSizeLabelVal;
	double d_sampleRateLabelVal;

	bool running = false;
	void setSampleRate();
	void setTriggerDelay(bool silent = false);
	void setHWTriggerDelay(long long delay);
	double plotRefreshRate;
	int timespanLimitStream;

	double pixelToTime(int pix);
	int timeToPixel(double time);

	std::shared_ptr<LogicAnalyzerSymmetricBufferMode> symmBufferMode;
	QWidget* trigger_settings;
	double active_plot_timebase;

	double value_cursor1;
	double value_cursor2;
	double value_cursors_delta;
	void cleanHWParams();
	void cursorsFormatDelta();

	void recomputeCursorsValue(bool zoom);
	bool initialised;
	QTimer *timer;
	int timer_timeout_ms;
	std::atomic<bool> armed;
	void autoCaptureEnable(bool);

	DigitalBufferPreviewer *buffer_previewer;
	void set_buffersize();
	MouseWheelWidgetGuard *wheelEventGuard;
	bool offline_mode;
	int acquisition_mode;
	bool zoomed_in;
	int min_detached_width;

	void configParams(double timebase, double timepos);
	StateUpdater *triggerUpdater;
	bool trigger_is_forced;
	ExportSettings *exportSettings;
	QMap<int, bool> exportConfig;
	void init_export_settings();
	bool exportTabCsv(QString separator, QString);
	bool exportVCD(QString, QString, QString);
	void init_buffer_scrolling();
	void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
};
}
Q_DECLARE_METATYPE(std::shared_ptr<sigrok::OutputFormat>);
#endif // LOGIC_ANALYZER_H
