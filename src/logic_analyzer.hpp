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

/* Local includes */
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

using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;

namespace pv {
namespace view {
class Viewport;
}
class Session;
}

namespace Glibmm {
}

namespace Ui {
class LogicAnalyzer;
class Channel;
class LChannelSettings;
}

namespace adiscope {
class LogicAnalyzer : public QWidget
{
	Q_OBJECT

public:
	explicit LogicAnalyzer(struct iio_context *ctx,
	                       Filter *filt,
	                       QPushButton *runButton,
	                       QWidget *parent = 0);
	~LogicAnalyzer();
	void updateAreaTimeTriggerPadding();

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

public Q_SLOTS:
	void onTimeTriggerHandlePosChanged(int);

private:
	Ui::LogicAnalyzer *ui;
	QButtonGroup *settings_group;
	QPushButton *active_settings_btn;
	QPushButton *menuRunButton;

	ScaleSpinButton *timeBase;
	PositionSpinButton *timePosition;

	const std::string& dev_name;

	struct iio_context *ctx;
	struct iio_device *dev;
	unsigned int no_channels;
	unsigned int itemsize;
	pv::MainWindow *main_win;

	void disconnectAll();
	static unsigned int get_no_channels(struct iio_device *dev);

	static const unsigned long maxBuffersize;
	static const unsigned long maxSampleRate;
	double active_sampleRate;
	unsigned long active_sampleCount;
	unsigned long custom_sampleCount;
	double pickSampleRateFor(double timeSpanSecs,
		double desiredBufferSize);

	void settings_panel_update(int id);
	void toggleRightMenu(QPushButton *btn);

	bool menuOpened;

	std::map<std::string, Glib::VariantBase> options;
	std::shared_ptr<pv::devices::BinaryStream> logic_analyzer_ptr;

	LogicAnalyzerChannelManager chm;
	LogicAnalyzerChannelManagerUI *chm_ui;
	Ui::LChannelSettings *lachannelsettings;

	void clearLayout(QLayout *layout);

	HorizHandlesArea *d_bottomHandlesArea;
	FreePlotLineHandleH *d_timeTriggerHandle;
	QWidget* bottomHandlesArea();

	MetricPrefixFormatter d_cursorMetricFormatter;
	TimePrefixFormatter d_cursorTimeFormatter;

	void updateBuffersizeSamplerateLabel(int samples, double samplerate);
	void setBuffersizeLabelValue(double value);
	void setSamplerateLabelValue(double value);
	int d_bufferSizeLabelVal;
	double d_sampleRateLabelVal;

	bool running = false;
	void setSampleRate();
	double plotRefreshRate;
	int timespanLimitStream;
};
}

#endif // LOGIC_ANALYZER_H
