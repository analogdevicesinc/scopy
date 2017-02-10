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

/* GNU Radio includes */
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/iio/math.h>

/* Qt includes */
#include <QtWidgets>
#include <QDebug>
#include <QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QSignalBlocker>

/* Local includes */
#include "adc_sample_conv.hpp"
#include "math.hpp"
#include "oscilloscope.hpp"
#include "dynamicWidget.hpp"
#include "measurement_gui.h"
#include "measure_settings.h"
#include "statistic_widget.h"
#include "state_updater.h"
#include "osc_capture_params.hpp"
#include "buffer_previewer.hpp"

/* Generated UI */
#include "ui_math_panel.h"
#include "ui_channel.h"
#include "ui_channel_math.h"
#include "ui_channel_settings.h"
#include "ui_cursors_settings.h"
#include "ui_osc_general_settings.h"
#include "ui_measure_panel.h"
#include "ui_measure_settings.h"
#include "ui_statistics_panel.h"
#include "ui_cursor_readouts.h"
#include "ui_oscilloscope.h"
#include "ui_trigger.h"
#include "ui_trigger_settings.h"

using namespace adiscope;
using namespace gr;
using namespace std;

Oscilloscope::Oscilloscope(struct iio_context *ctx,
		Filter *filt, QPushButton *runButton, QJSEngine *engine,
		float gain_ch1, float gain_ch2, QWidget *parent) :
	QWidget(parent),
	adc(ctx, filt),
	nb_channels(Oscilloscope::adc.numChannels()),
	sampling_rates(adc.availSamplRates()),
	active_sample_rate(adc.sampleRate()),
	nb_math_channels(0),
	ui(new Ui::Oscilloscope),
	trigger_settings(ctx, adc),
	measure_settings(nullptr),
	plot(parent, 16, 10),
	fft_plot(nb_channels, parent),
	xy_plot(nb_channels / 2, parent),
	hist_plot(nb_channels, parent),
	ids(new iio_manager::port_id[nb_channels]),
	fft_ids(new iio_manager::port_id[nb_channels]),
	hist_ids(new iio_manager::port_id[nb_channels]),
	xy_ids(new iio_manager::port_id[nb_channels & ~1]),
	fft_is_visible(false), hist_is_visible(false), xy_is_visible(false),
	statistics_enabled(false),
	trigger_is_forced(false),
	new_data_is_triggered(false),
	triggerUpdater(new StateUpdater(250, this)),
	selectedChannel(-1),
	menuOpened(false), current_channel(0), math_chn_counter(0),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	active_settings_btn(nullptr),
	last_non_general_settings_btn(nullptr),
	menuRunButton(runButton), osc_api(new Oscilloscope_API(this))
{
	ui->setupUi(this);
	int triggers_panel = ui->stackedWidget->insertWidget(-1, &trigger_settings);
	settings_group->setExclusive(true);
	symmBufferMode = make_shared<SymmetricBufferMode>();
	symmBufferMode->setSampleRates(adc.availSamplRates().toVector().toStdVector());
	symmBufferMode->setEntireBufferMaxSize(500000); // max 0.5 mega-samples
	symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
	symmBufferMode->setTimeDivisionCount(plot.xAxisNumDiv());

	adc.setChannelGain(0, gain_ch1);
	adc.setChannelGain(1, gain_ch2);

	/* Measurements Settings */
	measure_settings_init();

	fft_size = 1024;
	last_set_sample_count = 0;
	last_set_time_pos = 0;

	/* Gnuradio Blocks */

	this->qt_time_block = adiscope::scope_sink_f::make(0, adc.sampleRate(),
		"Osc Time", nb_channels, (QObject *)&plot);

	this->qt_fft_block = adiscope::scope_sink_f::make(fft_size, adc.sampleRate(),
			"Osc Frequency", nb_channels, (QObject *)&fft_plot);

	this->qt_hist_block = adiscope::histogram_sink_f::make(1024, 100, 0, 20,
			"Osc Histogram", nb_channels, (QObject *)&hist_plot);

	this->qt_xy_block = adiscope::xy_sink_c::make(
			400, "Osc XY", nb_channels / 2, (QObject*)&xy_plot);

	this->qt_time_block->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");
	this->qt_fft_block->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	// Prevent the application from hanging while waiting for a trigger condition
	iio_context_set_timeout(ctx, UINT_MAX);

	plot.registerSink(qt_time_block->name(), nb_channels, 0);
	plot.disableLegend();

	iio = iio_manager::get_instance(ctx,
			filt->device_name(TOOL_OSCILLOSCOPE));
	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug() << "Manager created:\n" << gr::dot_graph(hier).c_str();

	struct iio_device *dev = adc.iio_adc();
	unsigned int chIdx = 0;
	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (iio_channel_is_output(chn) ||
				!iio_channel_is_scan_element(chn))
			continue;

		const char *id = iio_channel_get_name(chn);
		string s = "Channel ";
		if (!id) {
			s += to_string(chIdx + 1);
			id = s.c_str();
		}

		QWidget *ch_widget = new QWidget(this);
		Ui::Channel channel_ui;

		channel_ui.setupUi(ch_widget);
		channel_ui.name->setText(id);

		QString stylesheet(channel_ui.box->styleSheet());
		stylesheet += QString("\nQCheckBox::indicator {\nborder-color: %1;\n}\nQCheckBox::indicator:checked {\nbackground-color: %1;\n}\n"
				).arg(plot.getLineColor(chIdx).name());

		channel_ui.box->setStyleSheet(stylesheet);

		channel_ui.box->setProperty("id", QVariant(chIdx));
		channel_ui.btn->setProperty("id", QVariant(chIdx));
		channel_ui.name->setProperty("id", QVariant(chIdx));

		connect(channel_ui.box, SIGNAL(toggled(bool)), this,
				SLOT(channel_box_toggled(bool)));
		connect(channel_ui.name, SIGNAL(toggled(bool)),
				SLOT(channel_name_checked(bool)));

		/* For now, we only have one menu - only the channel1 options
		 * icon can open it */
		connect(channel_ui.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

		ui->channelsList->addWidget(ch_widget);

		settings_group->addButton(channel_ui.btn);
		channels_group->addButton(channel_ui.name);

		chIdx++;
	}

	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	/* Cursors Settings */
	QWidget *cursor_widget = new QWidget(this);
	cursor_ui = new Ui::Channel();
	cursor_ui->setupUi(cursor_widget);
	ui->cursors_settings->addWidget(cursor_widget);

	settings_group->addButton(cursor_ui->btn);
	cursor_ui->btn->setProperty("id", QVariant(-1));
	cursor_ui->name->setText("Cursors");
	cursor_ui->box->setChecked(false);
	QString stylesheet(cursor_ui->box->styleSheet());
	stylesheet += QString("\nQCheckBox::indicator {"
				"border-color: rgb(74, 100, 255);"
				"border-radius: 4px;"
				"}"
				"QCheckBox::indicator:checked {"
				"background-color: rgb(74, 100, 255);"
				"}");
	cursor_ui->box->setStyleSheet(stylesheet);
	connect(cursor_ui->btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));
	connect(cursor_ui->box, SIGNAL(toggled(bool)), this,
			SLOT(onCursorsToggled(bool)));


	/* Trigger Settings */
	QWidget *trig_widget = new QWidget(this);
	Ui::Trigger trig_ui;

	trig_ui.setupUi(trig_widget);
	ui->trigger_settings->addWidget(trig_widget);
	settings_group->addButton(trig_ui.btn);
	trig_ui.btn->setProperty("id", QVariant(-triggers_panel));
	connect(trig_ui.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

	/* Trigger Status Updater */
	triggerUpdater->setOffState(CapturePlot::Stop);
	onTriggerModeChanged(trigger_settings.triggerMode());
	connect(triggerUpdater, SIGNAL(outputChanged(int)),
		&plot, SLOT(setTriggerState(int)));

	plot.setZoomerEnabled(true);

	create_math_panel();

	/* Gnuradio Blocks Connections */

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	auto adc_samp_conv = gnuradio::get_initial_sptr(
			new adc_sample_conv(nb_channels));
	adc_samp_conv->setCorrectionGain(0, gain_ch1);
	adc_samp_conv->setCorrectionGain(1, gain_ch2);

	for (unsigned int i = 0; i < nb_channels; i++) {
		ids[i] = iio->connect(adc_samp_conv, i, i,
				true, qt_time_block->nsamps());

		iio->connect(adc_samp_conv, i, qt_time_block, i);
	}

	adc_samp_conv_block = adc_samp_conv;

	if (started)
		iio->unlock();

	/* Measure panel */
	measure_panel_init();

	/* Statistics panel */
	statistics_panel_init();

	/* Buffer Previewer widget */
	buffer_previewer = new AnalogBufferPreviewer(40, M_PI / 2);

	buffer_previewer->setVerticalSpacing(6);
	buffer_previewer->setMinimumHeight(20);
	buffer_previewer->setMaximumHeight(20);
	buffer_previewer->setMinimumWidth(480);
	buffer_previewer->setMaximumWidth(480);
	buffer_previewer->setHighlightBgColor(QColor("#141416"));
	buffer_previewer->setHighlightFgColor(QColor("#ff7200"));
	buffer_previewer->setCursorColor(QColor("#4A64FF"));
	stylesheet = "adiscope--BufferPreviewer {"
			"background-color: #272730;"
			"color: #ffffff;"
			"border: 1px solid #7092be;"
		"}";
	buffer_previewer->setStyleSheet(stylesheet);
	ui->vLayoutBufferSlot->addWidget(buffer_previewer);

	buffer_previewer->setCursorPos(0.5);

	/* Plot layout */

	QSpacerItem *plotSpacer = new QSpacerItem(0, 25,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

	ui->gridLayoutPlot->addWidget(measurePanel, 0, 1, 1, 1);
	ui->gridLayoutPlot->addWidget(plot.topArea(), 1, 0, 1, 3);
	ui->gridLayoutPlot->addWidget(plot.leftHandlesArea(), 1, 0, 3, 1);
	ui->gridLayoutPlot->addWidget(&plot, 2, 1, 1, 1);
	ui->gridLayoutPlot->addWidget(plot.rightHandlesArea(), 1, 2, 3, 1);
	ui->gridLayoutPlot->addWidget(plot.bottomHandlesArea(), 3, 0, 1, 3);
	ui->gridLayoutPlot->addItem(plotSpacer, 4, 0, 1, 3);
	ui->gridLayoutPlot->addWidget(statisticsPanel, 5, 1, 1, 1);

	/* Default plot settings */
	plot.setSampleRate(adc.sampleRate(), 1, "");
	plot.setActiveVertAxis(0);

	for (unsigned int i = 0; i < nb_channels; i++) {
		plot.Curve(i)->setAxes(
				QwtAxisId(QwtPlot::xBottom, 0),
				QwtAxisId(QwtPlot::yLeft, i));
	}

	plot.setMinimumHeight(300);
	plot.setMinimumWidth(500);

	plot.levelTriggerA()->setMobileAxis(QwtAxisId(QwtPlot::yLeft, 0));
	plot.levelTriggerB()->setMobileAxis(QwtAxisId(QwtPlot::yLeft, 1));

	plot.setTriggerAEnabled(trigger_settings.levelA_enabled());
	plot.setTriggerBEnabled(trigger_settings.levelB_enabled());
	plot.levelTriggerA()->setPosition(trigger_settings.levelA_value());
	plot.levelTriggerB()->setPosition(trigger_settings.levelB_value());

	// TO DO: Give user the option to make these axes visible
	plot.enableAxis(QwtPlot::yLeft, false);
	plot.enableAxis(QwtPlot::xBottom, false);
	plot.setUsingLeftAxisScales(false);

	fft_plot.setMinimumHeight(250);
	fft_plot.setMinimumWidth(500);

	hist_plot.setMinimumHeight(200);
	hist_plot.setMinimumWidth(300);

	xy_plot.setMinimumHeight(200);
	xy_plot.setMinimumWidth(300);

	xy_plot.setVertUnitsPerDiv(5);
	xy_plot.setHorizUnitsPerDiv(5);

	ui->hlayout_fft->addWidget(&fft_plot);
	ui->container_fft_plot->hide();

	ui->gridLayoutHist->addWidget(&hist_plot, 0, 0);
	hist_plot.hide();

	ui->gridLayout_XY->addWidget(&xy_plot, 0, 0);
	xy_plot.hide();

	ui->rightMenu->setMaximumWidth(0);

	// Controls for scale/division and position controls (Horizontal & Vertical)
	timeBase = new ScaleSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0}
				}, "Time Base", 100e-9, 1E0);
	timePosition = new PositionSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0}
				}, "Position",
				-timeBase->maxValue() * 5,
				timeBase->maxValue() * 5);
	voltsPerDiv = new ScaleSpinButton({
				{"μVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0}
				}, "Volts/Div", 1e-6, 1e1);
	voltsPosition  = new PositionSpinButton({
				{"μVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0}
				}, "Position",
				-voltsPerDiv->maxValue() * 5,
				voltsPerDiv->maxValue() * 5);

	ch_ui = new Ui::ChannelSettings();
	ch_ui->setupUi(ui->channelSettings);

	ch_ui->horizontal->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	ch_ui->horizontal->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(1, voltsPerDiv, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(2, voltsPosition, 0, Qt::AlignLeft);

	timeBase->setValue(plot.HorizUnitsPerDiv());
	voltsPerDiv->setValue(plot.VertUnitsPerDiv());
	timePosition->setStep(timeBase->value() / 10);
	voltsPosition->setStep(voltsPerDiv->value() / 10);

	plot.setTimeBaseLabelValue(timeBase->value());

	/* General Settings Menu */
	gsettings_ui = new Ui::OscGeneralSettings();
	gsettings_ui->setupUi(ui->generalSettings);
	settings_group->addButton(ui->btnGeneralSettings);

	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

	gsettings_ui->Histogram_view->hide();
	gsettings_ui->XY_view->hide();

	connect(gsettings_ui->FFT_view, SIGNAL(toggled(bool)),
		SLOT(onFFT_view_toggled(bool)));
	connect(gsettings_ui->XY_view, SIGNAL(toggled(bool)),
		SLOT(onXY_view_toggled(bool)));
	connect(gsettings_ui->Histogram_view, SIGNAL(toggled(bool)),
		SLOT(onHistogram_view_toggled(bool)));

	connect(ui->btnGeneralSettings, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

	connect(runButton, SIGNAL(toggled(bool)), this,
			SLOT(runStopToggled(bool)));
	connect(ui->pushButtonRunStop, SIGNAL(toggled(bool)), this,
			SLOT(runStopToggled(bool)));
	connect(ui->pushButtonSingle, SIGNAL(toggled(bool)), this,
			SLOT(runStopToggled(bool)));
	connect(runButton, SIGNAL(toggled(bool)), ui->pushButtonRunStop,
			SLOT(setChecked(bool)));
	connect(ui->pushButtonRunStop, SIGNAL(toggled(bool)), runButton,
			SLOT(setChecked(bool)));

	// Signal-Slot Connections

	connect(timeBase, SIGNAL(valueChanged(double)),
		SLOT(onHorizScaleValueChanged(double)));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
	connect(timePosition, SIGNAL(valueChanged(double)),
		SLOT(onTimePositionChanged(double)));
	connect(voltsPosition, SIGNAL(valueChanged(double)),
		SLOT(onVertOffsetValueChanged(double)));

	/* Sync timePosition with plot time trigger bar */
	connect(&plot, SIGNAL(timeTriggerValueChanged(double)),
		this, SLOT(onTimeTriggerDelayChanged(double)));
	connect(this, SIGNAL(triggerPositionChanged(double)),
		timePosition, SLOT(setValue(double)));

	/* Update Timebase label each time the oscilloscope timebase changes */
	connect(timeBase, SIGNAL(valueChanged(double)),
		&plot, SLOT(setTimeBaseLabelValue(double)));

	connect(&plot, SIGNAL(channelOffsetChanged(double)),
		SLOT(onChannelOffsetChanged(double)));

	connect(this, SIGNAL(selectedChannelChanged(int)),
		&plot, SLOT(setSelectedChannel(int)));
	connect(this, SIGNAL(selectedChannelChanged(int)),
		&plot, SLOT(setZoomerVertAxis(int)));

	connect(&plot,
		SIGNAL(cursorReadoutsChanged(struct cursorReadoutsText)),
		SLOT(onCursorReadoutsChanged(struct cursorReadoutsText)));

	// Connections with Trigger Settings
	connect(&trigger_settings, SIGNAL(triggerAenabled(bool)),
		&plot, SLOT(setTriggerAEnabled(bool)));
	connect(&trigger_settings, SIGNAL(triggerBenabled(bool)),
		&plot, SLOT(setTriggerBEnabled(bool)));
	connect(&trigger_settings, SIGNAL(levelAChanged(double)),
		plot.levelTriggerA(), SLOT(setPosition(double)));
	connect(plot.levelTriggerA(), SIGNAL(positionChanged(double)),
		&trigger_settings, SLOT(setTriggerLevelA(double)));
	connect(&trigger_settings, SIGNAL(levelBChanged(double)),
		plot.levelTriggerB(), SLOT(setPosition(double)));
	connect(plot.levelTriggerB(), SIGNAL(positionChanged(double)),
		&trigger_settings, SLOT(setTriggerLevelB(double)));

	connect(&trigger_settings, &TriggerSettings::levelAChanged,
		[=](double level) {
			plot.setPeriodDetectLevel(0, level);
		});
	connect(&trigger_settings, &TriggerSettings::levelBChanged,
		[=](double level) {
			plot.setPeriodDetectLevel(1, level);
		});

	connect(&trigger_settings, SIGNAL(triggerModeChanged(int)),
		this, SLOT(onTriggerModeChanged(int)));

	Ui::CursorsSettings cr_ui;
	cr_ui.setupUi(ui->cursorsSettings);
	connect(cr_ui.hCursorsEnanble, SIGNAL(toggled(bool)),
		&plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui.vCursorsEnanble, SIGNAL(toggled(bool)),
		&plot, SLOT(setHorizCursorsEnabled(bool)));

	connect(cr_ui.hCursorsEnanble, SIGNAL(toggled(bool)),
		cursor_readouts_ui->TimeCursors,
		SLOT(setVisible(bool)));
	connect(cr_ui.vCursorsEnanble, SIGNAL(toggled(bool)),
		cursor_readouts_ui->VoltageCursors,
		SLOT(setVisible(bool)));

	connect(&*iio, SIGNAL(timeout()),
			&trigger_settings, SLOT(autoTriggerDisable()));
	connect(&plot, SIGNAL(newData()),
			&trigger_settings, SLOT(autoTriggerEnable()));
	connect(&plot, SIGNAL(newData()), this, SLOT(singleCaptureDone()));

	connect(&*iio, SIGNAL(timeout()),
			SLOT(onIioDataRefillTimeout()));
	connect(&plot, SIGNAL(newData()), this, SLOT(onPlotNewData()));


	if (nb_channels < 2)
		gsettings_ui->XY_view->hide();

	QWidget *chn0_widget = channelWidgetAtId(0);
	if (chn0_widget) {
		QPushButton *name = chn0_widget->findChild<QPushButton *>("name");
		name->setChecked(true);
	}

	// Default hysteresis levels for measurements
	for (int i = 0; i < nb_channels; i++)
		plot.setPeriodDetectHyst(i, 1.0 / 5);

	ui->pushButtonRunStop->setProperty("normal_text",
			QVariant(ui->pushButtonRunStop->text()));
	ui->pushButtonSingle->setProperty("normal_text",
			QVariant(ui->pushButtonSingle->text()));

	// Calculate initial sample count and sample rate
	onHorizScaleValueChanged(timeBase->value());
	onTimePositionChanged(timePosition->value());

	osc_api->load();
	osc_api->js_register(engine);
}

Oscilloscope::~Oscilloscope()
{
	for (unsigned int i = 0; i < nb_channels; i++)
		iio->stop(ids[i]);
	if (fft_is_visible)
		for (unsigned int i = 0; i < nb_channels; i++)
			iio->stop(fft_ids[i]);
	if (hist_is_visible)
		for (unsigned int i = 0; i < nb_channels; i++)
			iio->stop(hist_ids[i]);
	if (xy_is_visible)
		for (unsigned int i = 0; i < (nb_channels & ~1); i++)
			iio->stop(xy_ids[i]);

	bool started = iio->started();
	if (started)
		iio->lock();

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->disconnect(ids[i]);
	if (fft_is_visible)
		for (unsigned int i = 0; i < nb_channels; i++)
			iio->disconnect(fft_ids[i]);
	if (hist_is_visible)
		for (unsigned int i = 0; i < nb_channels; i++)
			iio->disconnect(hist_ids[i]);
	if (xy_is_visible)
		for (unsigned int i = 0; i < (nb_channels & ~1); i++)
			iio->disconnect(xy_ids[i]);

	if (started)
		iio->unlock();

	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug() << "OSC disconnected:\n" << gr::dot_graph(hier).c_str();

	osc_api->save();
	delete osc_api;

	delete[] xy_ids;
	delete[] hist_ids;
	delete[] fft_ids;
	delete[] ids;
	delete measure_ui;
	delete cursor_ui;
	delete ch_ui;
	delete gsettings_ui;
	delete measure_panel_ui;
	delete cursor_readouts_ui;
	delete ui;
}

void Oscilloscope::create_math_panel()
{
	/* Math stuff */
    settings_group->addButton(ui->btnAddMath);
    connect(ui->btnAddMath, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

	QWidget *panel = new QWidget(this);
	Ui::MathPanel math_ui;

	math_ui.setupUi(panel);
	QPushButton *btn = math_ui.btnAddChannel;

	Math *math = new Math(nullptr, nb_channels);

	connect(math, &Math::functionValid,
			[=](const QString &function) {
				btn->setEnabled(true);
				btn->setProperty("function",
						QVariant(function));
			});

	connect(math, &Math::stateReseted, [=]() {
				btn->setEnabled(false);
			});

	connect(btn, &QPushButton::clicked,
			[=]() {
				QVariant var = btn->property("function");
				add_math_channel(var.toString().toStdString());
			});

	QVBoxLayout *layout = static_cast<QVBoxLayout *>(panel->layout());
	layout->insertWidget(0, math);

	int panel_id = ui->stackedWidget->insertWidget(-1, panel);
	ui->btnAddMath->setProperty("id", QVariant(-panel_id));
}

unsigned int Oscilloscope::find_curve_number()
{
	unsigned int id = 0;
	bool found;

	do {
		found = false;

		for (unsigned int i = 0; !found && i < nb_math_channels; i++) {
			QWidget *parent = ui->channelsList->itemAt(nb_channels + i)->widget();

			found = parent->property("curve_nb").toUInt() == id;
		}

		id++;
	} while (found);

	return id - 1;
}

void Oscilloscope::add_math_channel(const std::string& function)
{
	auto math = iio::iio_math::make(function, nb_channels);
	unsigned int curve_id = nb_channels + nb_math_channels;
	unsigned int curve_number = find_curve_number();

	nb_math_channels++;

	QString qname = QString("Math %1").arg(math_chn_counter++);
	std::string name = qname.toStdString();

	auto math_sink = adiscope::scope_sink_f::make(
			plot.axisInterval(QwtPlot::xBottom).width() * adc.sampleRate(),
			adc.sampleRate(), name, 1, (QObject *)&plot);

	/* Add the math block and the math scope sink into a container, so that
	 * we can disconnect them when removing the math channel later */
	auto math_pair = QPair<gr::basic_block_sptr, gr::basic_block_sptr>(
				math, math_sink);
	math_sinks.insert(qname, math_pair);

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->connect(adc_samp_conv_block, i, math, i);
	iio->connect(math, 0, math_sink, 0);

	if (started)
		iio->unlock();

	plot.registerSink(name, 1,
			plot.axisInterval(QwtPlot::xBottom).width() *
			adc.sampleRate());

	QWidget *channel_widget = new QWidget(this);
	Ui::ChannelMath channel_ui;

	channel_ui.setupUi(channel_widget);
	channel_ui.name->setText(QString("Math %1").arg(curve_number + 1));

	QString stylesheet(channel_ui.box->styleSheet());
	stylesheet += QString("\nQCheckBox::indicator {\nborder-color: %1;\n}\nQCheckBox::indicator:checked {\nbackground-color: %1;\n}\n"
			).arg(plot.getLineColor(curve_id).name());
	channel_ui.box->setStyleSheet(stylesheet);

	channel_widget->setProperty("curve_nb", QVariant(curve_number));
	channel_ui.box->setProperty("id", QVariant(curve_id));
	channel_ui.btn->setProperty("id", QVariant(curve_id));
	channel_ui.name->setProperty("id", QVariant(curve_id));
	channel_ui.delBtn->setProperty("curve_name", QVariant(qname));

	connect(channel_ui.box, SIGNAL(toggled(bool)), this,
			SLOT(channel_box_toggled(bool)));

	connect(channel_ui.btn, SIGNAL(pressed()),
			this, SLOT(toggleRightMenu()));

	connect(channel_ui.name, SIGNAL(toggled(bool)),
				SLOT(channel_name_checked(bool)));

	connect(channel_ui.delBtn, SIGNAL(pressed()),
			this, SLOT(del_math_channel()));

	ui->channelsList->addWidget(channel_widget);

	settings_group->addButton(channel_ui.btn);
	channels_group->addButton(channel_ui.name);

	plot.Curve(curve_id)->setAxes(
			QwtAxisId(QwtPlot::xBottom, 0),
			QwtAxisId(QwtPlot::yLeft, curve_id));
	plot.replot();

	/* We added a Math channel that is enabled by default,
	 * so enable the Run button */
	updateRunButton(true);

	// Default hysteresis levels for measurements of the new channel
	plot.setPeriodDetectHyst(curve_id, 1.0 / 5);
}

void Oscilloscope::del_math_channel()
{
	QPushButton *delBtn = static_cast<QPushButton *>(QObject::sender());
	QWidget *parent = delBtn->parentWidget();
	QPushButton *btn = parent->findChild<QPushButton *>("btn");
	unsigned int curve_id = btn->property("id").toUInt();
	QString qname = delBtn->property("curve_name").toString();

	measure_settings->onChannelRemoved(curve_id);

	plot.unregisterSink(qname.toStdString());

	nb_math_channels--;

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	/* Disconnect the blocks from the running flowgraph */
	auto pair = math_sinks.take(qname);
	for (unsigned int i = 0; i < nb_channels; i++)
		iio->disconnect(adc_samp_conv_block, i, pair.first, i);
	iio->disconnect(pair.first, 0, pair.second, 0);

	if (started)
		iio->unlock();

	/* Close the right menu if it shows the actual channel */
	if (btn == settings_group->checkedButton()) {
		settings_group->setExclusive(false);
		ui->btnSettings->setChecked(false);
		active_settings_btn = nullptr;
		last_non_general_settings_btn = active_settings_btn;
	}

	/* Remove the math channel from the bottom list of channels */
	settings_group->removeButton(btn);
	ui->channelsList->removeWidget(parent);
	delete parent;

	/* If the removed channel is before the current axis, we update the
	 * current axis to account for the index change */
	int current_axis = plot.activeVertAxis();
	if (current_axis > curve_id)
		plot.setActiveVertAxis(current_axis - 1);

	/* Before removing the axis make sure cursors are not sing it */
	QWidget *chn_widget = channelWidgetAtId(curve_id);
	QPushButton *name = chn_widget->findChild<QPushButton *>("name");
	channels_group->removeButton(name);
	name->setChecked(false);
	if (channels_group->buttons().size() > 0)
		channels_group->buttons()[0]->setChecked(true);

	/* Before removing the axis, remove the offset widgets */
	plot.removeOffsetWidgets(curve_id);

	/* Remove the axis that corresponds to the curve we drop */
	plot.removeLeftVertAxis(curve_id);

	for (unsigned int i = nb_channels;
			i < nb_channels + nb_math_channels; i++) {
		QWidget *parent = ui->channelsList->itemAt(i)->widget();
		QPushButton *btn = parent->findChild<QPushButton *>("btn");
		QCheckBox *box = parent->findChild<QCheckBox *>("box");

		/* Update the IDs */
		btn->setProperty("id", QVariant(i));
		box->setProperty("id", QVariant(i));

		/* Update the curve-to-axis map */
		plot.Curve(i)->setAxes(
				QwtAxisId(QwtPlot::xBottom, 0),
				QwtAxisId(QwtPlot::yLeft, i));
	}

	updateRunButton(false);
}

void Oscilloscope::on_actionClose_triggered()
{
	this->close();
}

void Oscilloscope::runStopToggled(bool checked)
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());

	if (btn == menuRunButton)
		btn = ui->pushButtonRunStop;

	if (checked) {
		btn->setText("Stop");

		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);

		last_set_sample_count = active_sample_count;

		if (active_sample_rate != adc.sampleRate())
			adc.setSampleRate(active_sample_rate);

		if (active_trig_sample_count !=
				trigger_settings.triggerDelay()) {
			trigger_settings.setTriggerDelay(
				active_trig_sample_count);
			last_set_time_pos = active_time_pos;
		}

		if (timePosition->value() != active_time_pos)
			timePosition->setValue(active_time_pos);

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->start(ids[i]);
		if (fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->start(fft_ids[i]);
		if (hist_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->start(hist_ids[i]);
		if (xy_is_visible)
			for (unsigned int i = 0; i < (nb_channels & ~1); i++)
				iio->start(xy_ids[i]);
	} else {
		btn->setText(btn->property("normal_text").toString());

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->stop(ids[i]);
		if (fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->stop(fft_ids[i]);
		if (hist_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->stop(hist_ids[i]);
		if (xy_is_visible)
			for (unsigned int i = 0; i < (nb_channels & ~1); i++)
				iio->stop(xy_ids[i]);
	}

	// Update trigger status
	triggerUpdater->setEnabled(checked);
}

void Oscilloscope::onFFT_view_toggled(bool visible)
{
	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	if (visible) {
		std::string mag2dB_formula = "20 * log10(x/4096/" + std::to_string(qt_fft_block->nsamps() / 2) + ")"; // 4096 = 2^NUM_ADC_BITS

		for (unsigned int i = 0; i < nb_channels; i++) {
			auto fft = gnuradio::get_initial_sptr(
					new fft_block(false, fft_size));

			fft_ids[i] = iio->connect(fft, i, 0, true);

			auto ctm = blocks::complex_to_mag::make(1);
			auto m2dB = iio::iio_math::make(mag2dB_formula);

			iio->connect(fft, 0, ctm, 0);
			iio->connect(ctm, 0, m2dB, 0);
			iio->connect(m2dB, 0, qt_fft_block, i);

			if (ui->pushButtonRunStop->isChecked())
				iio->start(fft_ids[i]);
		}

		ui->container_fft_plot->show();
	} else {
		ui->container_fft_plot->hide();

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->disconnect(fft_ids[i]);
	}

	fft_is_visible = visible;

	if (started)
		iio->unlock();
}

void Oscilloscope::onHistogram_view_toggled(bool visible)
{
	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	if (visible) {
		for (unsigned int i = 0; i < nb_channels; i++) {
			hist_ids[i] = iio->connect(qt_hist_block, i, i, true);

			if (ui->pushButtonRunStop->isChecked())
				iio->start(hist_ids[i]);
		}

		hist_plot.show();
	} else {
		hist_plot.hide();

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->disconnect(hist_ids[i]);
	}

	hist_is_visible = visible;

	if (started)
		iio->unlock();
}

void Oscilloscope::onXY_view_toggled(bool visible)
{
	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	if (visible) {

		for (unsigned int i = 0; i < nb_channels / 2; i++) {
			auto ftc = blocks::float_to_complex::make(1);
			auto basic = ftc->to_basic_block();

			xy_ids[i * 2] = iio->connect(basic, i * 2, 0, true);
			xy_ids[i * 2 + 1] = iio->connect(basic,
					i * 2 + 1, 1, true);

			printf("IDs: %s / %s\n", xy_ids[i * 2]->alias().c_str(),
					xy_ids[i * 2 + 1]->alias().c_str());

			iio->connect(ftc, 0, this->qt_xy_block, i);
		}

		if (ui->pushButtonRunStop->isChecked())
			for (unsigned int i = 0; i < (nb_channels & ~1); i++)
				iio->start(xy_ids[i]);

		xy_plot.show();
	} else {
		xy_plot.hide();

		for (unsigned int i = 0; i < (nb_channels & ~1); i++)
			iio->iio_manager::disconnect(xy_ids[i]);
	}

	xy_is_visible = visible;

	if (started)
		iio->unlock();
}

void adiscope::Oscilloscope::onCursorsToggled(bool on)
{
	QCheckBox *box = static_cast<QCheckBox *>(QObject::sender());
	QPushButton *btn = box->parentWidget()->findChild<QPushButton *>("btn");

	if (!on) {
		if (btn->isChecked()) {
			settings_group->setExclusive(false);
			ui->btnSettings->setChecked(false);
		}
	}

	plot.setMeasurementCursorsEnabled(on);

	// Set the visibility of the cursor readouts owned by the Oscilloscope
	QCheckBox *mbox = ui->measure_settings->itemAt(0)->widget()->
		findChild<QCheckBox *>("box");
	if (on)
		plot.setCursorReadoutsVisible(!mbox->isChecked());
	measure_panel_ui->cursorReadouts->setVisible(on);
}

void adiscope::Oscilloscope::onMeasureToggled(bool on)
{
	QCheckBox *box = static_cast<QCheckBox *>(QObject::sender());
	QPushButton *btn = box->parentWidget()->findChild<QPushButton *>("btn");

	if (!on) {
		if (btn->isChecked()) {
			settings_group->setExclusive(false);
			ui->btnSettings->setChecked(false);
		}
	} else {
		update_measure_for_channel(selectedChannel);
	}
	measurePanel->setVisible(on);

	// Set the visibility of the cursor readouts owned by the plot
	if (plot.measurementCursorsEnabled())
		plot.setCursorReadoutsVisible(!on);
}

void Oscilloscope::onTimeTriggerDelayChanged(double value)
{
	if (timePosition->value() != value)
		Q_EMIT triggerPositionChanged(value);
}

void Oscilloscope::comboBoxUpdateToValue(QComboBox *box, double value, std::vector<double>list)
{
	int i = find_if( list.begin(), list.end(),
				[&value](const double element) {return element == value;} ) - list.begin();
	if (i < list.size())
		box->setCurrentIndex(i);
}

void adiscope::Oscilloscope::on_comboBox_currentIndexChanged(const QString &arg1)
{
	bool ok;
	int size = arg1.toInt(&ok);

	if (!ok)
		return;

	if (size != fft_size) {
		fft_size = size;

		if (fft_is_visible) {
			bool started = iio->started();
			if (started)
				iio->lock();

			qt_fft_block->set_nsamps(fft_size);

			for (unsigned int i = 0; i < nb_channels; i++)
				iio->disconnect(fft_ids[i]);

			if (started)
				iio->unlock();

			onFFT_view_toggled(fft_is_visible);

			if (started)
				iio->lock();

			for (unsigned int i = 0; i < nb_channels; i++)
				iio->set_buffer_size(fft_ids[i], fft_size);

			if (started)
				iio->unlock();
		}
	}
}

void adiscope::Oscilloscope::updateRunButton(bool ch_enabled)
{
	for (unsigned int i = 0; !ch_enabled &&
			i < nb_channels + nb_math_channels; i++) {
		QWidget *parent = ui->channelsList->itemAt(i)->widget();
		QCheckBox *box = parent->findChild<QCheckBox *>("box");
		ch_enabled = box->isChecked();
	}

	ui->pushButtonRunStop->setEnabled(ch_enabled);
	menuRunButton->setEnabled(ch_enabled);
	if (!ch_enabled) {
		ui->pushButtonRunStop->setChecked(false);
		menuRunButton->setChecked(false);
	}
}

void adiscope::Oscilloscope::channel_box_toggled(bool checked)
{
	QCheckBox *box = static_cast<QCheckBox *>(QObject::sender());
	QPushButton *btn = box->parentWidget()->findChild<QPushButton *>("btn");
	QPushButton *name = box->parentWidget()->findChild<QPushButton *>("name");
	unsigned int id = box->property("id").toUInt();

	if (checked) {
		qDebug() << "Attaching curve" << id;
		plot.AttachCurve(id);
		channels_group->addButton(name);
		name->setChecked(true);

	} else {
		if (btn->isChecked()) {
			settings_group->setExclusive(false);
			ui->btnSettings->setChecked(false);
		}

		qDebug() << "Detaching curve" << id;
		plot.DetachCurve(id);

		channels_group->removeButton(name);
		name->setChecked(false);
		if (channels_group->buttons().size() > 0)
			channels_group->buttons()[0]->setChecked(true);
	}

	plot.setOffsetWidgetVisible(id, checked);

	plot.replot();
	updateRunButton(checked);
}

void adiscope::Oscilloscope::channel_name_checked(bool checked)
{
	QPushButton *name = static_cast<QPushButton *>(QObject::sender());
	QCheckBox *box = name->parentWidget()->findChild<QCheckBox *>("box");

	setDynamicProperty(name->parentWidget(), "selected", checked);

	if (checked && !box->checkState())
		box->setChecked(true);

	// Get the channel that is curently selected
	QAbstractButton *selBtn = channels_group->checkedButton();
	int id;
	if (!selBtn)
		id = -1;
	else
		id = selBtn->property("id").toUInt();
	if (selectedChannel != id) {
		selectedChannel = id;
		Q_EMIT selectedChannelChanged(id);
	}

	if (checked && plot.measurementsEnabled() && id != -1) {
		update_measure_for_channel(id);
	}
}

void adiscope::Oscilloscope::onVertScaleValueChanged(double value)
{
	if (value != plot.VertUnitsPerDiv(plot.activeVertAxis())) {
		plot.setVertUnitsPerDiv(value, plot.activeVertAxis());
		plot.replot();
	}
	voltsPosition->setStep(value / 10);

	// Send scale information to the measure object
	plot.setPeriodDetectHyst(current_channel, value / 5);
}

void adiscope::Oscilloscope::onHorizScaleValueChanged(double value)
{
	symmBufferMode->setTimeBase(value);
	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sample_rate = params.sampleRate;
	active_sample_count = params.entireBufferSize;
	active_trig_sample_count = -(long long)params.triggerBufferSize;
	active_time_pos = -params.timePos;

	// Realign plot data based on the new sample count and trigger position
	plot.setHorizUnitsPerDiv(value);
	plot.replot();
	plot.setDataStartingPoint(active_trig_sample_count);
	plot.resetXaxisOnNextReceivedData();

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	bool started = iio->started();
	if (started)
		iio->lock();
	this->qt_time_block->set_nsamps(active_sample_count);

	// Apply amplitude corrections when using different sample rates
	if (active_sample_rate != adc.sampleRate()) {
		boost::shared_ptr<adc_sample_conv> block =
			dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);
		block->setFilterCompensation(0, adc.compTable(active_sample_rate));
		block->setFilterCompensation(1, adc.compTable(active_sample_rate));
	}

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);

		last_set_sample_count = active_sample_count;

		adc.setSampleRate(active_sample_rate);
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;

		// Time base changes can limit the time position value
		if (timePosition->value() != -params.timePos)
			timePosition->setValue(-params.timePos);
	}

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->set_buffer_size(ids[i], active_sample_count);

	if (started)
		iio->unlock();

	// Change the sensitivity of time position control
	timePosition->setStep(value / 10);

	updateBufferPreviewer();
}

void adiscope::Oscilloscope::onVertOffsetValueChanged(double value)
{
	if (value != plot.VertOffset(plot.activeVertAxis())) {
		plot.setVertOffset(value, plot.activeVertAxis());
		plot.replot();
	}
}

void adiscope::Oscilloscope::onTimePositionChanged(double value)
{
	bool started = iio->started();

	unsigned long oldSampleCount = symmBufferMode->captureParameters().entireBufferSize;
	symmBufferMode->setTriggerPos(-value);
	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sample_rate = params.sampleRate;
	active_sample_count = params.entireBufferSize;
	active_trig_sample_count = -(long long)params.triggerBufferSize;
	active_time_pos = -params.timePos;

	// Realign plot data based on the new time position
	plot.setHorizOffset(value);
	plot.replot();
	plot.setDataStartingPoint(active_trig_sample_count);
	plot.resetXaxisOnNextReceivedData();

	if (started) {
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;
	}

	updateBufferPreviewer();

	if (active_sample_rate == adc.sampleRate() &&
			(active_sample_count == oldSampleCount))
		return;

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	if (started)
		iio->lock();
	this->qt_time_block->set_nsamps(active_sample_count);

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);

		last_set_sample_count = active_sample_count;

		adc.setSampleRate(active_sample_rate);
	}

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->set_buffer_size(ids[i], active_sample_count);

	if (started)
		iio->unlock();
}

void adiscope::Oscilloscope::rightMenuFinished(bool opened)
{
	menuOpened = opened;

	if (!opened && active_settings_btn && active_settings_btn->isChecked()) {
		int id = active_settings_btn->property("id").toInt();
		settings_panel_update(id);
		if (id >= 0) {
			update_chn_settings_panel(id);
		}
		ui->rightMenu->toggleMenu(true);
	}
}

void adiscope::Oscilloscope::toggleRightMenu(QPushButton *btn)
{
	int id = btn->property("id").toInt();
	bool btn_old_state = btn->isChecked();
	bool open = !menuOpened;

	active_settings_btn = btn;
	if (id != -ui->stackedWidget->indexOf(ui->generalSettings))
		last_non_general_settings_btn = active_settings_btn;

	settings_group->setExclusive(!btn_old_state);

	if (open)
		settings_panel_update(id);

	if (id >= 0) {
		current_channel = id;
		plot.setActiveVertAxis(id);
		if (open) {
			update_chn_settings_panel(id, btn->parentWidget());
		}
	}

	ui->rightMenu->toggleMenu(open);
}

void adiscope::Oscilloscope::toggleRightMenu()
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());

	toggleRightMenu(btn);

	int id = btn->property("id").toInt();
	bool settingsState = !btn->isChecked();

	if (id == -ui->stackedWidget->indexOf(ui->generalSettings) &&
			settingsState) {
		settingsState = false;
	}
	const QSignalBlocker blocker(ui->btnSettings);
		ui->btnSettings->setChecked(settingsState);
}

void Oscilloscope::settings_panel_update(int id)
{
	if (id >= 0)
        ui->stackedWidget->setCurrentIndex(0);
	else
        ui->stackedWidget->setCurrentIndex(-id);

        settings_panel_size_adjust();
}

void Oscilloscope::settings_panel_size_adjust()
{
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;

		if (i == ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
		}
		QWidget *widget = ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
	}
	ui->stackedWidget->adjustSize();
}

void Oscilloscope::onChannelOffsetChanged(double value)
{
	voltsPosition->setValue(plot.VertOffset(plot.activeVertAxis()));
}

QWidget * Oscilloscope::channelWidgetAtId(int id)
{
	QWidget *w = NULL;
	QPushButton *btn;
	bool found = false;

	for (unsigned int i = 0; !found &&
				i < nb_channels + nb_math_channels; i++) {

			w = ui->channelsList->itemAt(i)->widget();
			btn = w->findChild<QPushButton *>("btn");
			found = btn->property("id").toUInt() == id;
		}

	return w;
}

void Oscilloscope::update_chn_settings_panel(int id, QWidget *chn_widget)
{
	if (!chn_widget)
		chn_widget = channelWidgetAtId(id);
	if (!chn_widget)
		return;

	voltsPerDiv->setValue(plot.VertUnitsPerDiv(id));
	voltsPosition->setValue(plot.VertOffset(id));

	QPushButton *name = chn_widget->findChild<QPushButton *>("name");
	ch_ui->label_channelName->setText(name->text());
	QString stylesheet = QString("border: 2px solid %1"
					).arg(plot.getLineColor(id).name());
	ch_ui->line_channelColor->setStyleSheet(stylesheet);
}

void Oscilloscope::onMeasuremetsAvailable()
{
	measureUpdateValues();

	if (statistics_enabled) {
		statisticsUpdateValues();
		statisticsUpdateGui();
	}
}

void Oscilloscope::update_measure_for_channel(int ch_idx)
{
	QWidget *chn_widget = channelWidgetAtId(ch_idx);
	QPushButton *name = chn_widget->findChild<QPushButton *>("name");

	measure_settings->setChannelName(name->text());
	measure_settings->setChannelUnderlineColor(plot.getLineColor(ch_idx));
}

void Oscilloscope::measureCreateAndAppendGuiFrom(const MeasurementData&
		measurement)
{
	std::shared_ptr<MeasurementGui> p;

	switch(measurement.unitType()) {

	case MeasurementData::METRIC:
		p = std::make_shared<MetricMeasurementGui>();
		break;
	case MeasurementData::TIME:
		p = std::make_shared<TimeMeasurementGui>();
		break;
	case MeasurementData::PERCENTAGE:
		p = std::make_shared<PercentageMeasurementGui>();
		break;
	case MeasurementData::DIMENSIONLESS:
		p = std::make_shared<DimensionlessMeasurementGui>();
		break;
	default:
		break;
	}
	if (p)
		measurements_gui.push_back(p);
}

void Oscilloscope::measureLabelsRearrange()
{
	QWidget *container = measure_panel_ui->measurements->
					findChild<QWidget *>("container");

	if (container) {
		measure_panel_ui->measurements->layout()->removeWidget(container);
		delete container;
	}

	container = new QWidget();
	container->setObjectName("container");
	if (!measure_panel_ui->measurements->layout()) {
		QVBoxLayout *measurementsLayout = new
				QVBoxLayout(measure_panel_ui->measurements);
		measurementsLayout->addWidget(container);
		measurementsLayout->setContentsMargins(0, 0, 0, 0);
	} else {
		measure_panel_ui->measurements->layout()->addWidget(container);
	}

	QGridLayout*gLayout = new QGridLayout(container);
	gLayout->setContentsMargins(0, 0, 0, 0);
	gLayout->setVerticalSpacing(5);
	gLayout->setHorizontalSpacing(5);
	int max_rows = 4;
	int nb_meas_added = 0;

	for (int i = 0; i < measurements_data.size(); i++) {
		QLabel *name = new QLabel();
		QLabel *value = new QLabel();

		int row = nb_meas_added % max_rows;
		int col = nb_meas_added / max_rows;

		gLayout->addWidget(name, row, 2 * col);

		QHBoxLayout *value_layout = new QHBoxLayout();
		value_layout->setContentsMargins(0, 0, 10, 0);
		value_layout->addWidget(value);
		gLayout->addLayout(value_layout, row, 2 * col + 1);

		measurements_gui[i]->init(name, value);
		measurements_gui[i]->update(*(measurements_data[i]));
		measurements_gui[i]->setLabelsColor(plot.getLineColor(
			measurements_data[i]->channel()));

		nb_meas_added++;
	}
}

void Oscilloscope::measureUpdateValues()
{
	for (int i = 0; i < measurements_data.size(); i++)
		measurements_gui[i]->update(*(measurements_data[i]));
}

void Oscilloscope::measure_settings_init()
{
	measure_settings = new MeasureSettings(&plot, this);

	int measure_panel = ui->stackedWidget->insertWidget(-1, measure_settings);

	connect(measure_settings,
		SIGNAL(measurementActivated(int, int)),
		SLOT(onMeasurementActivated(int, int)));

	connect(measure_settings,
		SIGNAL(measurementDeactivated(int, int)),
		SLOT(onMeasurementDeactivated(int, int)));

	connect(measure_settings,
		SIGNAL(measurementSelectionListChanged()),
		SLOT(onMeasurementSelectionListChanged()));

	connect(measure_settings,
		SIGNAL(statisticActivated(int, int)),
		SLOT(onStatisticActivated(int, int)));
	connect(measure_settings,
		SIGNAL(statisticDeactivated(int, int)),
		SLOT(onStatisticDeactivated(int, int)));

	connect(measure_settings, SIGNAL(statisticsEnabled(bool)),
		SLOT(onStatisticsEnabled(bool)));

	connect(measure_settings, SIGNAL(statisticsReset()),
		SLOT(onStatisticsReset()));

	connect(&plot, SIGNAL(channelAdded(int)),
		measure_settings, SLOT(onChannelAdded(int)));

	connect(this, SIGNAL(selectedChannelChanged(int)),
		measure_settings, SLOT(setSelectedChannel(int)));

	connect(measure_settings,
		SIGNAL(statisticSelectionListChanged()),
		SLOT(onStatisticSelectionListChanged()));

	QWidget *measure_widget = new QWidget(this);

	measure_ui = new Ui::Channel();
	measure_ui->setupUi(measure_widget);
	ui->measure_settings->addWidget(measure_widget);
	settings_group->addButton(measure_ui->btn);
	measure_ui->btn->setProperty("id", QVariant(-measure_panel));
	measure_ui->name->setText("Measure");
	measure_ui->box->setChecked(false);
	QString stylesheet(measure_ui->box->styleSheet());
	stylesheet += QString("\nQCheckBox::indicator {"
				"border-color: rgb(74, 100, 255);"
				"border-radius: 4px;"
				"}"
				"QCheckBox::indicator:checked {"
				"background-color: rgb(74, 100, 255);"
				"}");
	measure_ui->box->setStyleSheet(stylesheet);

	connect(measure_ui->btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));
	connect(measure_ui->box, SIGNAL(toggled(bool)), this,
			SLOT(onMeasureToggled(bool)));
	connect(measure_ui->box, SIGNAL(toggled(bool)),
		&plot, SLOT(setMeasuremensEnabled(bool)));
}

void Oscilloscope::onMeasurementActivated(int id, int chnIdx)
{
	int oldActiveMeasCount = plot.activeMeasurementsCount(chnIdx);

	auto mList = plot.measurements(chnIdx);
	mList[id]->setEnabled(true);
	measurements_data.push_back(mList[id]);
	measureCreateAndAppendGuiFrom(*mList[id]);

	// Even if a measurement had been added after data was captured, it
	// should display the measurement value corresponding to that data
	if (oldActiveMeasCount == 0) {
		plot.measure();
	}

	measureLabelsRearrange();
}

void Oscilloscope::onMeasurementDeactivated(int id, int chnIdx)
{
	auto mList = plot.measurements(chnIdx);
	QString name = mList[id]->name();

	mList[id]->setEnabled(false);

	auto it = find_if(measurements_data.begin(), measurements_data.end(),
		[&](std::shared_ptr<MeasurementData> const& p)
		{ return  (p->name() == name) && (p->channel() == chnIdx); });
	if (it != measurements_data.end()) {
		int i = it - measurements_data.begin();
		measurements_data.removeAt(i);
		measurements_gui.removeAt(i);
		measureLabelsRearrange();
	}
}

void Oscilloscope::onMeasurementSelectionListChanged()
{
	// Clear all measurements in list
	for (int i = 0; i < measurements_data.size(); i++) {
		measurements_data[i]->setEnabled(false);
	}
	measurements_data.clear();
	measurements_gui.clear();

	// Use the new list from MeasureSettings
	auto newList = measure_settings->measurementSelection();
	for (int i = 0; i < newList.size(); i++) {
		auto pMeasurement = plot.measurement(newList[i].id(),
			newList[i].channel_id());
		if (pMeasurement) {
			pMeasurement->setEnabled(true);
			measurements_data.push_back(pMeasurement);
			measureCreateAndAppendGuiFrom(*pMeasurement);
		}
	}
	measureLabelsRearrange();
}

void Oscilloscope::onCursorReadoutsChanged(struct cursorReadoutsText data)
{
	fillCursorReadouts(data);
}

void Oscilloscope::fillCursorReadouts(const struct cursorReadoutsText& data)
{
	cursor_readouts_ui->cursorT1->setText(data.t1);
	cursor_readouts_ui->cursorT2->setText(data.t2);
	cursor_readouts_ui->timeDelta->setText(data.tDelta);
	cursor_readouts_ui->frequencyDelta->setText(data.freq);
	cursor_readouts_ui->cursorV1->setText(data.v1);
	cursor_readouts_ui->cursorV2->setText(data.v2);
	cursor_readouts_ui->voltageDelta->setText(data.vDelta);
}

void Oscilloscope::measure_panel_init()
{
	measurePanel = new QWidget(this);
	measure_panel_ui = new Ui::MeasurementsPanel();
	measure_panel_ui->setupUi(measurePanel);
	measurePanel->hide();

	connect(&plot, SIGNAL(measurementsAvailable()),
		SLOT(onMeasuremetsAvailable()));

	// The second CursorReadouts belongs to the Measure panel. The first
	// one is drawn on top of the plot canvas.
	cursorReadouts = new QWidget(measure_panel_ui->cursorReadouts);
	cursor_readouts_ui = new Ui::CursorReadouts();
	cursor_readouts_ui->setupUi(cursorReadouts);
	cursor_readouts_ui->horizontalSpacer->changeSize(15, 0,
		QSizePolicy::Fixed, QSizePolicy::Fixed);
	cursor_readouts_ui->horizontalSpacer_2->changeSize(0, 0,
		QSizePolicy::Fixed, QSizePolicy::Fixed);
	cursor_readouts_ui->horizontalSpacer_3->changeSize(10, 0,
		QSizePolicy::Fixed, QSizePolicy::Fixed);
	cursor_readouts_ui->horizontalSpacer_4->changeSize(0, 0,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

	cursor_readouts_ui->TimeCursors->setStyleSheet("QWidget {"
		"background-color: transparent;"
		"color: white;}");
	cursor_readouts_ui->VoltageCursors->setStyleSheet("QWidget {"
		"background-color: transparent;"
		"color: white;}");

	// Avoid labels jumping around to left or right by imposing a min width
	QLabel *label = new QLabel(this);
	label->setStyleSheet("font-size: 14px");
	label->setText("-999.999 ns");
	double minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->cursorT1->setMinimumWidth(minWidth);
	cursor_readouts_ui->cursorT2->setMinimumWidth(minWidth);
	cursor_readouts_ui->timeDelta->setMinimumWidth(minWidth);
	label->setText("-999.999 MHz");
	minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->frequencyDelta->setMinimumWidth(minWidth);
	label->setText("-999.999 mV");
	minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->cursorV1->setMinimumWidth(minWidth);
	cursor_readouts_ui->cursorV2->setMinimumWidth(minWidth);
	cursor_readouts_ui->voltageDelta->setMinimumWidth(minWidth);
	delete label;

	QHBoxLayout *hLayout = static_cast<QHBoxLayout *>(
		measure_panel_ui->cursorReadouts->layout());
	if (hLayout)
		hLayout->insertWidget(0, cursorReadouts);

	fillCursorReadouts(plot.allCursorReadouts());
	measure_panel_ui->cursorReadouts->hide();
}

void Oscilloscope::statistics_panel_init()
{
	statisticsPanel = new QWidget(this);
	statistics_panel_ui = new Ui::StatisticsPanel();
	statistics_panel_ui->setupUi(statisticsPanel);
	QHBoxLayout *hLayout = new QHBoxLayout(statistics_panel_ui->statistics);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(25);

	// Make sure the scroll area knows beforehand the height of the content
	// that will be added to the statistics widget
	StatisticWidget *dummyStat = new StatisticWidget();
	statistics_panel_ui->statistics->setMinimumHeight(dummyStat->height());
	delete dummyStat;

	statisticsPanel->hide();
}

void Oscilloscope::statisticsUpdateValues()
{
	for (int i = 0; i < statistics_data.size(); i++) {
		if (!statistics_data[i].first->enabled())
			continue;
		double meas_data = statistics_data[i].first->value();
		statistics_data[i].second.pushNewData(meas_data);
	}
}

void Oscilloscope::statisticsReset()
{
	for (int i = 0; i < statistics_data.size(); i++)
		statistics_data[i].second.clear();
}

void Oscilloscope::statisticsUpdateGui()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->updateStatistics(statistics_data[i].second);
}

void Oscilloscope::statisticsUpdateGuiTitleColor()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->setTitleColor(plot.getLineColor(
			statistics[i]->channelId()));
}

void Oscilloscope::statisticsUpdateGuiPosIndex()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->setPositionIndex(i + 1);
}

void Oscilloscope::onStatisticActivated(int id, int chnIdx)
{
	std::shared_ptr<MeasurementData> pmd = plot.measurement(id, chnIdx);
	if (!pmd)
		return;

	statistics_data.push_back(QPair<std::shared_ptr<MeasurementData>,
		Statistic>(pmd, Statistic()));

	/* Add a widget for the new statistic */
	QWidget *statisticContainer = statistics_panel_ui->statistics;
	QHBoxLayout *hLayout = static_cast<QHBoxLayout *>
		(statisticContainer->layout());

	StatisticWidget *statistic = new StatisticWidget(statisticContainer);
	statistic->initForMeasurement(*pmd);
	statistic->setTitleColor(plot.getLineColor(chnIdx));
	statistic->setPositionIndex(statistics_data.size());
	statistic->updateStatistics(statistics_data.last().second);
	hLayout->addWidget(statistic);
}

void Oscilloscope::onStatisticDeactivated(int id, int chnIdx)
{
	std::shared_ptr<MeasurementData> pmd = plot.measurement(id, chnIdx);
	if (!pmd)
		return;

	QString name = pmd->name();

	auto it = std::find_if(statistics_data.begin(), statistics_data.end(),
		[&](QPair<std::shared_ptr<MeasurementData>, Statistic> pair) {
			return pair.first->name() == name &&
				pair.first->channel() == chnIdx;
		});
	if (it != statistics_data.end()) {
		statistics_data.erase(it);
	}

	/* remove the widget corresponding to the statistic we deleted */
	QList<StatisticWidget *>statistics = statistics_panel_ui->statistics->
		findChildren<StatisticWidget *>(QString("Statistic"),
		Qt::FindDirectChildrenOnly);

	auto stat_it = std::find_if(statistics.begin(), statistics.end(),
		[=](StatisticWidget *statistic){
			return statistic->title() == name &&
				statistic->channelId() == chnIdx;
		});
	if (stat_it != statistics.end()) {
		QWidget *w = static_cast<QWidget *> (*stat_it);
		statisticsPanel->hide(); // Avoid flickers
		statistics_panel_ui->statistics->layout()->removeWidget(w);
		delete w;
		statisticsPanel->show();

		statisticsUpdateGuiPosIndex();
	}
}

void Oscilloscope::onStatisticSelectionListChanged()
{
	// Clear all statistics in list
	statistics_data.clear();

	QList<StatisticWidget *>statistics = statistics_panel_ui->statistics->
		findChildren<StatisticWidget *>(QString("Statistic"),
		Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++) {
		statistics_panel_ui->statistics->layout()->removeWidget(
			statistics[i]);
		delete statistics[i];
	}

	// Use the new list from MeasureSettings
	auto newList = measure_settings->statisticSelection();
	for (int i = 0; i < newList.size(); i++)
		onStatisticActivated(newList[i].id(), newList[i].channel_id());
	statisticsUpdateGui();

}

void Oscilloscope::onStatisticsEnabled(bool on)
{
	statistics_enabled = on;
	statisticsPanel->setVisible(on);

	if (!on)
		statisticsReset();
	else
		statisticsUpdateGui();
}

void Oscilloscope::onStatisticsReset()
{
	statisticsReset();
	statisticsUpdateGui();
}

void Oscilloscope::singleCaptureDone()
{
	ui->pushButtonSingle->setChecked(false);
}

void Oscilloscope::onIioDataRefillTimeout()
{
	if (trigger_settings.triggerMode() == TriggerSettings::AUTO)
		trigger_is_forced = true;
}

void Oscilloscope::onPlotNewData()
{
	// Flag the new received data as Triggered or Untriggered.
	bool triggerOn = trigger_settings.triggerIsArmed();

	if (triggerOn && !trigger_is_forced)
		new_data_is_triggered = true;
	else
		new_data_is_triggered = false;

	// Reset the Forced Trigger flag.
	trigger_is_forced = false;

	// Update trigger status
	if (new_data_is_triggered)
		triggerUpdater->setInput(CapturePlot::Triggered);
	else
		triggerUpdater->setInput(CapturePlot::Auto);

	updateBufferPreviewer();
}

void Oscilloscope::onTriggerModeChanged(int mode)
{
	if (mode == 0) { // normal
		triggerUpdater->setIdleState(CapturePlot::Waiting);
		triggerUpdater->setInput(CapturePlot::Waiting);
	} else if (mode == 1) { // auto
		triggerUpdater->setIdleState(CapturePlot::Auto);
		triggerUpdater->setInput(CapturePlot::Auto);
	}
}

void Oscilloscope::updateBufferPreviewer()
{
	// Time interval within the plot canvas
	QwtInterval plotInterval = plot.axisInterval(QwtPlot::xBottom);

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, 0.0);
	long long triggerSamples = trigger_settings.triggerDelay();
	long long totalSamples = last_set_sample_count;

	if (totalSamples > 0) {
		dataInterval.setMinValue(triggerSamples / adc.sampleRate());
		dataInterval.setMaxValue((triggerSamples + totalSamples)
			/ adc.sampleRate());
	}

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
		fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
		fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();

	// Determine the cursor position
	QwtInterval containerInterval = (totalSamples > 0) ? dataInterval :
		fullInterval;
	double containerWidth = (totalSamples > 0) ? wWidth : 1;
	double containerPos = (totalSamples > 0) ? wPos : 0;
	double cPosInContainer = 1 - (containerInterval.maxValue() - 0) /
		containerInterval.width();
	double cPos = cPosInContainer * containerWidth + containerPos;

	// Update the widget
	buffer_previewer->setWaveformWidth(wWidth);
	buffer_previewer->setWaveformPos(wPos);
	buffer_previewer->setHighlightWidth(hWidth);
	buffer_previewer->setHighlightPos(hPos);
	buffer_previewer->setCursorPos(cPos);
}

void Oscilloscope::on_btnSettings_toggled(bool checked)
{
	QPushButton *btn = nullptr;

	if (checked) {
		if (last_non_general_settings_btn) {
			btn = last_non_general_settings_btn;
		} else { // search for the button of the first channel
			auto buttons = settings_group->buttons();
			for (int i = 0; i < buttons.size(); i++) {
				if (buttons[i]->property("id").toInt() == 0) {
					btn = static_cast<QPushButton *>(
					buttons[i]);
					break;
				}
			}
		}
	} else {
		btn = static_cast<QPushButton *>(
			settings_group->checkedButton());
	}

	if (btn) {
		toggleRightMenu(btn);
		btn->setChecked(checked);
	}
}

bool Oscilloscope_API::hasCursors() const
{
	return osc->cursor_ui->box->isChecked();
}

void Oscilloscope_API::setCursors(bool en)
{
	osc->cursor_ui->box->setChecked(en);
}

bool Oscilloscope_API::hasMeasure() const
{
	return osc->measure_ui->box->isChecked();
}

void Oscilloscope_API::setMeasure(bool en)
{
	osc->measure_ui->box->setChecked(en);
}

bool Oscilloscope_API::measureAll() const
{
	return osc->measure_settings->m_ui->button_measDisplayAll->isChecked();
}

void Oscilloscope_API::setMeasureAll(bool en)
{
	osc->measure_settings->m_ui->button_measDisplayAll->setChecked(en);
}

bool Oscilloscope_API::hasCounter() const
{
	return osc->measure_settings->m_ui->button_Counter->isChecked();
}

void Oscilloscope_API::setCounter(bool en)
{
	osc->measure_settings->m_ui->button_Counter->setChecked(en);
}

bool Oscilloscope_API::hasStatistics() const
{
	return osc->measure_settings->m_ui->button_StatisticsEn->isChecked();
}

void Oscilloscope_API::setStatistics(bool en)
{
	osc->measure_settings->m_ui->button_StatisticsEn->setChecked(en);
}

double Oscilloscope_API::cursorV1() const
{
	return osc->plot.value_v1;
}

double Oscilloscope_API::cursorV2() const
{
	return osc->plot.value_v2;
}

double Oscilloscope_API::cursorH1() const
{
	return osc->plot.value_h1;
}

double Oscilloscope_API::cursorH2() const
{
	return osc->plot.value_h2;
}

void Oscilloscope_API::setCursorV1(double val)
{
	osc->plot.d_vBar1->setPosition(val);
}

void Oscilloscope_API::setCursorV2(double val)
{
	osc->plot.d_vBar2->setPosition(val);
}

void Oscilloscope_API::setCursorH1(double val)
{
	osc->plot.d_hBar1->setPosition(val);
}

void Oscilloscope_API::setCursorH2(double val)
{
	osc->plot.d_hBar2->setPosition(val);
}

bool Oscilloscope_API::autoTrigger() const
{
	return osc->trigger_settings.ui->btnAuto->isChecked();
}

void Oscilloscope_API::setAutoTrigger(bool en)
{
	if (en)
		osc->trigger_settings.ui->btnAuto->setChecked(true);
	else
		osc->trigger_settings.ui->btnNormal->setChecked(true);
}
