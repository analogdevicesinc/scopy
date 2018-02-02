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

/* Qt includes */
#include <QDebug>
#include <QListView>
#include <QPushButton>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>

/* Local includes */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/session.hpp"
#include "pulseview/pv/view/ruler.hpp"
#include "pulseview/pv/data/logic.hpp"
#include "pulseview/pv/data/logicsegment.hpp"
#include "streams_to_short.h"
#include "logic_analyzer.hpp"
#include "spinbox_a.hpp"
#include "scroll_filter.hpp"
#include "buffer_previewer.hpp"
#include "handles_area.hpp"
#include "plot_line_handle.h"
#include "state_updater.h"
#include "dynamicWidget.hpp"
#include "config.h"
#include "osc_export_settings.h"

/* Sigrok includes */
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <libsigrokdecode/libsigrokdecode.h>

/* Generated UI */
#include "ui_logic_analyzer.h"
#include "ui_logic_channel_settings.h"
#include "ui_digital_trigger_settings.h"
#include "ui_la_channel_group.h"

/* Boost includes */
#include <boost/thread.hpp>

using namespace std;
using namespace adiscope;
using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;
using sigrok::ConfigKey;

const unsigned long LogicAnalyzer::maxBuffersize = 16000;
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
                             ToolLauncher *parent,
                             bool offline_mode_) :
	Tool(ctx, runBtn, new LogicAnalyzer_API(this), "Logic Analyzer", parent),
	itemsize(sizeof(uint16_t)),
	dev_name(),
	dev(nullptr),
	menuOpened(false),
	settings_group(new QButtonGroup(this)),
	ui(new Ui::LogicAnalyzer),
	active_settings_btn(nullptr),
	timespanLimitStream(11),
	plotRefreshRate(100),
	active_sampleRate(0.0),
	maxSamplingFrequency(0),
	active_hw_sampleRate(0.0),
	active_sampleCount(0),
	active_triggerSampleCount(0),
	active_hw_trigger_sample_count(0),
	active_timePos(0),
	trigger_settings(new QWidget(this)),
	value_cursor1(-0.033),
	value_cursor2(0.033),
	initialised(false),
	timer(new QTimer(this)),
	armed(false),
	offline_mode(offline_mode_),
	zoomed_in(false),
	triggerUpdater(new StateUpdater(250, this)),
	trigger_is_forced(false)
{
	ui->setupUi(this);
	timer->setSingleShot(true);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	if(!offline_mode) {
		iio_context_set_timeout(ctx, UINT_MAX);
		dev_name = filt->device_name(TOOL_LOGIC_ANALYZER);
		dev = iio_context_find_device(ctx, dev_name.c_str());
	}
	this->configureMaxSampleRate();
	if( maxSamplingFrequency == 0 )
		maxSamplingFrequency = 80000000;

	symmBufferMode = make_shared<LogicAnalyzerSymmetricBufferMode>();
	symmBufferMode->setMaxSampleRate(maxSamplingFrequency);
	symmBufferMode->setEntireBufferMaxSize(500000); // max 0.5 mega-samples
	symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
	symmBufferMode->setTimeDivisionCount(10);

	/* Buffer Previewer widget */
	buffer_previewer = new DigitalBufferPreviewer(40, this);

	buffer_previewer->setVerticalSpacing(6);
	buffer_previewer->setMinimumHeight(20);
	buffer_previewer->setMaximumHeight(20);
	buffer_previewer->setMinimumWidth(375);
	buffer_previewer->setMaximumWidth(375);
	ui->vLayoutBufferSlot->addWidget(buffer_previewer);

	buffer_previewer->setCursorPos(0.5);

	for(int i=0; i < get_no_channels(dev)+2; i++) {
		trigger_cache.push_back(trigger_mapping[0]);
	}

	/* Time position widget */
	this->d_bottomHandlesArea = new HorizHandlesArea(this);
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

	device_manager = new pv::DeviceManager(context);
	pv::MainWindow *w = new pv::MainWindow(*device_manager, filt, open_file,
	                                       open_file_format, this);

	for (unsigned int j = 0; j < get_no_channels(dev); j++) {
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
	}, "Time Base", 10e-9, 10e0,
	true, false, this);
	timePosition = new PositionSpinButton({
		{"ns", 1E-9},
		{"μs", 1E-6},
		{"ms", 1E-3},
		{"s", 1E0}
	}, "Position",
	-timeBase->maxValue() * 5,
	timeBase->maxValue() * 5,
	true,
	false,
	this);

	ui->verticalLayout_7->insertWidget(ui->verticalLayout_7->count() - 6,
		timeBase, 0, Qt::AlignLeft);
	ui->verticalLayout_7->insertWidget(ui->verticalLayout_7->count() - 5,
		timePosition, 0, Qt::AlignLeft);

	QDoubleValidator *validator = new QDoubleValidator(ui->lineeditSampleRate);
	validator->setNotation(QDoubleValidator::ScientificNotation);
	validator->setDecimals(5);

	ui->lineeditSampleRate->setValidator(validator);

	int chn = (no_channels == 0) ? 16 : no_channels;
	options["numchannels"] = Glib::Variant<gint32>(
			g_variant_new_int32(chn),true);

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
	connect(ui->btnSingleRun, SIGNAL(toggled(bool)),
		this, SLOT(singleRun(bool)));
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
	connect(trigger_settings_ui->btnAuto, SIGNAL(toggled(bool)),
		this, SLOT(onTriggerModeChanged(bool)));

	connect(this, SIGNAL(starttimeout()),
		this, SLOT(startTimer()));
	connect(this, SIGNAL(stoptimeout()),
		this, SLOT(stopTimer()));
	connect(timer, &QTimer::timeout,
		this, &LogicAnalyzer::triggerTimeout);

	connect(main_win->view_, SIGNAL(new_segment_received()),
		this, SLOT(requestUpdateBufferPreviewer()));
	connect(main_win->view_, SIGNAL(new_segment_received()),
		this, SLOT(onDataReceived()));
	connect(main_win->view_, SIGNAL(frame_ended()),
		this, SLOT(onFrameEnded()));
	connect(ui->cmbRunMode, SIGNAL(currentIndexChanged(int)),
		this, SLOT(runModeChanged(int)));
	connect(ui->lineeditSampleRate, &QLineEdit::returnPressed,
		this, &LogicAnalyzer::validateSamplingFrequency);
	connect(ui->btnApply, SIGNAL(clicked()),
		this, SLOT(validateSamplingFrequency()));
	connect(ui->lineeditSampleRate, SIGNAL(textChanged(const QString&)),
		this, SLOT(resetState()));

	triggerUpdater->setOffState(Stop);
	connect(triggerUpdater, SIGNAL(outputChanged(int)),
		this, SLOT(setTriggerState(int)));

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
	updateAreaTimeTrigger();

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

	chm_ui->setWidgetMinimumNrOfChars(ui->triggerStateLabel, 9);
	chm_ui->update_ui();
	init_export_settings();
	installWheelEventGuard();

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_LOGIC_ANALYZER)));
	api->load(*settings);
	api->js_register(engine);
}

void LogicAnalyzer::init_export_settings()
{
	exportSettings = new ExportSettings(this);
	exportSettings->enableExportButton(false);
	ui->verticalLayout_5->addWidget(exportSettings);
	for(int i = 0; i < no_channels; i++){
		exportSettings->addChannel(i, "DIO" + QString::number(i));
	}

	connect(exportSettings->getExportButton(), SIGNAL(clicked()), this,
		SLOT(btnExportPressed()));
	connect(this, &LogicAnalyzer::activateExportButton,
		[=](){
		exportSettings->enableExportButton(true);
	});

	exportSettings->disableUIMargins();
}


void LogicAnalyzer::configureMaxSampleRate()
{
	if(!offline_mode) {
		auto logic_analyzer_dev = iio_context_find_device(ctx,
			"m2k-logic-analyzer");
		if(!logic_analyzer_dev)
			return;
		long long maxSampling;
		int ret = iio_device_attr_read_longlong(logic_analyzer_dev,
			"sampling_frequency", &maxSampling);
		if(ret < 0)
			return;
		maxSamplingFrequency = maxSampling;
	}
}

LogicAnalyzer::~LogicAnalyzer()
{
	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	if(running)
		startStop(false);
	logic_analyzer_ptr.reset();
	context.reset();
	timer->stop();

	delete trigger_settings_ui;
	delete ui;

	/* Destroy libsigrokdecode */
	qDeleteAll(channel_groups_api);
	channel_groups_api.clear();
}

void LogicAnalyzer::set_buffersize()
{
	if(logic_analyzer_ptr) {
		main_win->view_->session().set_buffersize(logic_analyzer_ptr->get_buffersize());
		if(running) {
			//restart acquisition to recreate buffer
			main_win->restart_acquisition();
		}
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

void LogicAnalyzer::installWheelEventGuard()
{
	if(!wheelEventGuard)
		wheelEventGuard = new MouseWheelWidgetGuard(this);
	wheelEventGuard->installEventRecursively(this);
}

void LogicAnalyzer::resizeEvent()
{
	if(!initialised) {
		updateAreaTimeTrigger();
		timePosition->setValue(0);
		value_cursor1 = -(active_timePos - timeBase->value() * 2);
		value_cursor2 = -(active_timePos + timeBase->value() * 2);
		cursorValueChanged_1(timeToPixel(value_cursor1));
		cursorValueChanged_2(timeToPixel(value_cursor2));
		initialised = true;
	}

	int x1 = timeToPixel(value_cursor1 - active_timePos);
	d_hCursorHandle1->setPositionSilenty(x1);
	main_win->view_->viewport()->cursorValueChanged_1(x1);

	int x2 = timeToPixel(value_cursor2 - active_timePos);
	d_hCursorHandle2->setPositionSilenty(x2);
	main_win->view_->viewport()->cursorValueChanged_2(x2);

	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);
}

void LogicAnalyzer::updateAreaTimeTrigger()
{
	ui->areaTimeTriggerLayout->setContentsMargins(
		chm_ui->sizeHint().width()-20, 0, 0, 0);
}

QString LogicAnalyzer::saveToFile()
{
	QString separator = "";
	QString startRow = ";";
	QString endRow = "\n";
	QString selectedFilter;
	bool done = false;
	bool paused = false;
	bool noChannelEnabled = true;
	
	exportConfig = exportSettings->getExportConfig();
	for( auto x : exportConfig.keys() ) {
		if(exportConfig[x]) {
			noChannelEnabled =  false;
			break;
		}
	}

	if( noChannelEnabled )
		return "";

	if(running) {
		paused = true;
		startStop(false);
	}

	QString filename = QFileDialog::getSaveFileName(this,
		tr("Scopy Logic Analyzer export"), "",
		tr("Comma-separated values files (*.csv);;Tab-delimited values files(*.txt);;Value Change Dump(*.vcd);;All Files(*)"),
		&selectedFilter);

	if(filename.isEmpty())
		return "";

	// Check the selected file type
	if(selectedFilter != "") {
		if(selectedFilter.contains("comma", Qt::CaseInsensitive))
			separator = ",";
		if(selectedFilter.contains("tab", Qt::CaseInsensitive))
			separator = "\t";
		if(selectedFilter.contains("Change Dump", Qt::CaseInsensitive)) {
			endRow = " $end\n";
			startRow = "$";
		}
	}

	QFile file(filename);
	if( !file.open(QIODevice::WriteOnly)) {
		return "";
	}

	QTextStream out(&file);

	/* Write the general information */
	out << startRow << "date " << QDateTime::currentDateTime().toString() << endRow;
	out << startRow << "version Scopy - " << QString(SCOPY_VERSION_GIT) << endRow;
	out << startRow << "comment " << QString::number(main_win->session_.get_logic_sample_count()) <<
	       " samples acquired at " << QString::number(main_win->session_.get_samplerate()) <<
	       " Hz " << endRow;

	file.close();

	if( separator != "" )
		done = exportTabCsv(separator, filename);
	else
		done = exportVCD(filename, startRow, endRow);

	if(paused)
		startStop(true);

	return (done ? filename : "");
}

bool LogicAnalyzer::exportVCD(QString filename, QString startSep, QString endSep)
{
	uint64_t current_sample, prev_sample;
	int current_bit, prev_bit, p;
	QString timescaleFormat;
	double timescale;
	double timestamp;
	bool timestamp_written = false;

	QFile file(filename);
	if( !file.open(QIODevice::Append)) {
		return false;
	}

	QTextStream out(&file);

	/* Write the specific header */
	if( active_plot_timebase < 1e-6 ) {
		timescaleFormat = "ns";
		timescale = 1e-9;
	}
	else if( active_plot_timebase < 1e-3 ) {
		timescaleFormat = "us";
		timescale = 1e-6;
	}
	else if( active_plot_timebase < 1 ) {
		timescaleFormat = "ms";
		timescale = 1e-3;
	}
	else {
		timescaleFormat = "s";
		timescale = 1;
	}

	out << startSep << "timescale 1 " << timescaleFormat << endSep;
	out << startSep << "scope module Scopy" << endSep;
	int counter = 0;
	for(int ch = 0; ch < no_channels; ch++) {
		if( exportConfig[ch] ) {
			char c = '!' + counter;
			out << startSep << "var wire 1" << c << " DIO" <<
			       QString::number(ch) << endSep;
			counter++;
		}
	}
	out << startSep << "upscope" << endSep;
	out << startSep << "enddefinitions" << endSep;

	/* Write the values */
	std::shared_ptr<pv::data::Logic> logic_data = main_win->session_.get_logic_data();
	if(logic_data) {
		shared_ptr<pv::data::LogicSegment> segment = logic_data->logic_segments().front();
		uint64_t sample_count = segment->get_sample_count();
		for(int i = 0; i < sample_count; i++) {
			current_sample = segment->get_sample(i);
			if( i == 0)
				prev_sample = current_sample;
			else
				prev_sample = segment->get_sample(i-1);
			timestamp_written = false;
			p = 0;
			timestamp = ((i * active_plot_timebase * 10) / sample_count)/timescale;

			for(int ch = 0; ch < no_channels; ch++) {
				current_bit = (current_sample >> ch) & 1;
				prev_bit = (prev_sample >> ch) & 1;

				if((current_bit == prev_bit) && (i != 0))
					continue;

				if( !timestamp_written )
					out << "#" << QString::number(timestamp);

				if(exportConfig[ch]) {
					char c = '0' + current_bit;
					char c2 = '!' + p;
					out << ' ' << c << c2;
					p++;
					timestamp_written = true;
				}
			}
			if(timestamp_written)
				out << "\n";
		}
	}
	else {
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool LogicAnalyzer::exportTabCsv(QString separator, QString filename)
{
	QFile file(filename);
	if( !file.open(QIODevice::Append)) {
		return false;
	}

	QTextStream out(&file);

	// Write the header ( sample number + channels)
	for(int ch = 0; ch < no_channels; ch++) {
		if( exportConfig[ch] ) {
			out << "Channel " + QString::number(ch) +
			       ((ch == no_channels - 1) ? "\n" : separator);
		}
		else if( ch == no_channels - 1)
			out << "\n";
	}

	// Write the values
	std::shared_ptr<pv::data::Logic> logic_data = main_win->session_.get_logic_data();
	if(logic_data) {
		shared_ptr<pv::data::LogicSegment> segment = logic_data->logic_segments().front();
		for(int i = 0; i < segment->get_sample_count(); i++) {
			uint64_t sample = segment->get_sample(i);
			for(int ch = 0; ch < no_channels; ch++) {
				int bit = (sample >> ch) & 1;
				if(exportConfig[ch]) {
					out << (bit ? "1" : "0");
					out << ((ch == no_channels - 1) ? "\n" : separator);
				}
				else if( ch == no_channels - 1)
					out << "\n";
			}
		}
	}
	else {
		file.close();
		return false;
	}
	file.close();
	return true;
}

void LogicAnalyzer::btnExportPressed()
{
	if( !main_win->session_.is_data())
		return;

	QString filename = saveToFile();
}

void LogicAnalyzer::startTimer()
{
	if(!timer->isActive()) {
		timer->setSingleShot(true);
		timer->start(timer_timeout_ms);
	}
}

void LogicAnalyzer::triggerTimeout()
{
	if(armed) {
		trigger_is_forced = true;
		autoCaptureEnable(false);
	}
}

void LogicAnalyzer::startTimeout()
{
	Q_EMIT starttimeout();
}

void LogicAnalyzer::stopTimeout()
{
	Q_EMIT stoptimeout();
}

void LogicAnalyzer::stopTimer()
{
	if(trigger_settings_ui->btnAuto->isChecked()) {
		if(timer->isActive()) {
			timer->stop();
		}
		else {
			if(!armed)
				trigger_is_forced = true;
			autoCaptureEnable(true);
		}
		if(ui->btnSingleRun->isChecked())
			ui->btnSingleRun->setChecked(false);
	}
}

void LogicAnalyzer::setTriggerState(int triggerState)
{
	if(!trigger_settings_ui)
		return;

	ui->triggerStateLabel->hide();
	switch (triggerState) {
	case Waiting:
		ui->triggerStateLabel->setText("Waiting");
		break;
	case Triggered:
		ui->triggerStateLabel->setText("Triggered");
		break;
	case Stop:
		ui->triggerStateLabel->setText("Stop");
		break;
	case Auto:
		ui->triggerStateLabel->setText("Auto");
		break;
	case Stream:
		ui->triggerStateLabel->setText("Scan");
		break;
	default:
		break;
	};

	ui->triggerStateLabel->show();
}

void LogicAnalyzer::onTriggerModeChanged(bool val)
{
	if(!trigger_settings_ui)
		return;
	if(trigger_settings_ui->btnAuto->isChecked()) {
		triggerUpdater->setIdleState(Auto);
		triggerUpdater->setInput(Auto);
	}
	else {
		triggerUpdater->setIdleState(Waiting);
		triggerUpdater->setInput(Waiting);
	}
	if(acquisition_mode != REPEATED)
		triggerUpdater->setIdleState(Stream);
}

void LogicAnalyzer::onHorizScaleValueChanged(double value)
{
	zoomed_in = false;
	configParams(value, active_timePos);
}

void LogicAnalyzer::setSampleRate()
{
	if(!dev)
		return;

	if( acquisition_mode != REPEATED )
		validateSamplingFrequency();

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
	text = (acquisition_mode == REPEATED) ? text : "Streaming at " + txtSamplerate;
	ui->samplerateLabel->setText(text);
	ui->lineeditSampleRate->setText(QString::number(samplerate, 'g', 5));
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

void LogicAnalyzer::onRulerChanged(double ruler_value, bool silent)
{
	double timePos = ruler_value + active_plot_timebase * 10 / 2;
	if(!silent) {
		if(timePosition->value() != timePos)
			timePosition->setValue(timePos);
	}
	else {
		zoomed_in = true;
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

void LogicAnalyzer::configParams(double timebase, double timepos)
{

	timeBase->blockSignals(true);
	timePosition->blockSignals(true);

	timePosition->setValue(timepos);
	if(!zoomed_in)
		timeBase->setValue(timebase);

	timeBase->blockSignals(false);
	timePosition->blockSignals(false);

	symmBufferMode->setTriggerPos(-timepos);
	symmBufferMode->setTimeBase(timebase);
	LogicAnalyzerSymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();

        double plotTimeSpan = timebase * 10;
        timer_timeout_ms = plotTimeSpan * 1000  + 100; //transfer time

        acquisition_mode = ui->cmbRunMode->currentIndex();
        acquisition_mode = (acquisition_mode == REPEATED
                            && plotTimeSpan >= timespanLimitStream) ? SCREEN : acquisition_mode;

        main_win->session_.set_screen_mode(false);

        if(acquisition_mode == SCREEN || acquisition_mode == STREAM) {
                //Reset the triggered register
                iio_device_attr_write_bool(dev, "streaming", false);
                iio_device_attr_write_bool(dev, "streaming", true);
                if(active_plot_timebase != timebase)
                {
                        active_plot_timebase = timebase;
                        d_timeTriggerHandle->setPositionSilenty(0);
                        onTimeTriggerHandlePosChanged(0);
                }
                else if(active_timePos != -params.timePos)
                        active_timePos = -params.timePos;

                int trigX = timeToPixel(-active_timePos);
                d_timeTriggerHandle->setPositionSilenty(trigX);

                if( trigX != 0 ) {
                        if(chm_ui->is_streaming_mode()) {
                                chm_ui->set_streaming_mode(false);
                                chm_ui->update_ui();
                        }
                        acquisition_mode = REPEATED;
                        ui->cmbRunMode->blockSignals(true);
                        ui->cmbRunMode->setCurrentIndex(0);
                        ui->cmbRunMode->blockSignals(false);
                        setDynamicProperty(ui->lineeditSampleRate, "enabled", false);
                        ui->btnApply->setEnabled(false);
                }
                else {
                        if(!chm_ui->is_streaming_mode())
                                chm_ui->set_streaming_mode(true);

                        if( acquisition_mode == SCREEN)
                                main_win->session_.set_screen_mode(true);

                        setDynamicProperty(ui->lineeditSampleRate, "enabled", true);
                        ui->btnApply->setEnabled(true);

                        active_triggerSampleCount = 0;
                        custom_sampleCount = 4096;

			double bufferTimeSpan = custom_sampleCount / active_sampleRate;
			buffer_previewer->setNoOfSteps(plotTimeSpan / bufferTimeSpan + 1);
			active_sampleCount = (plotTimeSpan / bufferTimeSpan) * custom_sampleCount;

			timer_timeout_ms = bufferTimeSpan * 1000  + 100; //transfer time

			main_win->session_.set_entire_buffersize(active_sampleCount);
			if(logic_analyzer_ptr)
			{
				logic_analyzer_ptr->set_buffersize(custom_sampleCount, false);
				logic_analyzer_ptr->set_entire_buffersize(active_sampleCount);
				set_buffersize();
			}
			setSampleRate();
			if( running )
			{
				last_set_sample_count = custom_sampleCount;
				setBuffersizeLabelValue(active_sampleCount);
				setSamplerateLabelValue(active_sampleRate);
				setHWTriggerDelay(active_triggerSampleCount);
			}

			setTriggerDelay();
			d_timeTriggerHandle->setPosition(trigX);
			main_win->view_->viewport()->setTimeTriggerPixel(trigX);
			main_win->view_->time_item_appearance_changed(true, true);

			// Change the sensitivity of time position control
			timePosition->setStep(timebase / 10);
			recomputeCursorsValue(true);
			updateBufferPreviewer();
		}
        }
        if(acquisition_mode == REPEATED) {
                if(chm_ui->is_streaming_mode()) {
                        chm_ui->set_streaming_mode(false);
                        iio_device_attr_write_bool(dev, "streaming", false);
                        chm_ui->update_ui();
                }

                if(active_plot_timebase != timebase)
                        active_plot_timebase = timebase;
                else if(active_timePos != -params.timePos)
                        active_timePos = -params.timePos;

                setDynamicProperty(ui->lineeditSampleRate, "enabled", false);
                ui->btnApply->setEnabled(false);

                active_sampleRate = params.sampleRate;
                active_sampleCount = params.entireBufferSize;
                active_triggerSampleCount = -(long long)params.triggerBufferSize;
                buffer_previewer->setNoOfSteps(0);

                main_win->session_.set_entire_buffersize(0);
                if( logic_analyzer_ptr )
                {
                        if(logic_analyzer_ptr->get_buffersize() != active_sampleCount)
                        {
                                logic_analyzer_ptr->set_buffersize(active_sampleCount, true);
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

                // Change the sensitivity of time position control
                timePosition->setStep(timebase / 10);
                recomputeCursorsValue(true);
                updateBufferPreviewer();
        }
        onTriggerModeChanged(trigger_settings_ui->btnAuto->isChecked());
        logic_analyzer_ptr->set_stream(acquisition_mode == STREAM);
        main_win->view_->time_item_appearance_changed(true, true);
}

void LogicAnalyzer::onTimePositionSpinboxChanged(double value)
{
        configParams(active_plot_timebase, value);
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
	if(!dev)
		return;

	if (start) {
		//Reset the triggered register
		iio_device_attr_write_bool(dev, "streaming", false);
		if(acquisition_mode != REPEATED){
			iio_device_attr_write_bool(dev, "streaming", true);
		}
		buffer_previewer->setWaveformWidth(0);
		if(ui->btnSingleRun->isChecked()) {
			ui->btnSingleRun->setChecked(false);
		}
		last_set_sample_count = active_sampleCount;
		if(main_win->view_->scale() != timeBase->value()){
			zoomed_in = false;
			Q_EMIT timeBase->valueChanged(timeBase->value());
		}
		main_win->view_->viewport()->disableDrag();
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
		setSampleRate();
		running = true;
		ui->btnRunStop->setText("Stop");
		setHWTriggerDelay(active_triggerSampleCount);
		setTriggerDelay();
		if(!armed)
			autoCaptureEnable(true);
		updateBufferPreviewer();
	} else {
		main_win->view_->viewport()->enableDrag();
		running = false;
		ui->btnRunStop->setText("Run");
		if(timer->isActive()) {
			timer->stop();
		}
		if(!armed && trigger_settings_ui->btnAuto->isChecked()) {
			autoCaptureEnable(true);
		}
	}
	main_win->run_stop();
	Q_EMIT activateExportButton();

	triggerUpdater->setEnabled(start);
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
	if(!dev)
		return;
	std::string name = "voltage0";
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	QString s = QString::number(delay);
	iio_channel_attr_write(triggerch, "trigger_delay",
		s.toLocal8Bit().QByteArray::constData());
	active_hw_trigger_sample_count = delay;
}

void LogicAnalyzer::singleRun(bool checked)
{
	if(checked) {
		if(!armed)
			autoCaptureEnable(true);
		ui->btnSingleRun->setText("Stop");
		buffer_previewer->setWaveformWidth(0);
		if(!dev)
			return;
		if( running )
		{
			ui->btnRunStop->setChecked(false);
		}
		last_set_sample_count = active_sampleCount;
		if(main_win->view_->scale() != timeBase->value()){
			zoomed_in = false;
			Q_EMIT timeBase->valueChanged(timeBase->value());
		}
		setSampleRate();
		running = true;
		triggerUpdater->setEnabled(running);
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
		setHWTriggerDelay(active_triggerSampleCount);
		setTriggerDelay();
		//	if (timePosition->value() != active_timePos)
		//		timePosition->setValue(active_timePos);
		logic_analyzer_ptr->set_single(true);
		main_win->run_stop();
		running = false;
		updateBufferPreviewer();
	}
	else {
		ui->btnSingleRun->setText("Single");
		if(logic_analyzer_ptr->get_single()
				&& logic_analyzer_ptr->is_running()) {
			main_win->run_stop();
		}
		running = false;
		if(timer->isActive())
			timer->stop();
		if(!armed && trigger_settings_ui->btnAuto->isChecked()) {
			autoCaptureEnable(true);
		}
		Q_EMIT activateExportButton();
		triggerUpdater->setEnabled(running);
	}
}

unsigned int LogicAnalyzer::get_no_channels(struct iio_device *dev)
{
	if(!dev)
		return 0;
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
	if(!dev)
		return;
	std::string name = "voltage" + to_string(chid);
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);

	if( !triggerch )
		return;

	iio_channel_attr_write(triggerch, "trigger", trigger_val.c_str());
}

std::string LogicAnalyzer::get_trigger_from_device(int chid)
{
	if(!dev)
		"none";
	std::string name = "voltage" + to_string(chid);
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	if( !triggerch )
		return "";
	char trigger_val[4096];
	iio_channel_attr_read(triggerch, "trigger", trigger_val, sizeof(trigger_val));
	string res(trigger_val);
	return res;
}

std::vector<std::string> LogicAnalyzer::get_iio_trigger_options()
{
	char buf[1024];
	std::vector<std::string> values;

	if(!dev)
		return trigger_mapping;
	std::string name = "voltage0";
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	if( !triggerch )
		return trigger_mapping;
	int ret = iio_channel_attr_read(triggerch, "trigger_available", buf, sizeof(buf));

	if (ret > 0) {
		QStringList list = QString::fromUtf8(buf).split(' ');

		for (auto it = list.cbegin(); it != list.cend(); ++it) {
			values.push_back(it->toStdString());
		}
	}

	if (values.empty()) {
		values = trigger_mapping;
	}
	return values;
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
		ui->areaTimeTriggerLayout->setContentsMargins(value-20, 0, 0, 0);
		timePosition->valueChanged(timePosition->value());
	}
}

void LogicAnalyzer::setHWTriggerLogic(const QString value)
{
	if(!dev)
		return;
	std::string name = "voltage0";
	struct iio_channel *triggerch = iio_device_find_channel(dev, name.c_str(), false);
	QString s = value.toLower();
	iio_channel_attr_write(triggerch, "trigger_logic_mode",
		s.toLocal8Bit().QByteArray::constData());
}

void LogicAnalyzer::requestUpdateBufferPreviewer()
{
	if(acquisition_mode != REPEATED)
		updateBufferPreviewer();
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
	long long bufferSamples = (acquisition_mode == REPEATED) ? 0 : custom_sampleCount;

	if(totalSamples > 0) {
		dataMin = triggerSamples / active_hw_sampleRate;
		dataMax = (triggerSamples + totalSamples + bufferSamples) / active_hw_sampleRate;
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
	if(acquisition_mode != SCREEN) {
		buffer_previewer->setWaveformWidth(wWidth);
		buffer_previewer->setWaveformPos(wPos);
		buffer_previewer->setHighlightWidth(hWidth);
		buffer_previewer->setHighlightPos(hPos);
		buffer_previewer->setCursorPos(cPos);
	}
	if(acquisition_mode == SCREEN) {
		buffer_previewer->setWaveformPos(wPos);
		buffer_previewer->setHighlightWidth(hWidth);
		buffer_previewer->setHighlightPos(hPos);
		buffer_previewer->setCursorPos(0);
	}

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
		cleanTrigger();
	}
}

void LogicAnalyzer::cleanTrigger()
{
	for(int i = 0; i < get_no_channels(dev) + 2; i++) {
		setHWTrigger(i, trigger_mapping[0]);
		if(i < get_no_channels(dev))
			chm.get_channel(i)->setTrigger(trigger_mapping[0]);
	}
	chm_ui->update_ui();
}

void LogicAnalyzer::bufferSentSignal(bool lastBuffer)
{
	if(acquisition_mode == REPEATED)
		return;
	if(!lastBuffer) {
		double div =1 / buffer_previewer->noOfSteps();
		double val = buffer_previewer->waveformWidth() + div;
		buffer_previewer->setWaveformWidth(val);
	}
	else {
		buffer_previewer->setWaveformWidth(0);
	}
}

void LogicAnalyzer::autoCaptureEnable(bool check)
{
	if(check) {
		for(int i = 0; i < get_no_channels(dev) + 2; i++) {
			setHWTrigger(i, trigger_cache[i]);
		}
	}
	if(!check && armed){
		for(int i = 0; i < get_no_channels(dev) + 2; i++) {
			trigger_cache[i] = get_trigger_from_device(i);
			setHWTrigger(i, trigger_mapping[0]);
		}
	}
	armed = check;
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
		int pairPos = timeToPixel(value_cursor2 - active_timePos);
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
		int pairPos = timeToPixel(value_cursor1-active_timePos);
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
	value_cursor2 = -(pixelToTime(x2) + active_plot_timebase * 10 / 2 - active_timePos);
	value_cursor1 = -(pixelToTime(x1) + active_plot_timebase * 10 / 2 - active_timePos);
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
	cleanHWParams();
	chm.clearTrigger();
	for(int i = 0; i < no_channels; i++) {
		chm.add_channel_group(new LogicAnalyzerChannelGroup(chm.get_channel(i)));
	}
	chm.highlightChannel(chm.get_channel_group(0));
	chm_ui->update_ui();
	timePosition->setValue(0);
	timeBase->setValue(1e-3);
	setSampleRate();
	setHWTriggerDelay(active_triggerSampleCount);
	setTriggerDelay();
	ui->boxCursors->setChecked(false);
	if(ui->btnCursorsLock->isChecked())
		ui->btnCursorsLock->toggle();
	ui->cursorSettings->update();
	ui->btnShowChannels->clicked(false);
	exportSettings->getExportAllButton()->setChecked(true);
	initialised = false;
	main_win->session_.clear_data();
	updateBufferPreviewer();
}

void LogicAnalyzer::setTimeout(bool checked)
{
	if(!armed) {
		autoCaptureEnable(true);
	}
	logic_analyzer_ptr->set_timeout(checked);
	if(checked)
		startTimer();
	else if(timer->isActive())
		timer->stop();
}

void LogicAnalyzer::runModeChanged(int index)
{
        bool en;

        switch(index){
        case REPEATED:
        {
                acquisition_mode = REPEATED;
                if( logic_analyzer_ptr )
                        logic_analyzer_ptr->set_stream(false);

                setDynamicProperty(ui->lineeditSampleRate, "enabled", false);
                ui->btnApply->setEnabled(false);

                main_win->session_.set_screen_mode(false);
                en = false;
                if(timeBase->value() * 10 >= timespanLimitStream) {
                        d_timeTriggerHandle->setPosition(0);
                        acquisition_mode = SCREEN;
                        main_win->session_.set_screen_mode(true);
                        setDynamicProperty(ui->lineeditSampleRate, "enabled", true);
                        ui->btnApply->setEnabled(true);
                        en = true;
                }
        }
                break;
        case STREAM:
        {
                if( logic_analyzer_ptr )
                        logic_analyzer_ptr->set_stream(true);
                d_timeTriggerHandle->setPosition(0);
                acquisition_mode = STREAM;
                main_win->session_.set_screen_mode(false);
                setDynamicProperty(ui->lineeditSampleRate, "enabled", true);
                ui->btnApply->setEnabled(true);
                en = true;
        }
                break;
        default:break;
        }
        chm_ui->set_streaming_mode(en);
}

void LogicAnalyzer::validateSamplingFrequency()
{
	bool ok;
	double srDivider =  0;
	double samplingFreq = ui->lineeditSampleRate->text().toDouble(&ok);
	if(ok) {
		if( samplingFreq < 10 ) {
			active_sampleRate = 10;
			setDynamicProperty(ui->lineeditSampleRate, "invalid", true);
			goto validate;
		}
		if( samplingFreq > 3e+6 ) {
			samplingFreq = 3e+6;
			setDynamicProperty(ui->lineeditSampleRate, "invalid", true);
		}

		srDivider = maxSamplingFrequency / samplingFreq;
		srDivider = round(srDivider);
		srDivider = (srDivider == 0.0) ? srDivider + 1 : srDivider;

		samplingFreq = maxSamplingFrequency / srDivider;
		if( samplingFreq == active_sampleRate ) {
			setDynamicProperty(ui->lineeditSampleRate, "valid", true);
			setDynamicProperty(ui->btnApply, "valid", true);
			return;
		}
		active_sampleRate = samplingFreq;

		active_sampleRate = maxSamplingFrequency / srDivider;
validate:
		ui->lineeditSampleRate->setText(QString::number(active_sampleRate));
		onHorizScaleValueChanged(timeBase->value());
		if(running)
			setSamplerateLabelValue(active_sampleRate);

		setDynamicProperty(ui->lineeditSampleRate, "valid", true);
		setDynamicProperty(ui->btnApply, "valid", true);
	}
	else {
		setDynamicProperty(ui->lineeditSampleRate, "invalid", true);
		setDynamicProperty(ui->btnApply, "invalid", true);
	}
}

void LogicAnalyzer::resetState()
{
	setDynamicProperty(ui->lineeditSampleRate, "invalid", false);
	setDynamicProperty(ui->btnApply, "invalid", false);
	setDynamicProperty(ui->lineeditSampleRate, "valid", false);
	setDynamicProperty(ui->btnApply, "valid", false);
}

void LogicAnalyzer::onDataReceived()
{
	bool new_data;
	if(!trigger_settings_ui)
		return;
	if(armed && !trigger_is_forced)
		new_data = true;
	else
		new_data = false;

	trigger_is_forced = false;

	if(new_data) {
		triggerUpdater->setInput(Triggered);
	}
	else {
		if(trigger_settings_ui->btnAuto->isChecked())
			triggerUpdater->setInput(Auto);
	}

	/* Single shot */
	if( !running )
		triggerUpdater->setEnabled(running);
}

void LogicAnalyzer::onFrameEnded()
{
	/* Reset the Single button state when the frame ended
	 * and data was received */
	if(ui->btnSingleRun->isChecked() && main_win->session_.is_data())
		ui->btnSingleRun->setChecked(false);
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

QVariantList LogicAnalyzer_API::getChannelGroups()
{
	QVariantList list;
	for(ChannelGroup_API *each : lga->channel_groups_api)
		list.append(QVariant::fromValue(each));
	return list;
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

QString LogicAnalyzer_API::runMode() const
{
	if(lga->acquisition_mode == 0)
	{
		return "REPEATED";
	}
	else if(lga->acquisition_mode == 1)
	{
		return "STREAM";
	}
	else
	{
		return "SCREEN";
	}
}

void LogicAnalyzer_API::setRunMode(QString value)
{
	if(value == "STREAM")
	{
		lga->ui->cmbRunMode->setCurrentIndex(1);
	}
	else
	{
		lga->ui->cmbRunMode->setCurrentIndex(0);
	}
}

bool LogicAnalyzer_API::getExportAll() const
{
	return lga->exportSettings->getExportAllButton()->isChecked();
}

void LogicAnalyzer_API::setExportAll(bool en)
{
	lga->exportSettings->getExportAllButton()->setChecked(en);
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
	auto chGroupUI = lga->chm_ui->getUiFromChGroup(
		lga->chm.get_channel_group(getIndex()));
	if(chGroupUI) {
		chGroupUI->ui->btnEnableChannel->setChecked(en);
	}
	else
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

QVariantList ChannelGroup_API::getChannels()
{
	QVariantList list;

	for (LogicChannel_API *each : channels_api)
		list.append(QVariant::fromValue(each));

	return list;
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

QString ChannelGroup_API::getDecoder() const
{
	if(lga->chm.get_channel_group(this->getIndex())->getDecoder())
		return QString::fromUtf8(lga->chm.get_channel_group(
			this->getIndex())->getDecoder()->name);
	return "";
}

void ChannelGroup_API::setDecoder(QString val)
{
	lga->chm.get_channel_group(getIndex())->setDecoder(
		lga->chm.get_decoder_from_name(val.toUtf8()));
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
	lga->chm.get_channel_group(lchg->getIndex())->add_logic_channel(
		lga->chm.get_channel(val));
}

QString LogicChannel_API::getRole() const
{
	if(lga->chm.get_channel_group(lchg->getIndex())->is_grouped()) {
		auto ch = lga->chm.get_channel_group(lchg->getIndex())->get_channel_by_id(
			getIndex())->getChannel_role();
		if(ch) {
			return ch->name;
		}
	}
	return "";
}

void LogicChannel_API::setRole(QString val)
{
	auto ch = lga->chm.get_channel_group(
		lchg->getIndex())->get_srd_channel_from_name(val.toUtf8());
	lga->chm.get_channel_group(lchg->getIndex())->get_channel_by_id(
		getIndex())->setChannel_role(ch);
}
