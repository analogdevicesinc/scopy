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
#include <QPushButton>
#include <QTimer>
#include <QQmlListProperty>

/* Local includes */
#include "apiObject.hpp"
#include "filter.hpp"
#include "pulseview/pv/widgets/sweeptimingwidget.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/devices/binarystream.hpp"
#include "la_channel_manager.hpp"
#include "handles_area.hpp"
#include "plot_line_handle.h"
#include "spinbox_a.hpp"
#include "la_capture_params.hpp"
#include "buffer_previewer.hpp"

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

namespace Glibmm {
}

namespace Ui {
class LogicAnalyzer;
class Channel;
class LChannelSettings;
class DigitalTriggerSettings;
}

class QJSEngine;

namespace adiscope {
class LogicAnalyzer_API;
class ChannelGroup_API;
class LogicChannel_API;
class MouseWheelWidgetGuard;

class LogicAnalyzer : public QWidget
{
	friend class LogicAnalyzer_API;
	friend class ToolLauncher_API;
	friend class ChannelGroup_API;
	friend class LogicChannel_API;

	Q_OBJECT

public:
	explicit LogicAnalyzer(struct iio_context *ctx,
	                       Filter *filt,
	                       QPushButton *runButton,
			       QJSEngine *engine,
	                       QWidget *parent = 0,
	                       unsigned int sample_rate = 200000);
	~LogicAnalyzer();
	void updateAreaTimeTrigger();

	void setHWTrigger(int chid, std::string trigger_val);
	std::string get_trigger_from_device(int chid);
	void set_triggered_status(std::string value);
	void refilling();
	void captured();
	void setTriggerCache(int chid, std::string trigger_value);
	void get_channel_groups_api();
	void installWheelEventGuard();

private Q_SLOTS:
	void startStop(bool start);
	void toggleRightMenu();
	void rightMenuFinished(bool opened);
	void toggleLeftMenu(bool val);
	void leftMenuFinished(bool opened);
	void on_btnShowChannelsClicked(bool check);
	void onHorizScaleValueChanged(double value);
	void setTimebaseLabel(double value);
	void singleRun();
	void onRulerChanged(double, bool);
	void setHWTriggerLogic(const QString value);
	void setupTriggerSettingsUI(bool enabled = false);
	void setExternalTrigger(int);
	void cursorValueChanged_1(int);
	void cursorValueChanged_2(int);
	void setCursorsActive(bool);
	void resizeEvent();
	void resetInstrumentToDefault();
	void setTimeout(bool);
	void triggerStateTimeout();
	void triggerTimeout();
	void startTimeout();
	void capturedSlot();
public Q_SLOTS:
	void onTimeTriggerHandlePosChanged(int);
	void onTimePositionSpinboxChanged(double value);
	void refreshTriggerPos(int);
	void onChmWidthChanged(int);
	void triggerChanged(int);
	void updateBufferPreviewer();
Q_SIGNALS:
	void startRefill();
	void capturedSignal();

private:
	std::shared_ptr<sigrok::Context> context;
	Ui::LogicAnalyzer *ui;
	Ui::DigitalTriggerSettings *trigger_settings_ui;
	QButtonGroup *settings_group;
	QPushButton *active_settings_btn;
	QPushButton *menuRunButton;
	QPushButton *triggerBtn;

	static std::vector<std::string> trigger_mapping;
	std::vector<std::string> trigger_cache;

	ScaleSpinButton *timeBase;
	PositionSpinButton *timePosition;

	const std::string& dev_name;

	struct iio_context *ctx;
	struct iio_device *dev;
	unsigned int no_channels;
	unsigned int itemsize;
	pv::MainWindow *main_win;

	LogicAnalyzer_API *la_api;
	QList<ChannelGroup_API *> channel_groups_api;
	LogicAnalyzerChannelManager chm;
	LogicAnalyzerChannelManagerUI *chm_ui;

	void disconnectAll();
	static unsigned int get_no_channels(struct iio_device *dev);

	static const unsigned long maxBuffersize;
	static const unsigned long maxSampleRate;
	static const unsigned long maxTriggerBufferSize;
	double active_sampleRate;
	double active_hw_sampleRate;
	unsigned long active_sampleCount;
	unsigned long last_set_sample_count;
	unsigned long custom_sampleCount;
	long long active_triggerSampleCount;
	long long active_hw_trigger_sample_count;
	double active_timePos;
	double pickSampleRateFor(double timeSpanSecs,
		double desiredBufferSize);

	void settings_panel_update(int id);
	void toggleRightMenu(QPushButton *btn);

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
	void enableTrigger(bool value);
	void cleanHWParams();
	void cursorsFormatDelta();

	void recomputeCursorsValue(bool zoom);
	bool initialised;
	QTimer *timer;
	int timer_timeout_ms;
	std::atomic<bool> armed;
	void autoCaptureEnable();
	QString trigger_state;
	QTimer *state_timer;

	DigitalBufferPreviewer *buffer_previewer;
	void set_buffersize();
	MouseWheelWidgetGuard *wheelEventGuard;
};

class LogicAnalyzer_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(int channel_groups_list_size READ channel_groups_list_size WRITE
		setChannelGroupsListSize SCRIPTABLE false)
	Q_PROPERTY(QVariantList channel_groups READ getChannelGroups)
	Q_PROPERTY(bool running READ running WRITE run STORED false)
	Q_PROPERTY(double time_position READ getTimePos WRITE setTimePos)
	Q_PROPERTY(double time_base READ getTimeBase WRITE setTimeBase)
	Q_PROPERTY(bool external_trigger READ externalTrigger WRITE setExternalTrigger)
	Q_PROPERTY(bool cursors_active READ cursorsActive WRITE setCursorsActive)
	Q_PROPERTY(bool cursors_locked READ cursorsLocked WRITE setCursorsLocked)
	Q_PROPERTY(bool inactive_hidden READ inactiveHidden WRITE setInactiveHidden)

public:
	explicit LogicAnalyzer_API(LogicAnalyzer *lga) :
		ApiObject(), lga(lga) {}
	~LogicAnalyzer_API() {}

	int channel_groups_list_size() const;
	void setChannelGroupsListSize(int size);

	QVariantList getChannelGroups();

	bool running() const;
	void run(bool en);

	double getTimePos() const;
	void setTimePos(double pos);

	double getTimeBase() const;
	void setTimeBase(double base);

	bool externalTrigger() const;
	void setExternalTrigger(bool val);

	bool cursorsActive() const;
	void setCursorsActive(bool en);

	bool cursorsLocked() const;
	void setCursorsLocked(bool en);

	bool inactiveHidden() const;
	void setInactiveHidden(bool en);

private:
	LogicAnalyzer *lga;
};

class ChannelGroup_API : public ApiObject
{
	friend class LogicChannel_API;
	Q_OBJECT
	Q_PROPERTY(QString decoder READ getDecoder WRITE setDecoder)
	Q_PROPERTY(int channels_list_size READ channels_list_size WRITE
		   setChannelsListSize SCRIPTABLE false)
	Q_PROPERTY(QVariantList channels READ getChannels)
	Q_PROPERTY(bool enabled READ chEnabled WRITE setChEnabled)
	Q_PROPERTY(bool grouped READ chGrouped WRITE setChGrouped)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(bool collapsed READ getChCollapsed WRITE setChCollapsed)

public:
	explicit ChannelGroup_API(LogicAnalyzer *lga, int index = -1,
				bool load = true) :
		ApiObject(), lga(lga), index(index)
		{
			if(!load)
				set_channels_api();
		}
	~ChannelGroup_API() {
		qDeleteAll(channels_api);
		channels_api.clear();
	}
	bool chEnabled() const;
	void setChEnabled(bool en);

	bool chGrouped() const;
	void setChGrouped(bool val);

	QString getName() const;
	void setName(QString val);

	bool getChCollapsed() const;
	void setChCollapsed(bool val);

	int channels_list_size() const;
	void setChannelsListSize(int size);

	QVariantList getChannels();

	int getIndex() const;
	void set_channels_api();

	QString getDecoder() const;
	void setDecoder(QString val);
private:
	LogicAnalyzer *lga;
	QList<LogicChannel_API *> channels_api;
	int index;
};

class LogicChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(int index READ getIndex WRITE setIndex)
	Q_PROPERTY(QString trigger READ getTrigger WRITE setTrigger)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(QString role READ getRole WRITE setRole)

public:
	explicit LogicChannel_API(LogicAnalyzer *lga,
			ChannelGroup_API *chg, int index=-1) :
		ApiObject(), lga(lga), lchg(chg), index(index)
		{
		}
	~LogicChannel_API() { }

	QString getTrigger() const;
	void setTrigger(QString val);

	QString getName() const;
	void setName(QString val);

	int getIndex() const;
	void setIndex(int);

	QString getRole() const;
	void setRole(QString val);
private:
	LogicAnalyzer *lga;
	ChannelGroup_API *lchg;
	int index;
};
}

#endif // LOGIC_ANALYZER_H
