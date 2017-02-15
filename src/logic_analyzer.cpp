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

LogicAnalyzer::LogicAnalyzer(struct iio_context *ctx,
                             Filter *filt,
                             QPushButton *runBtn,
                             QWidget *parent) :
	QWidget(parent),
	dev_name(filt->device_name(TOOL_LOGIC_ANALYZER)),
	ctx(ctx),
	itemsize(sizeof(uint16_t)),
	dev(iio_context_find_device(ctx, dev_name.c_str())),
	menuOpened(true),
	settings_group(new QButtonGroup(this)),
	menuRunButton(runBtn),
	ui(new Ui::LogicAnalyzer),
	active_settings_btn(nullptr),
	timespanLimitStream(1),
	plotRefreshRate(100),
	active_sampleRate(0.0),
	active_sampleCount(0),
	active_triggerSampleCount(0),
	active_timePos(0)
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

	QVBoxLayout *vLayout = new QVBoxLayout(ui->generalSettings);
	vLayout->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	vLayout->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	vLayout->insertSpacerItem(-1, new QSpacerItem(0, 0,
	                          QSizePolicy::Minimum,
	                          QSizePolicy::Expanding));
	ui->generalSettings->setLayout(vLayout);

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

	timePosition->setValue(0);
	timePosition->valueChanged(timePosition->value());
	main_win->view_->viewport()->setTimeTriggerSample(-active_triggerSampleCount);
}


LogicAnalyzer::~LogicAnalyzer()
{
	delete chm_ui;
	delete ui;

	/* Destroy libsigrokdecode */
	srd_exit();
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

void LogicAnalyzer::set_triggered_status(bool value)
{
	if( !value )
		ui->triggerStateLabel->setText("Waiting");
	else
		ui->triggerStateLabel->setText("Trig'd");
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

	// Change the sensitivity of time position control
	timePosition->setStep(value / 10);
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
	if(!silent)
		if(timePosition->value() != timePos)
			timePosition->setValue(timePos);
	else{
		active_plot_timebase = main_win->view_->scale();
		int pix = timeToPixel(-timePos);
		if( pix != d_timeTriggerHandle->position() )
		{
			d_timeTriggerHandle->setPositionSilenty(pix);
		}
	}
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
	if( pix != d_timeTriggerHandle->position() )
	{
		d_timeTriggerHandle->setPosition(pix);
	}
	if( running )
	{
		setHWTriggerDelay(active_triggerSampleCount);
		setSampleRate();
		setBuffersizeLabelValue(active_sampleCount);
		setSamplerateLabelValue(active_sampleRate);
	}
	setTriggerDelay();
}

void LogicAnalyzer::onTimeTriggerHandlePosChanged(int pos)
{
	int width = bottomHandlesArea()->geometry().width() -
			d_bottomHandlesArea->leftPadding() -
			d_bottomHandlesArea->rightPadding();
	double time = pixelToTime(pos);
	if( (time + active_plot_timebase * 10 / 2) != active_plot_timebase )
	{
		timePosition->setValue(time + active_plot_timebase * 10 / 2);
	}
	if(running)
		main_win->view_->viewport()->setTimeTriggerSample(
			-active_triggerSampleCount);
	else {
		setTriggerDelay();
	}
	main_win->view_->set_offset(timePosition->value(), active_plot_timebase * 10, running);
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
		setHWTriggerDelay(active_triggerSampleCount);
		setTriggerDelay();
		main_win->view_->viewport()->setTimeTriggerSample(
			-active_triggerSampleCount);
		if (timePosition->value() != active_timePos)
			timePosition->setValue(active_timePos);
	} else {
		running = false;
		ui->btnRunStop->setText("Run");
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
	bool open = true; //= !menuOpened;

	settings_panel_update(id);

	if (active_settings_btn == btn) {
		open = !menuOpened;
		ui->rightWidget->toggleMenu(false);
	} else {
		open = true;
	}

	if (!open) {
		settings_group->setExclusive(false);
		ui->btnChSettings->setChecked(false);
		ui->btnSettings->setChecked(false);
		settings_group->setExclusive(true);
	}

//	if (menuOpened != open) {
//		ui->rightWidget->toggleMenu(open);
//	}

	menuOpened = open;
	active_settings_btn = btn;

	if (open) {
		chm_ui->showHighlight(true);
	} else {
		chm_ui->showHighlight(false);
	}
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

void LogicAnalyzer::set_trigger_to_device(int chid, std::string trigger_val)
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
	updateAreaTimeTriggerPadding();
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
		ui->btnShowHideMenu->setText(">");
		chm_ui->collapse(true);
	} else {
		ui->btnGroupChannels->show();
		ui->btnShowChannels->show();
		ui->btnShowHideMenu->setText("<");
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
