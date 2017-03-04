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
	timespanLimitStream(1),
	plotRefreshRate(100),
	active_sampleRate(0.0),
	active_sampleCount(0),
	active_triggerSampleCount(0),
	active_timePos(0),
	trigger_settings(new QWidget(this)),
	value_cursor1(-0.033),
	value_cursor2(0.033),
	la_api(new LogicAnalyzer_API(this))
{
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	iio_context_set_timeout(ctx, UINT_MAX);

	symmBufferMode = make_shared<LogicAnalyzerSymmetricBufferMode>();
	symmBufferMode->setMaxSampleRate(80000000);
	symmBufferMode->setEntireBufferMaxSize(500000); // max 0.5 mega-samples
	symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
	symmBufferMode->setTimeDivisionCount(10);

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
	std::shared_ptr<sigrok::Context> context;
	std::string open_file, open_file_format;
	context = sigrok::Context::create();

	/* Initialise libsigrokdecode */
	QString path = QCoreApplication::applicationDirPath() + "/decoders";
	if (srd_init(path.toStdString().c_str()) != SRD_OK) {
		qDebug() << "ERROR: libsigrokdecode init failed.";
	}

	/* Load the protocol decoders */
	srd_decoder_load_all();

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

	ui->generalSettingsLayout->insertWidget(ui->generalSettingsLayout->count() - 2,
		timeBase, 0, Qt::AlignLeft);
	ui->generalSettingsLayout->insertWidget(ui->generalSettingsLayout->count() - 1,
		timePosition, 0, Qt::AlignLeft);

	options["numchannels"] = Glib::Variant<gint32>(
			g_variant_new_int32(no_channels),true);
	logic_analyzer_ptr = std::make_shared<pv::devices::BinaryStream>(
	                             device_manager.context(), dev, maxBuffersize,
	                             w->get_format_from_string("binary"),
	                             options, this);

	/* setup view */
	main_win = w;
	main_win->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->centralWidgetLayout->addWidget(static_cast<QWidget * >(main_win));
	main_win->select_device(logic_analyzer_ptr);

	main_win->session_.set_buffersize(maxBuffersize);
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

	trigger_settings_ui = new Ui::DigitalTriggerSettings;
	trigger_settings_ui->setupUi(trigger_settings);
	ui->triggerSettingsLayout->insertWidget(0, trigger_settings);
	setupTriggerSettingsUI();

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

	cleanHWParams();
	chm_ui = new LogicAnalyzerChannelManagerUI(0, main_win, &chm, ui->colorSettings,
	                this);
	ui->leftLayout->addWidget(chm_ui);
	chm_ui->update_ui();
	chm_ui->setVisible(true);
	lachannelsettings = new Ui::LChannelSettings;

	connect(ui->btnGroupChannels, SIGNAL(pressed()),
	        chm_ui, SLOT(on_groupSplit_clicked()));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        chm_ui, SLOT(on_hideInactive_clicked(bool)));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        this, SLOT(on_btnShowChannelsClicked(bool)));
	connect(chm_ui, SIGNAL(widthChanged(int)),
		this, SLOT(onChmWidthChanged(int)));

	main_win->view_->viewport()->setTimeTriggerPosActive(true);
	ui->areaTimeTriggerLayout->addWidget(this->bottomHandlesArea(), 0, 1, 1, 3);
	updateAreaTimeTriggerPadding();
	ui->triggerStateLabel->setText("Stop");

	this->ensurePolished();
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
	d_hCursorHandle1->setPosition(main_win->view_->viewport()->width() / 3);
	d_hCursorHandle2->setPosition(2 * main_win->view_->viewport()->width() / 3);

	la_api->load();
	la_api->js_register(engine);
}

LogicAnalyzer::~LogicAnalyzer()
{
	delete chm_ui;

	la_api->save();
	delete la_api;

	delete ui;

	/* Destroy libsigrokdecode */
	srd_exit();
}

void LogicAnalyzer::resizeEvent()
{
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

void LogicAnalyzer::set_triggered_status(std::string value)
{
	if( value == "awaiting" )
		ui->triggerStateLabel->setText("Waiting");
	else if(value == "running")
		ui->triggerStateLabel->setText("Trig'd");
	else if(value == "stopped")
		ui->triggerStateLabel->setText("Stop");
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

	custom_sampleCount = maxBuffersize / plotRefreshRate;

	enableTrigger(true);
	if( plotTimeSpan >= timespanLimitStream )
	{
		logic_analyzer_ptr->set_buffersize(custom_sampleCount);
		enableTrigger(false);
		active_triggerSampleCount = 0;
	}
	else if( logic_analyzer_ptr )
	{
		logic_analyzer_ptr->set_buffersize(active_sampleCount);
		main_win->session_.set_buffersize(active_sampleCount);
	}

	if( running )
	{
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
	cursorValueChanged_1(d_hCursorHandle1->position());
	cursorValueChanged_2(d_hCursorHandle2->position());
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

	if( logic_analyzer_ptr )
	{
		logic_analyzer_ptr->set_options(options);
	}

	/* Set IIO device parameters */
	iio_device_attr_write_longlong(dev, "sampling_frequency", active_sampleRate);
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
			cursorValueChanged_1(d_hCursorHandle1->position());
			cursorValueChanged_2(d_hCursorHandle2->position());
		}
		else {
			setTimebaseLabel(active_plot_timebase);
		}
	}

	int trigX = timeToPixel(-active_timePos);
	d_timeTriggerHandle->setPositionSilenty(trigX);
	main_win->view_->viewport()->setTimeTriggerPixel(trigX);
	main_win->view_->time_item_appearance_changed(true, true);
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
	if( running )
	{
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
	cursorValueChanged_1(d_hCursorHandle1->position());
	cursorValueChanged_2(d_hCursorHandle2->position());
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
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
		running = true;
		setSampleRate();
		ui->btnRunStop->setText("Stop");
		ui->btnSingleRun->setEnabled(false);
		setHWTriggerDelay(active_triggerSampleCount);
		setTriggerDelay();
		if (timePosition->value() != active_timePos)
			timePosition->setValue(active_timePos);
	} else {
		running = false;
		ui->btnRunStop->setText("Run");
		ui->btnSingleRun->setEnabled(true);
		ui->triggerStateLabel->setText("Stop");
	}
	main_win->run_stop();
	setTriggerDelay();
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
}

void LogicAnalyzer::singleRun()
{
	if( running )
	{
		startStop(false);
		ui->btnRunStop->setChecked(false);
	}
	setSampleRate();
	logic_analyzer_ptr->set_single(true);
	main_win->run_stop();
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
		}
		chm_ui->update_ui();
	}
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
	}
	QString text = d_cursorTimeFormatter.format(value_cursor1, "", 3);
	ui->lblCursor1->setText(text);
	main_win->view_->viewport()->cursorValueChanged_1(pos);
	cursorsFormatDelta();
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
	}
	QString text = d_cursorTimeFormatter.format(value_cursor2, "", 3);
	ui->lblCursor2->setText(text);
	main_win->view_->viewport()->cursorValueChanged_2(pos);
	cursorsFormatDelta();
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

bool LogicAnalyzer_API::running() const
{
	return lga->ui->btnRunStop->isChecked();
}

void LogicAnalyzer_API::run(bool en)
{
	lga->ui->btnRunStop->setChecked(en);
}
