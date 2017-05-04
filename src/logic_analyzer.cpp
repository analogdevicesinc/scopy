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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <iio.h>

/* GNU Radio includes */
#include <gnuradio/blocks/file_descriptor_sink.h>

/* Qt includes */
#include <QDebug>

/* Local includes */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/session.hpp"
#include "pulseview/pv/view/ruler.hpp"
#include "streams_to_short.h"
#include "logic_analyzer.hpp"
#include "spinbox_a.hpp"

/* Sigrok includes */
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

/* Generated UI */
#include "ui_logic_analyzer.h"
#include "ui_logic_channel_settings.h"
#include "ui_digital_trigger_settings.h"

/* Boost includes */
#include <boost/thread.hpp>
#include <QListView>

using namespace std;
using namespace adiscope;
using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;
using sigrok::ConfigKey;
using namespace Glibmm;

const unsigned long LogicAnalyzer::maxBuffersize = 16000;
const unsigned long LogicAnalyzer::maxSampleRate = 80000000;
const unsigned long LogicAnalyzer::maxTriggerBufferSize = 8192;

std::vector<std::string> LogicAnalyzer::trigger_mapping = {
		"none",
		"edge-any",
		"edge-rising",
		"edge-falling",
		"level-low",
		"level-high",
};

LogicAnalyzer::LogicAnalyzer(struct iio_context *ctx,
                             Filter *filt,
                             QPushButton *runBtn,
                             QJSEngine *engine,
                             QWidget *parent,
                             unsigned int sample_rate) :
	QWidget(parent),
	dev_name(filt->device_name(TOOL_LOGIC_ANALYZER)),
	ctx(ctx),
	itemsize(sizeof(uint16_t)),
	dev(iio_context_find_device(ctx, dev_name.c_str())),
	menuOpened(false),
	settings_group(new QButtonGroup(this)),
	menuRunButton(runBtn),
	ui(new Ui::LogicAnalyzer),
	active_settings_btn(nullptr),
	timespanLimitStream(11),
	plotRefreshRate(100),
	active_sampleRate(0.0),
	active_hw_sampleRate(0.0),
	active_sampleCount(0),
	active_triggerSampleCount(0),
	active_hw_trigger_sample_count(0),
	active_timePos(0),
	trigger_settings(new QWidget(this)),
	value_cursor1(-0.033),
	value_cursor2(0.033),
	la_api(new LogicAnalyzer_API(this)),
	initialised(false),
	timer(new QTimer(this)),
	armed(false),
	state_timer(new QTimer(this)),
	trigger_state("Stop")
{
	ui->setupUi(this);
	timer->setSingleShot(true);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	iio_context_set_timeout(ctx, UINT_MAX);

	symmBufferMode = make_shared<LogicAnalyzerSymmetricBufferMode>();
	symmBufferMode->setMaxSampleRate(80000000);
	symmBufferMode->setEntireBufferMaxSize(500000); // max 0.5 mega-samples
	symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
	symmBufferMode->setTimeDivisionCount(10);

	/* Buffer Previewer widget */
	buffer_previewer = new DigitalBufferPreviewer(40, this);

	buffer_previewer->setVerticalSpacing(6);
	buffer_previewer->setMinimumHeight(20);
	buffer_previewer->setMaximumHeight(20);
	buffer_previewer->setMinimumWidth(380);
	buffer_previewer->setMaximumWidth(380);
	buffer_previewer->setHighlightBgColor(QColor("#141416"));
	buffer_previewer->setHighlightFgColor(QColor("#ff7200"));
	buffer_previewer->setCursorColor(QColor("#4A64FF"));
	QString stylesheet = "adiscope--BufferPreviewer {"
			"background-color: #272730;"
			"color: #ffffff;"
			"border: 1px solid #7092be;"
		"}";
	buffer_previewer->setStyleSheet(stylesheet);
	ui->vLayoutBufferSlot->addWidget(buffer_previewer);

	buffer_previewer->setCursorPos(0.5);

	for(int i=0; i < get_no_channels(dev)+2; i++) {
		trigger_cache.push_back(trigger_mapping[0]);
	}

	/* Time position widget */
	this->d_bottomHandlesArea = new HorizHandlesArea(this);
	this->d_bottomHandlesArea->setMinimumHeight(50);
	this->d_bottomHandlesArea->setLeftPadding(20);
	this->d_bottomHandlesArea->setRightPadding(20);

	d_timeTriggerHandle = new FreePlotLineHandleH(
					QPixmap(":/icons/time_trigger_handle.svg"),
					QPixmap(":/icons/time_trigger_left.svg"),
					QPixmap(":/icons/time_trigger_right.svg"),
					d_bottomHandlesArea);

	d_timeTriggerHandle->setPen(QPen(QColor(74, 100, 255)));
	d_timeTriggerHandle->setInnerSpacing(0);

	connect(d_timeTriggerHandle, SIGNAL(positionChanged(int)),
		this, SLOT(onTimeTriggerHandlePosChanged(int)));

	d_cursorMetricFormatter.setTwoDecimalMode(false);
	d_cursorTimeFormatter.setTwoDecimalMode(false);

	/* Cursors */
	d_hCursorHandle1 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);
	d_hCursorHandle2 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);

	QPen cursorsLinePen = QPen(QColor(155, 155, 155), 1, Qt::DashLine);
	d_hCursorHandle1->setPen(cursorsLinePen);
	d_hCursorHandle2->setPen(cursorsLinePen);
	d_hCursorHandle1->setInnerSpacing(0);
	d_hCursorHandle2->setInnerSpacing(0);

	// Call to minimumSizeHint() is required. Otherwise font properties from
	// stylesheet will be ignored when calculating width using FontMetrics
	int width = ui->timebaseLabel->minimumSizeHint().width();
	QFontMetrics fm = ui->timebaseLabel->fontMetrics();
	width = fm.width("999.999 ms/div");
	ui->timebaseLabel->setMinimumWidth(width);

	this->settings_group->setExclusive(true);
	this->no_channels = get_no_channels(dev);

	/* sigrok and sigrokdecode initialisation */
	std::string open_file, open_file_format;
	context = sigrok::Context::create();

	pv::DeviceManager device_manager(context);
	pv::MainWindow *w = new pv::MainWindow(device_manager, filt, open_file,
	                                       open_file_format, parent);

	for (unsigned int j = 0; j < iio_device_get_channels_count(dev); j++) {
		struct iio_channel *chn = iio_device_get_channel(dev, j);

		if (!iio_channel_is_output(chn) &&
		    iio_channel_is_scan_element(chn)) {
			iio_channel_enable(chn);
		}
	}

	// Controls for scale/division and position
	timeBase = new ScaleSpinButton({
		{"ns", 1E-9},
		{"μs", 1E-6},
		{"ms", 1E-3},
		{"s", 1E0}
	}, "Time Base", 10e-9, 1e0);
	timePosition = new PositionSpinButton({
		{"ns", 1E-9},
		{"μs", 1E-6},
		{"ms", 1E-3},
		{"s", 1E0}
	}, "Position",
	-timeBase->maxValue() * 5,
	timeBase->maxValue() * 5);

	ui->generalSettingsLayout->insertWidget(ui->generalSettingsLayout->count() - 3,
		timeBase, 0, Qt::AlignLeft);
	ui->generalSettingsLayout->insertWidget(ui->generalSettingsLayout->count() - 2,
		timePosition, 0, Qt::AlignLeft);

	options["numchannels"] = Glib::Variant<gint32>(
			g_variant_new_int32(no_channels),true);
	logic_analyzer_ptr = std::make_shared<pv::devices::BinaryStream>(
	                            context, dev, maxBuffersize,
	                             w->get_format_from_string("binary"),
	                             options, this);

	/* setup view */
	main_win = w;
	main_win->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->centralWidgetLayout->addWidget(static_cast<QWidget * >(main_win));
	main_win->select_device(logic_analyzer_ptr);

	set_buffersize();
	main_win->session_.set_timespanLimit(timespanLimitStream);

	ui->rightWidget->setMaximumWidth(0);
	static_cast<QWidget *>(main_win->main_bar_)->setVisible(false);

	/* General settings */
	settings_group->addButton(ui->btnSettings);
	int settings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnSettings->setProperty("id", QVariant(-settings_panel));

	/* Channel settings */
	settings_group->addButton(ui->btnChSettings);
	int ch_settings_panel = ui->stackedWidget->indexOf(ui->colorSettings);
	ui->btnChSettings->setProperty("id", QVariant(-ch_settings_panel));

	/* Trigger Settings */
	triggerBtn = ui->btnTrigger;
	settings_group->addButton(triggerBtn);
	int trigger_panel = ui->stackedWidget->indexOf(ui->triggerSettings);
	ui->btnTrigger->setProperty("id", QVariant(-trigger_panel));

	trigger_settings_ui = new Ui::DigitalTriggerSettings();
	trigger_settings_ui->setupUi(trigger_settings);
	ui->triggerSettingsLayout->insertWidget(0, trigger_settings);
	setupTriggerSettingsUI();
	QAbstractItemView *trigVw = trigger_settings_ui->cmb_trigg_extern_cond_1->view();
	QListView* listVw = qobject_cast<QListView*>(trigVw);
	listVw->setSpacing(2);
	trigVw = trigger_settings_ui->cmb_trigg_extern_cond_2->view();
	listVw = qobject_cast<QListView*>(trigVw);
	listVw->setSpacing(2);

	/* Cursor Settings */
	settings_group->addButton(ui->btnCursors);
	int cursors_panel =  ui->stackedWidget->indexOf(ui->cursorSettings);
	ui->btnCursors->setProperty("id", QVariant(-cursors_panel));

	connect(trigger_settings_ui->cmb_trigg_extern_cond_1,
		SIGNAL(currentIndexChanged(int)),
		this, SLOT(setExternalTrigger(int)));
	connect(trigger_settings_ui->cmb_trigg_extern_cond_2,
		SIGNAL(currentIndexChanged(int)),
		this, SLOT(setExternalTrigger(int)));
	connect(trigger_settings_ui->trigg_extern_en, SIGNAL(toggled(bool)),
		this, SLOT(setupTriggerSettingsUI(bool)));
	connect(trigger_settings_ui->cmb_trigg_logic, SIGNAL(currentTextChanged(const QString)),
		this, SLOT(setHWTriggerLogic(const QString)));
	connect(ui->btnTrigger, SIGNAL(pressed()),
		this, SLOT(toggleRightMenu()));
	connect(ui->btnCursors, SIGNAL(pressed()),
		this, SLOT(toggleRightMenu()));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)),
	        this, SLOT(startStop(bool)));
	connect(ui->btnSingleRun, SIGNAL(pressed()),
	        this, SLOT(singleRun()));
	connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop,
	        SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn,
	        SLOT(setChecked(bool)));
	connect(ui->btnSettings, SIGNAL(pressed()),
	        this, SLOT(toggleRightMenu()));
	connect(ui->btnChSettings, SIGNAL(pressed()),
	        this, SLOT(toggleRightMenu()));
	connect(ui->rightWidget, SIGNAL(finished(bool)),
	        this, SLOT(rightMenuFinished(bool)));
	connect(ui->btnShowHideMenu, SIGNAL(clicked(bool)),
	        this, SLOT(toggleLeftMenu(bool)));
	connect(timeBase, SIGNAL(valueChanged(double)),
	        main_win->view_, SLOT(set_timebase(double)));
	connect(timeBase, SIGNAL(valueChanged(double)),
		this, SLOT(onHorizScaleValueChanged(double)));
	connect(timeBase, SIGNAL(valueChanged(double)),
		this, SLOT(setTimebaseLabel(double)));
	connect(timePosition, SIGNAL(valueChanged(double)),
		this, SLOT(onTimePositionSpinboxChanged(double)));
	connect(main_win->view_, SIGNAL(repaintTriggerHandle(double, bool)),
		this, SLOT(onRulerChanged(double, bool)));
	connect(main_win->view_->viewport(), SIGNAL(repaintTriggerHandle(int)),
		this, SLOT(refreshTriggerPos(int)));
	connect(d_hCursorHandle1, SIGNAL(positionChanged(int)),
		this, SLOT(cursorValueChanged_1(int)));
	connect(d_hCursorHandle2, SIGNAL(positionChanged(int)),
		this, SLOT(cursorValueChanged_2(int)));
	connect(ui->boxCursors, SIGNAL(toggled(bool)),
		this, SLOT(setCursorsActive(bool)));
	connect(main_win->view_, SIGNAL(resized()),
		this, SLOT(resizeEvent()));
	connect(ui->btnResetInstrument, SIGNAL(clicked(bool)),
		this, SLOT(resetInstrumentToDefault()));
	connect(trigger_settings_ui->btnAuto, SIGNAL(toggled(bool)),
		this, SLOT(setTimeout(bool)));
	connect(this, SIGNAL(startRefill()),
		this, SLOT(startTimeout()));
	connect(this, SIGNAL(capturedSignal()),
		this, SLOT(capturedSlot()));
	connect(timer, &QTimer::timeout,
		this, &LogicAnalyzer::triggerTimeout);
	connect(state_timer, &QTimer::timeout,
		this, &LogicAnalyzer::triggerStateTimeout);
	connect(main_win->view_, SIGNAL(data_received()),
		this, SLOT(updateBufferPreviewer()));

	cleanHWParams();
	chm_ui = new LogicAnalyzerChannelManagerUI(0, main_win, &chm, ui->scrollAreaWidgetContents,
			ui->scrollAreaLayout, this);
	ui->leftLayout->addWidget(chm_ui);
	chm_ui->update_ui();
	chm_ui->setVisible(true);

	connect(ui->btnGroupChannels, SIGNAL(pressed()),
	        chm_ui, SLOT(groupSplit_clicked()));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        chm_ui, SLOT(hideInactive_clicked(bool)));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        this, SLOT(on_btnShowChannelsClicked(bool)));
	connect(chm_ui, SIGNAL(widthChanged(int)),
		this, SLOT(onChmWidthChanged(int)));

	trigger_settings_ui->btnAuto->setChecked(false);
	trigger_settings_ui->btnNormal->setChecked(true);
	main_win->view_->viewport()->setTimeTriggerPosActive(true);
	ui->areaTimeTriggerLayout->addWidget(this->bottomHandlesArea(), 0, 1, 1, 3);
	updateAreaTimeTriggerPadding();
	ui->triggerStateLabel->setText("Stop");

	ensurePolished();
	main_win->view_->viewport()->ensurePolished();
	last_set_sample_count = 0;
	timeBase->setValue(1e-3);
	setTimebaseLabel(timeBase->value());
	onHorizScaleValueChanged(timeBase->value());
	setBuffersizeLabelValue(active_sampleCount);
	setSamplerateLabelValue(active_sampleRate);
	setSampleRate();

	timePosition->setValue(0);
	timePosition->valueChanged(timePosition->value());
	main_win->view_->viewport()->setTimeTriggerSample(-active_triggerSampleCount);
	setCursorsActive(false);

	timer->setInterval(timer_timeout_ms);
	QMetaObject::invokeMethod(timer, "start",Qt::QueuedConnection);

	la_api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_LOGIC_ANALYZER)));
	la_api->load();
	la_api->js_register(engine);

	chm_ui->setWidgetMinimumNrOfChars(ui->triggerStateLabel, 9);
	state_timer->setInterval(2);
	chm_ui->update_ui();
}

LogicAnalyzer::~LogicAnalyzer()
{
	la_api->save();
	delete la_api;

	if(running)
		startStop(false);
	logic_analyzer_ptr.reset();
	context.reset();
	timer->stop();

	delete trigger_settings_ui;
	delete ui;

	/* Destroy libsigrokdecode */
	srd_exit();
	qDeleteAll(channel_groups_api);
	channel_groups_api.clear();
}

void LogicAnalyzer::set_buffersize()
{
	main_win->view_->session().set_buffersize(logic_analyzer_ptr->get_buffersize());
	if(running) {
		//restart acquisition to recreate buffer
		main_win->restart_acquisition();
	}
}

void LogicAnalyzer::get_channel_groups_api()
{
	qDeleteAll(channel_groups_api);
	channel_groups_api.clear();
	for(int i=0; i < chm.get_channel_group_count(); i++) {
		channel_groups_api.append(new ChannelGroup_API(this, i, false));
	}
}

void LogicAnalyzer::resizeEvent()
{
	if(!initialised) {
		updateAreaTimeTriggerPadding();
		timePosition->setValue(0);
		value_cursor1 = -(timeBase->value() * 3 + active_plot_timebase * 10 / 2 - active_timePos);
		value_cursor2 = -(timeBase->value() * 6 + active_plot_timebase * 10 / 2 - active_timePos);
		cursorValueChanged_1(timeToPixel(value_cursor1));
		cursorValueChanged_2(timeToPixel(value_cursor2));
		initialised = true;
	}
	int x1 = timeToPixel(value_cursor1);
	d_hCursorHandle1->setPositionSilenty(x1);
	main_win->view_->viewport()->cursorValueChanged_1(x1);

	int x2 = timeToPixel(value_cursor2);
	d_hCursorHandle2->setPositionSilenty(x2);
	main_win->view_->viewport()->cursorValueChanged_2(x2);

	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);
}

double LogicAnalyzer::pickSampleRateFor(double timeSpanSecs, double desiredBuffersize)
{
	double idealSamplerate = desiredBuffersize / timeSpanSecs;

	// Pick the highest sample rate that we can set, that is lower or equal to
	// the idealSampleRate.
	double divider = maxSampleRate / idealSamplerate;
	double intpart = ceil(divider);

	if( intpart != 0 )
	{
		idealSamplerate = maxSampleRate / intpart;
	}
	else
	{
		idealSamplerate = maxSampleRate;
	}

	return idealSamplerate;
}

void LogicAnalyzer::startTimeout()
{
	timer->setSingleShot(true);
	timer->start(timer_timeout_ms);
}

void LogicAnalyzer::triggerTimeout()
{
	if(armed) {
		armed = false;
		autoCaptureEnable();
		timer->setInterval(timer_timeout_ms);
	}
}

void LogicAnalyzer::triggerStateTimeout()
{
	ui->triggerStateLabel->setText(trigger_state);
}

void LogicAnalyzer::refilling()
{
	if(!timer->isActive())
		Q_EMIT startRefill();
}

void LogicAnalyzer::captured()
{
	Q_EMIT capturedSignal();
}

void LogicAnalyzer::capturedSlot()
{
	if(timer->isActive()) {
		timer->stop();
	}
	else {
		armed = true;
		autoCaptureEnable();
	}
}

void LogicAnalyzer::set_triggered_status(std::string value)
{
	if(!trigger_settings_ui)
		return;
	if(trigger_settings_ui->btnAuto->isChecked()) {
		state_timer->stop();
		ui->triggerStateLabel->setText("Auto");
	}
	else {
		if( value == "awaiting" )
			trigger_state = "Waiting";
		else if(value == "running")
			trigger_state = "Triggered";
		else if(value == "stopped")
			trigger_state = "Stop";
	}
}

void LogicAnalyzer::onHorizScaleValueChanged(double value)
{
	symmBufferMode->setTimeBase(value);
	LogicAnalyzerSymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sampleRate = params.sampleRate;
	active_sampleCount = params.entireBufferSize;
	active_triggerSampleCount = -(long long)params.triggerBufferSize;
	active_timePos = -params.timePos;
	active_plot_timebase = value;

	double plotTimeSpan = value * 10; //Hdivision count
	timer_timeout_ms = plotTimeSpan * 1000  + 100; //transfer time

	custom_sampleCount = maxBuffersize / plotRefreshRate;

	enableTrigger(true);
	if( plotTimeSpan >= timespanLimitStream )
	{
		if(logic_analyzer_ptr->get_buffersize() != custom_sampleCount) {
			logic_analyzer_ptr->set_buffersize(custom_sampleCount);
			set_buffersize();
		}
		enableTrigger(false);
		active_triggerSampleCount = 0;
	}
	else if( logic_analyzer_ptr )
	{
		if(logic_analyzer_ptr->get_buffersize() != active_sampleCount)
		{
			logic_analyzer_ptr->set_buffersize(active_sampleCount);
			set_buffersize();
		}
	}

	if( running )
	{
		last_set_sample_count = active_sampleCount;
		setSampleRate();
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
		setHWTriggerDelay(active_triggerSampleCount);

		if( timePosition->value() != -params.timePos ) {
			timePosition->setValue(-params.timePos);
		}
	}
	setTriggerDelay();

	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);

	// Change the sensitivity of time position control
	timePosition->setStep(value / 10);
	recomputeCursorsValue(true);
	updateBufferPreviewer();
}

void LogicAnalyzer::enableTrigger(bool value)
{
	if( !value )
		d_timeTriggerHandle->hide();
	else
		d_timeTriggerHandle->show();
	main_win->view_->viewport()->setTimeTriggerPosActive(value);
	main_win->view_->time_item_appearance_changed(true, true);
}

void LogicAnalyzer::setSampleRate()
{
	options["samplerate"] = Glib::Variant<guint64>(
	                  g_variant_new_uint64(active_sampleRate),true);
	Glib::VariantBase tmp = logic_analyzer_ptr->get_options()["samplerate"];

	if( logic_analyzer_ptr )
	{
		if(!tmp.equal(options["samplerate"])) {
			main_win->view_->session().set_samplerate(active_sampleRate);
			logic_analyzer_ptr->set_options(options);
		}
	}

	/* Set IIO device parameters */
	iio_device_attr_write_longlong(dev, "sampling_frequency", active_sampleRate);
	active_hw_sampleRate = active_sampleRate;
}

void LogicAnalyzer::updateBuffersizeSamplerateLabel(int samples, double samplerate)
{
	QString txtSamplerate = d_cursorMetricFormatter.format(samplerate, "Hz", 0);
	QString txtSampleperiod = d_cursorTimeFormatter.format(1 / samplerate, "", 0);
	QString text = QString("%1 Samples at ").arg(samples) + txtSamplerate +
		"/" + txtSampleperiod;
	ui->samplerateLabel->setText(text);
}

void LogicAnalyzer::setTimebaseLabel(double value)
{
	QString text = d_cursorTimeFormatter.format(value, "", 3);
	ui->timebaseLabel->setText(text + "/div");
}

void LogicAnalyzer::setBuffersizeLabelValue(double value)
{
	d_bufferSizeLabelVal = value;
	updateBuffersizeSamplerateLabel(value, d_sampleRateLabelVal);
}

void LogicAnalyzer::setSamplerateLabelValue(double value)
{
	d_sampleRateLabelVal = value;
	updateBuffersizeSamplerateLabel(d_bufferSizeLabelVal, value);
}

void LogicAnalyzer::updateAreaTimeTriggerPadding()
{
	ui->areaTimeTriggerLayout->setContentsMargins(
		chm_ui->sizeHint().width() - 20, 0, 0, 0);
}

void LogicAnalyzer::onRulerChanged(double ruler_value, bool silent)
{
	double timePos = ruler_value + active_plot_timebase * 10 / 2;
	if(!silent) {
		if(timePosition->value() != timePos)
			timePosition->setValue(timePos);
	}
	else {
		active_plot_timebase = main_win->view_->scale();
		int pix = timeToPixel(-timePos);
		if( pix != d_timeTriggerHandle->position() )
		{
			d_timeTriggerHandle->setPositionSilenty(pix);
		}
		if( active_plot_timebase != timeBase->value()) {
			QString text = d_cursorTimeFormatter.format(
						active_plot_timebase, "", 3);
			ui->timebaseLabel->setText("Zoom: " + text + "/div");
			recomputeCursorsValue(true);
		}
		else {
			setTimebaseLabel(active_plot_timebase);
		}
	}
	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);
	updateBufferPreviewer();
}

QWidget* LogicAnalyzer::bottomHandlesArea()
{
	return d_bottomHandlesArea;
}

void LogicAnalyzer::refreshTriggerPos(int px)
{
	d_timeTriggerHandle->setPositionSilenty(px);
}

void LogicAnalyzer::onTimePositionSpinboxChanged(double value)
{
	symmBufferMode->setTriggerPos(-value);
	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sampleRate = params.sampleRate;
	active_sampleCount = params.entireBufferSize;
	active_triggerSampleCount = -(long long)params.triggerBufferSize;
	active_timePos = -params.timePos;

	int pix = timeToPixel(-value);
	if( logic_analyzer_ptr )
	{
		if(logic_analyzer_ptr->get_buffersize() != active_sampleCount)
		{
			logic_analyzer_ptr->set_buffersize(active_sampleCount);
			set_buffersize();
		}
	}
	if( running )
	{
		last_set_sample_count = active_sampleCount;
		setSampleRate();
		setHWTriggerDelay(active_triggerSampleCount);
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
	}
	setTriggerDelay();
	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);
	recomputeCursorsValue(false);
	updateBufferPreviewer();
}

void LogicAnalyzer::onTimeTriggerHandlePosChanged(int pos)
{
	int width = bottomHandlesArea()->geometry().width() -
			d_bottomHandlesArea->leftPadding() -
			d_bottomHandlesArea->rightPadding();
	double time = pixelToTime(pos);
	if( (time + active_plot_timebase * 10 / 2) != active_timePos )
	{
		timePosition->setValue(time + active_plot_timebase * 10 / 2);
	}
	main_win->view_->viewport()->setTimeTriggerPixel(pos);
	setTriggerDelay();
}

double LogicAnalyzer::pixelToTime(int pix)
{
	double timeSpan = active_plot_timebase * 10;
	int width = bottomHandlesArea()->geometry().width() -
		d_bottomHandlesArea->leftPadding() -
		d_bottomHandlesArea->rightPadding();
	double timestamp = timeSpan * pix / width;
	return -timestamp;
}

int LogicAnalyzer::timeToPixel(double time)
{
	double timeSpan = active_plot_timebase * 10;
	int width = bottomHandlesArea()->geometry().width() -
		d_bottomHandlesArea->leftPadding() -
		d_bottomHandlesArea->rightPadding();
	int pix = width * time / timeSpan + width / 2;
	return pix;
}

void LogicAnalyzer::startStop(bool start)
{
	if (start) {
		last_set_sample_count = active_sampleCount;
		if(main_win->view_->scale() != timeBase->value())
			Q_EMIT timeBase->valueChanged(timeBase->value());
		main_win->view_->viewport()->disableDrag();
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
		setSampleRate();
		running = true;
		ui->btnRunStop->setText("Stop");
		ui->btnSingleRun->setEnabled(false);
		setHWTriggerDelay(active_triggerSampleCount);
		setTriggerDelay();
		if (timePosition->value() != active_timePos)
			timePosition->setValue(active_timePos);
		if(!armed)
			armed = true;
		state_timer->start(2);
	} else {
		main_win->view_->viewport()->enableDrag();
		running = false;
		ui->btnRunStop->setText("Run");
		ui->btnSingleRun->setEnabled(true);
		if(timer->isActive()) {
			timer->stop();
		}
		if(!armed && trigger_settings_ui->btnAuto->isChecked()) {
			armed = true;
			autoCaptureEnable();
		}
	}
	main_win->run_stop();
//	setTriggerDelay();
}

void LogicAnalyzer::setTriggerDelay(bool silent)
{
	if( !silent ) {
		main_win->view_->set_offset(timePosition->value(), active_plot_timebase * 10, running);
		if( running )
			main_win->view_->viewport()->setTimeTriggerSample(
				-active_triggerSampleCount);
	}
}

void LogicAnalyzer::setHWTriggerDelay(long long delay)
{
	std::string name = "voltage0";
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	QString s = QString::number(delay);
	iio_channel_attr_write(triggerch, "trigger_delay",
		s.toLocal8Bit().QByteArray::constData());
	active_hw_trigger_sample_count = delay;
}

void LogicAnalyzer::singleRun()
{
	if( running )
	{
		startStop(false);
		ui->btnRunStop->setChecked(false);
	}
	last_set_sample_count = active_sampleCount;
	if(main_win->view_->scale() != timeBase->value())
		Q_EMIT timeBase->valueChanged(timeBase->value());
	setSampleRate();
	running = true;
	setBuffersizeLabelValue(active_sampleCount);
	setSamplerateLabelValue(active_sampleRate);
	setHWTriggerDelay(active_triggerSampleCount);
	setTriggerDelay();
	if (timePosition->value() != active_timePos)
		timePosition->setValue(active_timePos);
	logic_analyzer_ptr->set_single(true);
	main_win->run_stop();
//	setTriggerDelay();
	running = false;
}

unsigned int LogicAnalyzer::get_no_channels(struct iio_device *dev)
{
	unsigned int nb = 0;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_output(chn) &&
		    iio_channel_is_scan_element(chn)) {
			nb++;
		}
	}

	return nb;
}

void LogicAnalyzer::clearLayout(QLayout *layout)
{
	for (int i = 0 ; i < layout->children().size();) {
		delete layout->takeAt(i);
	}

	delete layout;
}

void LogicAnalyzer::toggleRightMenu(QPushButton *btn)
{
	int id = btn->property("id").toInt();
	bool btn_old_state = btn->isChecked();
	bool open = !menuOpened;

	settings_group->setExclusive(!btn_old_state);

	if( open )
		settings_panel_update(id);

	active_settings_btn = btn;
	ui->rightWidget->toggleMenu(open);
}

void LogicAnalyzer::settings_panel_update(int id)
{
	if (id < 0) {
		ui->stackedWidget->setCurrentIndex(-id);
	}
	else {
		ui->stackedWidget->setCurrentIndex(id);
	}
}

void LogicAnalyzer::toggleRightMenu()
{
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

void LogicAnalyzer::setHWTrigger(int chid, std::string trigger_val)
{
	std::string name = "voltage" + to_string(chid);
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);

	if( !triggerch )
		return;

	iio_channel_attr_write(triggerch, "trigger", trigger_val.c_str());
}

std::string LogicAnalyzer::get_trigger_from_device(int chid)
{
	std::string name = "voltage" + to_string(chid);
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	if( !triggerch )
		return "";
	char trigger_val[4096];
	iio_channel_attr_read(triggerch, "trigger", trigger_val, sizeof(trigger_val));
	string res(trigger_val);
	return res;
}

void LogicAnalyzer::toggleLeftMenu(bool val)
{
	if (val) {
		ui->btnGroupChannels->hide();
		chm_ui->collapse(true);
	} else {
		ui->btnGroupChannels->show();
		chm_ui->collapse(false);
	}
}

void LogicAnalyzer::rightMenuFinished(bool opened)
{
	menuOpened = opened;

	if (!opened && active_settings_btn && active_settings_btn->isChecked()) {
		int id = active_settings_btn->property("id").toInt();
		settings_panel_update(id);
		ui->rightWidget->toggleMenu(true);
	}
}

void LogicAnalyzer::leftMenuFinished(bool closed)
{
	if (ui->btnShowHideMenu->isChecked() && !closed) {
		ui->btnGroupChannels->hide();
		ui->btnShowChannels->hide();
		chm_ui->collapse(true);
	} else {
		ui->btnGroupChannels->show();
		ui->btnShowChannels->show();
		chm_ui->collapse(false);
	}
}

void LogicAnalyzer::on_btnShowChannelsClicked(bool check)
{
	if (check) {
		ui->btnShowChannels->setText("Show all");
	} else {
		ui->btnShowChannels->setText("Hide inactive");
	}
}

void LogicAnalyzer::onChmWidthChanged(int value)
{
	int l, r, b, t;
	ui->areaTimeTriggerLayout->getContentsMargins(&l, &t, &r, &b);
	if(l != value - 20 ){
		ui->areaTimeTriggerLayout->setContentsMargins(value - 20, 0, 0, 0);
		timePosition->valueChanged(timePosition->value());

	}
}

void LogicAnalyzer::setHWTriggerLogic(const QString value)
{
	std::string name = "voltage0";
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	QString s = value.toLower();
	iio_channel_attr_write(triggerch, "trigger_logic_mode",
		s.toLocal8Bit().QByteArray::constData());
}

void LogicAnalyzer::updateBufferPreviewer()
{
	// Time interval within the plot canvas
	double plotMin = -(active_plot_timebase * 10 / 2 - active_timePos);
	double plotMax = (active_plot_timebase * 10 / 2 + active_timePos);

	// Time interval that represents the captured data
	double dataMin = 0;
	double dataMax = 0;
	long long triggerSamples = active_hw_trigger_sample_count;
	long long totalSamples = last_set_sample_count;

	if(totalSamples > 0) {
		dataMin = triggerSamples / active_hw_sampleRate;
		dataMax = (triggerSamples + totalSamples) / active_hw_sampleRate;
	}

	double fullMin, fullMax;
	if( plotMin < dataMin)
		fullMin = plotMin;
	else
		fullMin = dataMin;
	if( plotMax > dataMax)
		fullMax = plotMax;
	else
		fullMax = dataMax;

	double wPos = 1 - (fullMax - dataMin) / (fullMax - fullMin);
	double wWidth = (dataMax - dataMin) / (fullMax - fullMin);

	double hPos = 1 - (fullMax - plotMin) / (fullMax - fullMin);
	double hWidth = (plotMax - plotMin) / (fullMax - fullMin);

	double containerMin = (totalSamples > 0) ? dataMin : fullMin;
	double containerMax = (totalSamples > 0) ? dataMax : fullMax;
	double containerWidth = (totalSamples > 0) ? wWidth : 1;
	double containerPos = (totalSamples > 0) ? wPos : 0;

	double cPosInContainer = 1 - (containerMax - 0) /
		(containerMax - containerMin);
	double cPos = cPosInContainer * containerWidth + containerPos;

	// Update the widget
	buffer_previewer->setWaveformWidth(wWidth);
	buffer_previewer->setWaveformPos(wPos);
	buffer_previewer->setHighlightWidth(hWidth);
	buffer_previewer->setHighlightPos(hPos);
	buffer_previewer->setCursorPos(cPos);

}

void LogicAnalyzer::setupTriggerSettingsUI(bool enabled)
{
	trigger_settings_ui->cmb_trigg_extern_cond_1->setEnabled(enabled);
	trigger_settings_ui->cmb_trigg_extern_cond_2->setEnabled(enabled);
	if( !enabled ) {
		trigger_settings_ui->cmb_trigg_extern_cond_1->setCurrentIndex(0);
		trigger_settings_ui->cmb_trigg_extern_cond_2->setCurrentIndex(0);
		setHWTrigger(16, trigger_mapping[0]);
		setHWTrigger(17, trigger_mapping[0]);
	}
	else {
		for(int i = 0; i < get_no_channels(dev) + 2; i++) {
			setHWTrigger(i, trigger_mapping[0]);
			if(i < get_no_channels(dev))
				chm.get_channel(i)->setTrigger(trigger_mapping[0]);
		}
		chm_ui->update_ui();
	}
}

void LogicAnalyzer::autoCaptureEnable()
{
	if(armed) {
		for(int i = 0; i < get_no_channels(dev) + 2; i++) {
			setHWTrigger(i, trigger_cache[i]);
		}
	}
	else {
		for(int i = 0; i < get_no_channels(dev) + 2; i++) {
			trigger_cache[i] = get_trigger_from_device(i);
			setHWTrigger(i, trigger_mapping[0]);
		}
	}
}

void LogicAnalyzer::setTriggerCache(int chid, std::string trigger_value)
{
	trigger_cache[chid] = trigger_value;
}

void LogicAnalyzer::setExternalTrigger(int index)
{
	int ext_1 = trigger_settings_ui->cmb_trigg_extern_cond_1->currentIndex();
	int ext_2 = trigger_settings_ui->cmb_trigg_extern_cond_2->currentIndex();
	std::string trigger_val;
	if( ext_1 == index ) {
		trigger_val = trigger_mapping[ext_1];
		setHWTrigger(16, trigger_val);
	}
	if( ext_2 == index ) {
		trigger_val = trigger_mapping[ext_2];
		setHWTrigger(17, trigger_val);
	}
}

void LogicAnalyzer::triggerChanged(int index)
{
	if(index != 0 && trigger_settings_ui->trigg_extern_en->isChecked() ) {
		trigger_settings_ui->trigg_extern_en->setChecked(false);
	}
}

void LogicAnalyzer::cleanHWParams()
{
	for(int i = 0; i < get_no_channels(dev) + 2; i++) {
		setHWTrigger(i, trigger_mapping[0]);
	}
	setHWTriggerDelay(active_triggerSampleCount);
	setHWTriggerLogic("or");
}


void LogicAnalyzer::cursorValueChanged_1(int pos)
{
	value_cursor1 = -(pixelToTime(pos) + active_plot_timebase * 10 / 2 - active_timePos);
	if( ui->btnCursorsLock->isChecked() ) {
		value_cursor2 = value_cursor1 - value_cursors_delta;
		int pairPos = timeToPixel(value_cursor2);
		d_hCursorHandle2->setPositionSilenty(pairPos);
		QString text = d_cursorTimeFormatter.format(value_cursor2, "", 3);
		ui->lblCursor2->setText(text);
		main_win->view_->viewport()->cursorValueChanged_2(pairPos);
	}
	else {
		value_cursors_delta = value_cursor1 - value_cursor2;
		cursorsFormatDelta();
	}
	QString text = d_cursorTimeFormatter.format(value_cursor1, "", 3);
	ui->lblCursor1->setText(text);
	main_win->view_->viewport()->cursorValueChanged_1(pos);
}

void LogicAnalyzer::cursorValueChanged_2(int pos)
{
	value_cursor2 = -(pixelToTime(pos) + active_plot_timebase * 10 / 2 - active_timePos);
	if( ui->btnCursorsLock->isChecked() ) {

		value_cursor1 = value_cursors_delta + value_cursor2;
		int pairPos = timeToPixel(value_cursor1);
		d_hCursorHandle1->setPositionSilenty(pairPos);
		QString text = d_cursorTimeFormatter.format(value_cursor1, "", 3);
		ui->lblCursor1->setText(text);
		main_win->view_->viewport()->cursorValueChanged_1(pairPos);
	}
	else {
		value_cursors_delta = value_cursor1 - value_cursor2;
		cursorsFormatDelta();
	}
	QString text = d_cursorTimeFormatter.format(value_cursor2, "", 3);
	ui->lblCursor2->setText(text);
	main_win->view_->viewport()->cursorValueChanged_2(pos);
}

void LogicAnalyzer::recomputeCursorsValue(bool zoom)
{
	int x1 = d_hCursorHandle1->position();
	int x2 = d_hCursorHandle2->position();
	value_cursor2 = -(pixelToTime(x1) + active_plot_timebase * 10 / 2 - active_timePos);
	value_cursor1 = -(pixelToTime(x2) + active_plot_timebase * 10 / 2 - active_timePos);
	QString text = d_cursorTimeFormatter.format(value_cursor2, "", 3);
	ui->lblCursor2->setText(text);
	text = d_cursorTimeFormatter.format(value_cursor1, "", 3);
	ui->lblCursor1->setText(text);
	if( zoom ) {
		value_cursors_delta = value_cursor1 - value_cursor2;
		cursorsFormatDelta();
	}
}

void LogicAnalyzer::cursorsFormatDelta()
{
	QString text = d_cursorTimeFormatter.format(value_cursors_delta, "", 3);
	QString freqDeltaText;
	if( value_cursors_delta != 0)
		freqDeltaText = d_cursorMetricFormatter.format(1/value_cursors_delta, "Hz", 3);
	else
		freqDeltaText = "Infinity";
	ui->lblCursorDiff->setText(text);
	ui->lblCursorDelta->setText(freqDeltaText);
}

void LogicAnalyzer::setCursorsActive(bool active)
{
	main_win->view_->viewport()->setCursorsActive(active);
	if(active) {
		d_hCursorHandle1->show();
		d_hCursorHandle2->show();
		ui->cursorsStatusWidget->show();
	}
	else  {
		d_hCursorHandle1->hide();
		d_hCursorHandle2->hide();
		ui->cursorsStatusWidget->hide();
	}
}

void LogicAnalyzer::resetInstrumentToDefault()
{
	chm.clearChannelGroups();
	for(int i = 0; i < no_channels; i++) {
		chm.add_channel_group(new LogicAnalyzerChannelGroup(chm.get_channel(i)));
	}
	chm.highlightChannel(chm.get_channel_group(0));
	chm_ui->update_ui();
	timePosition->setValue(0);
	timeBase->setValue(1e-3);
	ui->btnCursorsLock->setChecked(false);
	ui->boxCursors->setChecked(false);
}

void LogicAnalyzer::setTimeout(bool checked)
{
	logic_analyzer_ptr->set_timeout(checked);
}

/*
 * class LogicAnalyzer_API
 */


bool LogicAnalyzer_API::running() const
{
	return lga->ui->btnRunStop->isChecked();
}

int LogicAnalyzer_API::channel_groups_list_size() const
{
	return lga->channel_groups_api.size();
}
void LogicAnalyzer_API::setChannelGroupsListSize(int size)
{
	qDeleteAll(lga->channel_groups_api);
	lga->channel_groups_api.clear();

	lga->chm.clearChannelGroups();
	for (int i = 0; i < size; i++) {
			lga->channel_groups_api.append(new ChannelGroup_API(lga, i));
			lga->chm.add_channel_group(new LogicAnalyzerChannelGroup());
	}
}

QList<ChannelGroup_API*> LogicAnalyzer_API::getChannelGroupsForStoring()
{
	return lga->channel_groups_api;
}

QQmlListProperty<ChannelGroup_API> LogicAnalyzer_API::getChannelGroupsForScripting()
{
	return QQmlListProperty<ChannelGroup_API>(this, lga->channel_groups_api);
}

void LogicAnalyzer_API::run(bool en)
{
	lga->ui->btnRunStop->setChecked(en);
}

double LogicAnalyzer_API::getTimePos() const
{
	return lga->timePosition->value();
}

void LogicAnalyzer_API::setTimePos(double value)
{
	lga->timePosition->setValue(value);
}

double LogicAnalyzer_API::getTimeBase() const
{
	return lga->timeBase->value();
}

void LogicAnalyzer_API::setTimeBase(double value)
{
	lga->timeBase->setValue(value);
}

bool LogicAnalyzer_API::externalTrigger() const
{
	return lga->trigger_settings_ui->trigg_extern_en->isChecked();
}

void LogicAnalyzer_API::setExternalTrigger(bool en)
{
	lga->trigger_settings_ui->trigg_extern_en->setChecked(en);
}

bool LogicAnalyzer_API::cursorsActive() const
{
	lga->ui->boxCursors->isChecked();
}

void LogicAnalyzer_API::setCursorsActive(bool en)
{
	lga->ui->boxCursors->setChecked(en);
}

bool LogicAnalyzer_API::cursorsLocked() const
{
	lga->ui->btnCursorsLock->isChecked();
}

void LogicAnalyzer_API::setCursorsLocked(bool en)
{
	lga->ui->btnCursorsLock->setChecked(en);
}

bool LogicAnalyzer_API::inactiveHidden() const
{
	return lga->ui->btnShowChannels->isChecked();
}

void LogicAnalyzer_API::setInactiveHidden(bool en)
{
	lga->ui->btnShowChannels->clicked(en);
}


/*
 * ChannelGroup_API
 */
bool ChannelGroup_API::chEnabled() const
{
	if( lga->chm.get_channel_group(getIndex()) )
		return lga->chm.get_channel_group(getIndex())->is_enabled();
	return false;
}

void ChannelGroup_API::setChEnabled(bool en)
{
	lga->chm.get_channel_group(getIndex())->enable(en);
}

bool ChannelGroup_API::chGrouped() const
{
	return lga->chm.get_channel_group(getIndex())->is_grouped();
}

void ChannelGroup_API::setChGrouped(bool en)
{
	lga->chm.get_channel_group(getIndex())->group(en);
}

QString ChannelGroup_API::getTrigger() const
{
	if(!chGrouped()) {
		auto ch_group = lga->chm.get_channel_group(getIndex());
		auto ch = ch_group->get_channel();
		if(ch)
			return QString::fromStdString(lga->chm.get_channel(
				ch->get_id())->getTrigger());
	}
	return "none";
}

void ChannelGroup_API::setTrigger(QString val)
{
	if(!chGrouped()) {
		auto ch_group = lga->chm.get_channel_group(getIndex());
		auto ch = ch_group->get_channel();
		if(ch)
			lga->chm.get_channel(ch->get_id())->setTrigger(
				val.toStdString());
	}
}

QString ChannelGroup_API::getName() const
{
	auto ch_group = lga->chm.get_channel_group(getIndex());
	if( ch_group ) {
		return QString::fromStdString(ch_group->get_label());
	}
	return "";
}

void ChannelGroup_API::setName(QString val)
{
	lga->chm.get_channel_group(getIndex())->set_label(val.toStdString());
}

bool ChannelGroup_API::getChCollapsed() const
{
	return lga->chm.get_channel_group(this->getIndex())->isCollapsed();
}

void ChannelGroup_API::setChCollapsed(bool en)
{
	lga->chm.get_channel_group(this->getIndex())->collapse(en);
}

int ChannelGroup_API::getIndex() const
{
	if(index == -1) {
		return lga->channel_groups_api.indexOf(
			const_cast<ChannelGroup_API*>(this));
	}
	return index;
}

int ChannelGroup_API::channels_list_size() const
{
	return channels_api.size();
}
void ChannelGroup_API::setChannelsListSize(int size)
{
	qDeleteAll(channels_api);
	channels_api.clear();
	for (int i = 0; i < size; i++) {
		channels_api.append(new LogicChannel_API(lga, this));
	}
}

QList<LogicChannel_API*> ChannelGroup_API::getChannelsForStoring()
{
	return channels_api;
}

QQmlListProperty<LogicChannel_API> ChannelGroup_API::getChannelsForScripting()
{
	return QQmlListProperty<LogicChannel_API>(this, channels_api);
}

void ChannelGroup_API::set_channels_api()
{
	qDeleteAll(channels_api);
	channels_api.clear();
	auto ch_group = lga->chm.get_channel_group(index);
	if( ch_group ) {
		int size = ch_group->get_channel_count();
		int ch_index;
		for(int i=0; i < size; i++) {
			if(ch_group->get_channel(i)) {
				ch_index = ch_group->get_channel(i)->get_id();
				channels_api.append(
					new LogicChannel_API(lga, this, ch_index));
			}
		}
	}
}

/*
 * Channel_API
 */

QString LogicChannel_API::getTrigger() const
{
	return QString::fromStdString(lga->chm.get_channel(
	      getIndex())->getTrigger());
}

void LogicChannel_API::setTrigger(QString val)
{
	lga->chm.get_channel(getIndex())->setTrigger(val.toStdString());
}

QString LogicChannel_API::getName() const
{
	return QString::fromStdString(lga->chm.get_channel(
		getIndex())->get_label());
}

void LogicChannel_API::setName(QString val)
{
	lga->chm.get_channel(getIndex())->set_label(val.toStdString());
}

int LogicChannel_API::getIndex() const
{
	return index;
}

void LogicChannel_API::setIndex(int val)
{
	index = val;
	lga->chm.get_channel_group(lchg->getIndex())->add_channel(
		lga->chm.get_channel(val));
}
