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
#include <gnuradio/blocks/sub_ff.h>
#include <gnuradio/filter/iir_filter_ffd.h>

/* Qt includes */
#include <QtWidgets>
#include <QDebug>
#include <QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QSignalBlocker>
#include <QComboBox>

/* Local includes */
#include "adc_sample_conv.hpp"
#include "customPushButton.hpp"
#include "oscilloscope.hpp"
#include "dynamicWidget.hpp"
#include "measurement_gui.h"
#include "measure_settings.h"
#include "statistic_widget.h"
#include "state_updater.h"
#include "osc_capture_params.hpp"
#include "buffer_previewer.hpp"
#include "config.h"
#include "customplotpositionbutton.h"
#include "channel_widget.hpp"
#include "signal_sample.hpp"

/* Generated UI */
#include "ui_channel_settings.h"
#include "ui_cursors_settings.h"
#include "ui_osc_general_settings.h"
#include "ui_measure_panel.h"
#include "ui_measure_settings.h"
#include "ui_statistics_panel.h"
#include "ui_cursor_readouts.h"
#include "ui_oscilloscope.h"
#include "ui_trigger_settings.h"

#define MAX_MATH_CHANNELS 4

using namespace adiscope;
using namespace gr;
using namespace std;

Oscilloscope::Oscilloscope(struct iio_context *ctx, Filter *filt,
		std::shared_ptr<GenericAdc> adc, QPushButton *runButton,
		 QJSEngine *engine, ToolLauncher *parent) :
	Tool(ctx, runButton, new Oscilloscope_API(this), "Oscilloscope", parent),
	adc(adc),
	m2k_adc(dynamic_pointer_cast<M2kAdc>(adc)),
	nb_channels(Oscilloscope::adc->numAdcChannels()),
	active_sample_rate(adc->readSampleRate()),
	nb_math_channels(0),
	ui(new Ui::Oscilloscope),
	trigger_settings(adc),
	measure_settings(nullptr),
	plot(this, 16, 10),
	fft_plot(nb_channels, this),
	xy_plot(nb_channels / 2, this),
	hist_plot(nb_channels, this),
	ids(new iio_manager::port_id[nb_channels]),
	fft_ids(new iio_manager::port_id[nb_channels]),
	hist_ids(new iio_manager::port_id[nb_channels]),
	fft_is_visible(false), hist_is_visible(false), xy_is_visible(false),
	statistics_enabled(false),
	trigger_is_forced(false),
	new_data_is_triggered(false),
	triggerUpdater(new StateUpdater(250, this)),
	current_channel(-1), math_chn_counter(0),
	channels_group(new QButtonGroup(this)),
	zoom_level(0),
	current_ch_widget(-1),
	addChannel(true),
	index_x(0),
	index_y(1),
	ftc(nullptr),
	locked(false),
	triggerAcCoupled(false)
{
	ui->setupUi(this);
	int triggers_panel = ui->stackedWidget->insertWidget(-1, &trigger_settings);

	if (m2k_adc) {
		symmBufferMode = make_shared<SymmetricBufferMode>();
		symmBufferMode->setSampleRates(
			m2k_adc->availSamplRates().toVector().toStdVector());
		symmBufferMode->setEntireBufferMaxSize(500000); // max 0.5 mega-samples
		symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
		symmBufferMode->setTimeDivisionCount(plot.xAxisNumDiv());
	}

	/* Measurements Settings */
	measure_settings_init();

	fft_size = 1024;
	fft_plot.setNumPoints(0);

	last_set_sample_count = 0;
	last_set_time_pos = 0;

	/* Gnuradio Blocks */

	this->qt_time_block = adiscope::scope_sink_f::make(0, adc->sampleRate(),
		"Osc Time", nb_channels, (QObject *)&plot);

	this->qt_fft_block = adiscope::scope_sink_f::make(fft_size, adc->sampleRate(),
			"Osc Frequency", nb_channels, (QObject *)&fft_plot);

	this->qt_hist_block = adiscope::histogram_sink_f::make(1024, 100, 0, 20,
			"Osc Histogram", nb_channels, (QObject *)&hist_plot);

	this->qt_xy_block = adiscope::xy_sink_c::make(
			400, "Osc XY", nb_channels / 2, (QObject*)&xy_plot);

	this->qt_time_block->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	// Prevent the application from hanging while waiting for a trigger condition
	iio_context_set_timeout(ctx, UINT_MAX);

	plot.registerSink(qt_time_block->name(), nb_channels, 0);
	plot.disableLegend();

	fft_plot.disableLegend();
	fft_plot.setXaxisMouseGesturesEnabled(false);
	for (uint i = 0; i < nb_channels; i++)
		fft_plot.setYaxisMouseGesturesEnabled(i, false);

	iio = iio_manager::get_instance(ctx,
			filt->device_name(TOOL_OSCILLOSCOPE));
	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug() << "Manager created:\n" << gr::dot_graph(hier).c_str();

	auto adc_channels = adc->adcChannelList();
	for (unsigned int i = 0; i < adc_channels.size(); i++) {
		const char *id = iio_channel_get_name(adc_channels[i]);
		string s = "Channel ";
		if (!id) {
			s += to_string(i + 1);
			id = s.c_str();
		}

		ChannelWidget *ch_widget = new ChannelWidget(i, false, false,
			plot.getLineColor(i).name(), this);

		ch_widget->setFullName(id);
		ch_widget->setShortName(QString("CH %1").arg(i + 1));
		ch_widget->nameButton()->setText(ch_widget->shortName());

		connect(ch_widget, SIGNAL(enabled(bool)),
			SLOT(onChannelWidgetEnabled(bool)));
		connect(ch_widget, SIGNAL(selected(bool)),
			SLOT(onChannelWidgetSelected(bool)));
		connect(ch_widget, SIGNAL(menuToggled(bool)),
			SLOT(onChannelWidgetMenuToggled(bool)));

		ui->channelsList->addWidget(ch_widget);

		channels_group->addButton(ch_widget->nameButton());
		ui->settings_group->addButton(ch_widget->menuButton());

		QLabel *label= new QLabel(this);
		label->setText(vertMeasureFormat.format(
			plot.VertUnitsPerDiv(i), "V/div", 3));
		label->setStyleSheet(QString("QLabel {"
				"color: %1;"
				"font-weight: bold;"
				"}").arg(plot.getLineColor(i).name()));
		ui->chn_scales->addWidget(label);

		high_gain_modes.push_back(false);
		channel_offset.push_back(0.0);
		chnAcCoupled.push_back(false);
		filterBlocks.push_back(nullptr);
		subBlocks.push_back(nullptr);

		channels_api.append(new Channel_API(this));
	}
	triggerLevelSink = QPair<boost::shared_ptr<signal_sample>, int>(
				nullptr, -1);

	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	/* Cursors Settings */
	ui->btnCursors->setProperty("id", QVariant(-1));

	/* Trigger Settings */
	ui->btnTrigger->setProperty("id", QVariant(-triggers_panel));

	/* Trigger Status Updater */
	triggerUpdater->setOffState(CapturePlot::Stop);
	onTriggerModeChanged(trigger_settings.triggerMode());
	connect(triggerUpdater, SIGNAL(outputChanged(int)),
		&plot, SLOT(setTriggerState(int)));

	//plot.setZoomerEnabled(true);
	fft_plot.setZoomerEnabled();
	create_math_panel();

	/* Gnuradio Blocks Connections */

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	auto adc_samp_conv = gnuradio::get_initial_sptr(
			new adc_sample_conv(nb_channels, m2k_adc));
	if (m2k_adc) {
		adc_samp_conv->setCorrectionGain(0,
			m2k_adc->chnCorrectionGain(0));
		adc_samp_conv->setCorrectionGain(1,
			m2k_adc->chnCorrectionGain(1));
	}


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
	ui->vLayoutBufferSlot->addWidget(buffer_previewer);

	buffer_previewer->setCursorPos(0.5);

	/* Plot layout */

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
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
	plot.setSampleRate(adc->sampleRate(), 1, "");
	plot.setActiveVertAxis(0);

	for (unsigned int i = 0; i < nb_channels; i++) {
		plot.Curve(i)->setAxes(
				QwtAxisId(QwtPlot::xBottom, 0),
				QwtAxisId(QwtPlot::yLeft, i));
		plot.addZoomer(i);
		probe_attenuation.push_back(1);
	}

	plot.levelTriggerA()->setMobileAxis(QwtAxisId(QwtPlot::yLeft, 0));
	plot.setTriggerAEnabled(trigger_settings.analogEnabled());
	plot.levelTriggerA()->setPosition(trigger_settings.level());

	// TO DO: Give user the option to make these axes visible
	plot.enableAxis(QwtPlot::yLeft, false);
	plot.enableAxis(QwtPlot::xBottom, false);
	plot.setUsingLeftAxisScales(false);

	fft_plot.setMinimumHeight(250);
	fft_plot.setMinimumWidth(500);

	hist_plot.setMinimumHeight(200);
	hist_plot.setMinimumWidth(300);

	xy_plot.setMinimumHeight(50);
	xy_plot.setMinimumWidth(50);

	xy_plot.setVertUnitsPerDiv(5);
	xy_plot.setHorizUnitsPerDiv(5);

	xy_plot.disableLegend();

	xy_plot.setLineStyle(0, Qt::SolidLine);
	xy_plot.setLineMarker(0, QwtSymbol::NoSymbol);

	// Disable mouse interactions with the axes until we figure out if we want to use them
	xy_plot.setXaxisMouseGesturesEnabled(false);
	for (uint i = 0; i < nb_channels; i++)
		xy_plot.setYaxisMouseGesturesEnabled(i, false);

	// TO DO: refactor this once the source of the X and Y axes can be configured
	for(int i = 0; i < nb_channels + nb_math_channels; i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
			ui->channelsList->itemAt(i)->widget());
		ui->cmb_x_channel->addItem(cw->shortName());
		ui->cmb_y_channel->addItem(cw->shortName());
	}
	ui->cmb_x_channel->setCurrentIndex(0);
	ui->cmb_y_channel->setCurrentIndex(1);
	setup_xy_channels();

	xy_plot.setLineColor(0, QColor("#F8E71C"));

	ui->hlayout_fft->addWidget(&fft_plot);
	ui->container_fft_plot->hide();

	ui->gridLayoutHist->addWidget(&hist_plot, 0, 0);
	hist_plot.hide();

	QGridLayout *gridL = static_cast<QGridLayout *>(
		ui->xy_plot_container->layout());
	QWidget *w = gridL->itemAtPosition(1, 0)->widget();
	gridL->addWidget(&xy_plot, 1, 0);
	gridL->addWidget(w, 2, 0);

	connect(ui->cmb_x_channel, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setup_xy_channels()));
	connect(ui->cmb_y_channel, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setup_xy_channels()));

	ui->xy_plot_container->hide();

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
				-25,
				25);

	plot.setOffsetInterval(-25, 25);

	ch_ui = new Ui::ChannelSettings();
	ch_ui->setupUi(ui->channelSettings);

	ch_ui->horizontal->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	ch_ui->horizontal->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(1, voltsPerDiv, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(2, voltsPosition, 0, Qt::AlignLeft);

	init_channel_settings();

	timeBase->setValue(plot.HorizUnitsPerDiv());
	voltsPerDiv->setValue(plot.VertUnitsPerDiv());
	timePosition->setStep(timeBase->value() / 10);
	voltsPosition->setStep(voltsPerDiv->value() / 10);

	plot.setTimeBaseLabelValue(timeBase->value());

	/* General Settings Menu */
	gsettings_ui = new Ui::OscGeneralSettings();
	gsettings_ui->setupUi(ui->generalSettings);

	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

	gsettings_ui->Histogram_view->hide();

	connect(gsettings_ui->FFT_view, SIGNAL(toggled(bool)),
		SLOT(onFFT_view_toggled(bool)));
	connect(gsettings_ui->XY_view, SIGNAL(toggled(bool)),
		SLOT(onXY_view_toggled(bool)));
	connect(gsettings_ui->Histogram_view, SIGNAL(toggled(bool)),
		SLOT(onHistogram_view_toggled(bool)));

	connect(ui->pushButtonRunStop, SIGNAL(toggled(bool)), this,
			SLOT(runStopToggled(bool)));
	connect(ui->pushButtonSingle, SIGNAL(toggled(bool)), this,
			SLOT(runStopToggled(bool)));
	connect(runButton, SIGNAL(toggled(bool)), ui->pushButtonRunStop,
			SLOT(setChecked(bool)));
	connect(ui->pushButtonRunStop, SIGNAL(toggled(bool)), runButton,
			SLOT(setChecked(bool)));
	connect(this, SIGNAL(isRunning(bool)), runButton,
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
	connect(ch_ui->btnCoupled, SIGNAL(toggled(bool)),
		SLOT(onChannelCouplingChanged(bool)));

	connect(ch_ui->cmbChnLineWidth, SIGNAL(currentIndexChanged(int)),
		SLOT(channelLineWidthChanged(int)));

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
	connect(&trigger_settings, SIGNAL(sourceChanged(int)),
		SLOT(onTriggerSourceChanged(int)));
	connect(&trigger_settings, SIGNAL(analogTriggerEnabled(bool)),
		&plot, SLOT(setTriggerAEnabled(bool)));
	connect(&trigger_settings, SIGNAL(analogTriggerEnabled(bool)),
		this, SLOT(configureAcCouplingTrigger(bool)));
	connect(&trigger_settings, SIGNAL(levelChanged(double)),
		plot.levelTriggerA(), SLOT(setPosition(double)));
	connect(plot.levelTriggerA(), SIGNAL(positionChanged(double)),
		&trigger_settings, SLOT(setTriggerLevel(double)));

	connect(&trigger_settings, SIGNAL(levelChanged(double)),
		SLOT(onTriggerLevelChanged(double)));

	connect(&trigger_settings, SIGNAL(triggerModeChanged(int)),
		this, SLOT(onTriggerModeChanged(int)));

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

	// Set the first channel to be the selected channel (by default)
	ChannelWidget *chn0_widget = channelWidgetAtId(0);
	if (chn0_widget) {
		chn0_widget->nameButton()->setChecked(true);
	}

	// Default hysteresis levels for measurements
	for (int i = 0; i < nb_channels; i++)
		plot.setPeriodDetectHyst(i, 1.0 / 5);

	// Calculate initial sample count and sample rate
	onHorizScaleValueChanged(timeBase->value());
	onTimePositionChanged(timePosition->value());

	if (m2k_adc) {
		int crt_chn_copy = current_channel;
		int crt_chn_w_copy = current_ch_widget;
		for (int i = 0; i < nb_channels; i++) {
			current_channel = i;
			current_ch_widget = i;
			updateGainMode();

			auto adc_range = m2k_adc->inputRange(
					m2k_adc->chnHwGainMode(i));
			auto hyst_range = QPair<double, double>(
				0, adc_range.second / 10);
			double vscale = plot.VertUnitsPerDiv(i);
			trigger_settings.setTriggerLevelRange(i, adc_range);
			trigger_settings.setTriggerLevelStep(i, vscale);
			trigger_settings.setTriggerHystRange(i, hyst_range);
			trigger_settings.setTriggerHystStep(i, vscale / 10);
		}
		current_channel = crt_chn_copy;
		current_ch_widget = crt_chn_w_copy;
	}

	connect(plot.getZoomer(), &OscPlotZoomer::zoomIn, [=](){
		zoom_level++;
		plot.setTimeBaseZoomed(true);
	});
	connect(plot.getZoomer(), &OscPlotZoomer::zoomOut, [=](){
		if (zoom_level != 0) zoom_level--;
		if (zoom_level == 0)
			plot.setTimeBaseZoomed(false);
	});
	connect(plot.getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut){
		ChannelWidget *channel_widget = channelWidgetAtId(current_ch_widget);

		plot.setTimeBaseLabelValue(plot.HorizUnitsPerDiv());

		for (int i = 0; i < nb_channels + nb_math_channels; ++i) {
			QLabel *label = static_cast<QLabel *>(
						ui->chn_scales->itemAt(i)->widget());
			double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
			label->setText(vertMeasureFormat.format(value, "V/div", 3));
		}
	});

	connect(ch_ui->probe_attenuation, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](int index){
		double value = 0.1 * (std::pow(10, index));

		probe_attenuation[current_ch_widget] = value;
		if (current_channel == current_ch_widget) {
			plot.setDisplayScale(probe_attenuation[current_ch_widget]);
		}

		for (int i = 0; i < nb_channels + nb_math_channels; ++i) {
			QLabel *label = static_cast<QLabel *>(
						ui->chn_scales->itemAt(i)->widget());
			double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
			label->setText(vertMeasureFormat.format(value, "V/div", 3));
		}

		voltsPerDiv->setDisplayScale(probe_attenuation[current_ch_widget]);
		voltsPosition->setDisplayScale(probe_attenuation[current_ch_widget]);

		if (!runButton->isChecked() && plot.measurementsEnabled()) {
			measureUpdateValues();
		}

		onTriggerSourceChanged(trigger_settings.currentChannel());

	});

	export_settings_init();
	cursor_panel_init();
	setFFT_params(true);

	// The trigger is always available (cannot be disabled) and we add it to
	// the list so we can show in case all other menus are disabled
	menuOrder.push_back(ui->btnTrigger);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_OSCILLOSCOPE)));
	api->load(*settings);
	api->js_register(engine);

	plot.setDisplayScale(probe_attenuation[current_channel]);
	onTriggerSourceChanged(trigger_settings.currentChannel());
	for (int i = 0; i < nb_channels + nb_math_channels; ++i) {
		QLabel *label = static_cast<QLabel *>(
					ui->chn_scales->itemAt(i)->widget());
		double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
		label->setText(vertMeasureFormat.format(value, "V/div", 3));
	}

	if (!wheelEventGuard)
		wheelEventGuard = new MouseWheelWidgetGuard(ui->mainWidget);
	wheelEventGuard->installEventRecursively(ui->mainWidget);

	current_ch_widget = current_channel;

	connect(prefPanel, &Preferences::notify, this, &Oscilloscope::readPreferences);

	readPreferences();
}

void Oscilloscope::updateTriggerLevelValue(std::vector<float> value)
{
	if (!triggerLevelSink.first) {
		return;
	}
	triggerLevelSink.first->blockSignals(true);

	double m_dc_level = trigger_settings.dcLevel();
	int val = value.at(0) * 1e3;
	if ((int)(value.at(0)*1e3) == (int)(m_dc_level*1e3)) {
		triggerLevelSink.first->blockSignals(false);
		return;
	}
	if (trigger_settings.analogEnabled()) {
		trigger_settings.setDcLevelCoupled(value.at(0));
		trigger_settings.onSpinboxTriggerLevelChanged(
					trigger_settings.level());
	}
	triggerLevelSink.first->blockSignals(false);
}

Oscilloscope::~Oscilloscope()
{
	disconnect(prefPanel, &Preferences::notify, this, &Oscilloscope::readPreferences);


	ui->pushButtonRunStop->setChecked(false);

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
	if (started)
		iio->unlock();

	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug() << "OSC disconnected:\n" << gr::dot_graph(hier).c_str();

	api->save(*settings);
	delete api;

	filterBlocks.clear();
	subBlocks.clear();
	delete[] hist_ids;
	delete[] fft_ids;
	delete[] ids;
	delete ch_ui;
	delete gsettings_ui;
	delete measure_panel_ui;
	delete cursor_readouts_ui;
	delete cr_ui;
	delete ui;
}

void Oscilloscope::settingsLoaded()
{
	for (int i = 0; i < nb_channels + nb_math_channels; ++i)
		if (channelWidgetAtId(i)->menuButton()->isChecked()) {
			current_ch_widget = i;
			break;
		}

	disconnect(voltsPerDiv, SIGNAL(valueChanged(double)), this,
		SLOT(onVertScaleValueChanged(double)));
	voltsPerDiv->setValue(plot.VertUnitsPerDiv(current_ch_widget));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
}

void Oscilloscope::readPreferences()
{
	enableLabels(prefPanel->getOsc_labels_enabled());
}

void Oscilloscope::init_channel_settings()
{
	connect(ch_ui->btnEditMath, &QPushButton::toggled, this,
		&Oscilloscope::openEditMathPanel);

	connect(ui->btnAddMath, &QPushButton::toggled, [=](bool on){
		if (on && !addChannel) {
			addChannel = true;
			math_pair->first.btnAddChannel->setText("Add Channel");
			ch_ui->math_settings_widget->setVisible(false);
		}
	});
}

void Oscilloscope::activateAcCoupling(int i)
{
	double alpha, beta;
	bool trigger = (i == trigger_settings.currentChannel())
			&& trigger_settings.analogEnabled();
	if (trigger) {
		trigger_settings.setAcCoupled(true, i);
	}

	bool started = iio->started();
	if(started) {
		iio->lock();
	}

	if (active_sample_rate <= 1e6) {
		alpha = 1 / active_sample_rate;
	} else {
		alpha = 60 / (active_sample_rate + 60);
	}
	beta = 1 - alpha;

	filterBlocks[i] = gr::filter::iir_filter_ffd::make({alpha}, {1, -beta}, false);
	subBlocks[i] = gr::blocks::sub_ff::make();

	boost::shared_ptr<adc_sample_conv> block =
	dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);

	iio->disconnect(block, i, qt_time_block, i);
	iio->connect(block, i, subBlocks.at(i), 0);
	iio->connect(block, i, filterBlocks.at(i), 0);
	iio->connect(filterBlocks.at(i), 0, subBlocks.at(i), 1);
	iio->connect(subBlocks.at(i), 0, qt_time_block, i);

	if (trigger && !triggerLevelSink.first) {
		triggerLevelSink.first = boost::make_shared<signal_sample>();
		triggerLevelSink.second = i;
		keep_one = gr::blocks::keep_one_in_n::make(sizeof(float), 100);
		connect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		iio->connect(filterBlocks.at(i), 0, keep_one, 0);
		iio->connect(keep_one, 0, triggerLevelSink.first, 0);
	}

	for (auto pair = math_sinks.begin(); pair != math_sinks.end(); pair++) {
		auto math = pair.value().first;
		iio->disconnect(adc_samp_conv_block, i, math, i);
		iio->connect(subBlocks.at(i), 0, math, i);
	}

	for(int ch = 0; ch < nb_channels; ch++)
		iio->set_buffer_size(ids[ch], active_sample_count);

	if(started) {
		iio->unlock();
	}
}

void Oscilloscope::deactivateAcCoupling(int i)
{
	bool trigger = (i == trigger_settings.currentChannel())
			&& trigger_settings.analogEnabled();
	if (trigger) {
		trigger_settings.setAcCoupled(false, i);
	}

	bool started = iio->started();
	if(started) {
		iio->lock();
	}

	boost::shared_ptr<adc_sample_conv> block =
	dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);

	for (auto pair = math_sinks.begin(); pair != math_sinks.end(); pair++) {
		auto math = pair.value().first;
		iio->disconnect(subBlocks.at(i), 0, math, i);
		iio->connect(adc_samp_conv_block, i, math, i);
	}

	iio->disconnect(block, i, subBlocks.at(i), 0);
	iio->disconnect(block, i, filterBlocks.at(i), 0);
	iio->disconnect(filterBlocks.at(i), 0, subBlocks.at(i), 1);
	iio->disconnect(subBlocks.at(i), 0, qt_time_block, i);
	if (trigger && triggerLevelSink.first) {
		disconnect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));

		iio->disconnect(filterBlocks.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->disconnect(keep_one, 0, triggerLevelSink.first, 0);
		trigger_settings.updateHwVoltLevels(i);
		triggerLevelSink.first = nullptr;
		triggerLevelSink.second = -1;
		keep_one = nullptr;
	}
	filterBlocks[i] = nullptr;
	subBlocks[i] = nullptr;
	iio->connect(block, i, qt_time_block, i);

	for(int ch = 0; ch < nb_channels; ch++) {
		iio->set_buffer_size(ids[ch], active_sample_count);
	}

	if(started) {
		iio->unlock();
	}
}

void Oscilloscope::configureAcCouplingTrigger(bool enabled)
{
	int chIdx = trigger_settings.currentChannel();
	if (enabled && chnAcCoupled.at(chIdx)) {
		activateAcCouplingTrigger(chIdx);
	} else if (!enabled && chnAcCoupled.at(chIdx)) {
		deactivateAcCouplingTrigger();
	}
	triggerAcCoupled = enabled;
}

void Oscilloscope::activateAcCouplingTrigger(int chIdx)
{
	trigger_settings.setAcCoupled(true, chIdx);
	if (!triggerLevelSink.first) {
		bool started = iio->started();
		if (started) {
			iio->lock();
		}
		triggerLevelSink.first = boost::make_shared<signal_sample>();
		triggerLevelSink.second = chIdx;
		keep_one = gr::blocks::keep_one_in_n::make(sizeof(float), 100);
		connect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		iio->connect(filterBlocks.at(chIdx), 0, keep_one, 0);
		iio->connect(keep_one, 0, triggerLevelSink.first, 0);

		if (started) {
			iio->unlock();
		}
	}
}

void Oscilloscope::deactivateAcCouplingTrigger()
{
	trigger_settings.setAcCoupled(false, triggerLevelSink.second);
	if (triggerLevelSink.first) {
		bool started = iio->started();
		if (started) {
			iio->lock();
		}
		/* Disconnect the SLOT */
		disconnect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		/* Disconnect the GNU Radio block */
		iio->disconnect(filterBlocks.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->disconnect(keep_one, 0, triggerLevelSink.first, 0);
		trigger_settings.updateHwVoltLevels(triggerLevelSink.second);
		if (started) {
			iio->unlock();
		}
	}
	triggerLevelSink.first = nullptr;
	triggerLevelSink.second = -1;
	keep_one = nullptr;
}

void Oscilloscope::configureAcCoupling(int i, bool coupled)
{
	if (coupled && !chnAcCoupled.at(i)) {
		activateAcCoupling(i);
	} else if (!coupled && chnAcCoupled.at(i)) {
		deactivateAcCoupling(i);
	} else if (coupled && chnAcCoupled.at(i)) {
		deactivateAcCoupling(i);
		activateAcCoupling(i);
	}

	chnAcCoupled[i] = coupled;
}

void Oscilloscope::enableLabels(bool enable)
{
	plot.setUsingLeftAxisScales(enable);
	plot.enableLabels(enable);
	if (enable) {
		connect(&plot, &CapturePlot::repositionTimeTrigger, [=](){
			double value = timePosition->value();
			timePosition->setValue(0);
			timePosition->setValue(value);
		});
	} else {
		disconnect(&plot, &CapturePlot::repositionTimeTrigger, 0, 0);
	}
	if (enable) {
		bool allDisabled = true;
		for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
				ui->channelsList->itemAt(i)->widget());
			if (cw->enableButton()->isChecked()) {
				allDisabled = false;
			}
		}
		if (!allDisabled) {
			plot.setActiveVertAxis(current_channel);
			plot.enableAxisLabels(enable);
		}
	}
	if (!enable)
		plot.enableAxisLabels(enable);
}

void Oscilloscope::cursor_panel_init()
{
	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(ui->cursorsSettings);
	//cr_ui->posSelect->setStyleSheet("background-color:red;");
	cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		&plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		&plot, SLOT(setHorizCursorsEnabled(bool)));

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		cursor_readouts_ui->TimeCursors,
		SLOT(setVisible(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		cursor_readouts_ui->VoltageCursors,
		SLOT(setVisible(bool)));

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);
	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText("Transparency: " + QString::number(value) + "%");
		plot.setCursorReadoutsTransparency(value);
	});
	cr_ui->horizontalSlider->setSliderPosition(0);

	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		plot.moveCursorReadouts(position);
	});
}

void Oscilloscope::pause(bool paused)
{
	if (ui->pushButtonRunStop->isChecked()){
		toggle_blockchain_flow(!paused);
		trigger_settings.setAdcRunningState(!paused);
	}
}

void Oscilloscope::export_settings_init()
{

	exportSettings = new ExportSettings();
	exportSettings->enableExportButton(false);
	gsettings_ui->export_2->addWidget(exportSettings);

	for (int i = 0; i < nb_channels; ++i){
		exportSettings->addChannel(i, QString("Channel") +
					   QString::number(i + 1));
	}

	connect(exportSettings->getExportButton(), SIGNAL(clicked()), this,
		SLOT(btnExport_clicked()));
	connect(this, &Oscilloscope::activateExportButton,
		[=](){
		exportSettings->enableExportButton(true);
	});
}

void Oscilloscope::btnExport_clicked(){

	exportConfig = exportSettings->getExportConfig();
	pause(true);
	auto export_dialog( new QFileDialog( this ) );
	export_dialog->setWindowModality( Qt::WindowModal );
	export_dialog->setFileMode( QFileDialog::AnyFile );
	export_dialog->setAcceptMode( QFileDialog::AcceptSave );
	export_dialog->setNameFilters({"Comma-separated values files (*.csv)",
					       "Tab-delimited values files (*.txt)"});
	bool atleastOneChannelEnabled = false;
	for (auto x : exportConfig.keys())
		if (exportConfig[x]){
			atleastOneChannelEnabled = true;
			break;
		}
	if (!atleastOneChannelEnabled){
		return;
	}

	if (export_dialog->exec()){
		QString filter = export_dialog->selectedNameFilter();
		QFile f(export_dialog->selectedFiles().at(0));
		f.open(QIODevice::WriteOnly);
		QTextStream outputStream(&f);
		QString separator = "";
		if (filter.contains(".txt")){
			separator += "\t";
		} else if (filter.contains(".csv")){
			separator += ",";
		}

		//write header data
		outputStream << "Scopy Version:" << separator << QString(SCOPY_VERSION_GIT) << "\n";
		//TO DO: add more details for the device
		outputStream << "Device:" << separator << "M2K" << "\n";
		outputStream << "Generated on:" << separator << QDate::currentDate().toString("dddd MMMM dd/MM/yyyy") << "\n";
		//get nr of samples
		int samples = plot.Curve(0)->data()->size();
		outputStream << "Nr of samples:" << separator << QString::number(samples) << "\n";

		outputStream << "Sample" << separator;
		outputStream << "Time(s)" << separator;
		int channels_number = nb_channels + nb_math_channels;
		for (int i = 0; i < channels_number; ++i){
			if (exportConfig[i]){
				QString chNo = (i > 1) ? QString::number(i - 1) : QString::number(i + 1);
				outputStream << ((i > 1) ? "Math" : "Channel") + chNo + "(V)"
					<< ((i == channels_number - 1) ? "\n" : separator);
			} else {
				if (i == channels_number - 1){
					outputStream << "\n";
				}
			}
		}
		for (int i = 0; i < samples; ++i){
			outputStream << QString::number(i) << separator;
			outputStream << plot.Curve(0)->data()->sample(i).x() << separator;
			for (int j = 0; j < channels_number; ++j){
				if (exportConfig[j]){
					outputStream << plot.Curve(j)->data()->sample(i).y()
					<< ((j == channels_number - 1) ? "\n" : separator);
				} else {
					if (j == channels_number - 1){
						outputStream << "\n";
					}
				}
			}
		}
		f.close();
	}
	pause(false);
}

void Oscilloscope::create_math_panel()
{
	/* Math stuff */

	QWidget *panel = new QWidget(this);
	Ui::MathPanel math_ui;

	math_ui.setupUi(panel);
	QPushButton *btn = math_ui.btnAddChannel;

	Math *math = new Math(nullptr, nb_channels);

	math_pair = new QPair<Ui::MathPanel, Math*>(math_ui, math);

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
				if (addChannel) {
					QVariant var = btn->property("function");
					add_math_channel(var.toString().toStdString());
				} else {
					QVariant var = btn->property("function");
					editMathChannelFunction(current_ch_widget,
								var.toString().toStdString());
				}
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
	if (nb_math_channels == MAX_MATH_CHANNELS)
		return;

	auto math = iio::iio_math::make(function, nb_channels);
	unsigned int curve_id = nb_channels + nb_math_channels;
	unsigned int curve_number = find_curve_number();

	nb_math_channels++;
	probe_attenuation.push_back(1);

	QString qname = QString("Math %1").arg(math_chn_counter++);
	std::string name = qname.toStdString();

	auto math_sink = adiscope::scope_sink_f::make(
			noZoomXAxisWidth * adc->sampleRate(),
			adc->sampleRate(), name, 1, (QObject *)&plot);

	/* Add the math block and the math scope sink into a container, so that
	 * we can disconnect them when removing the math channel later */
	auto math_pair = QPair<gr::basic_block_sptr, gr::basic_block_sptr>(
				math, math_sink);
	math_sinks.insert(qname, math_pair);

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	math_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	for (unsigned int i = 0; i < nb_channels; i++) {
		if(subBlocks.at(i) != nullptr) {
			iio->connect(subBlocks.at(i), 0, math, i);
		} else {
			iio->connect(adc_samp_conv_block, i, math, i);
		}
	}
	iio->connect(math, 0, math_sink, 0);

	if (started)
		iio->unlock();

	plot.registerSink(name, 1,
			noZoomXAxisWidth *
			adc->sampleRate());

	ChannelWidget *channel_widget = new ChannelWidget(curve_id, true, false,
		plot.getLineColor(curve_id).name(), this);

	channel_widget->setMathChannel(true);
	channel_widget->setFunction(QString::fromStdString(function));

	channel_widget->setFullName(QString("Math %1").arg(curve_number + 1));
	channel_widget->setShortName(QString("M%1").arg(curve_number + 1));
	channel_widget->nameButton()->setText(channel_widget->shortName());

	exportSettings->addChannel(curve_id,
		channel_widget->fullName());

	channel_widget->setProperty("curve_nb", QVariant(curve_number));
	channel_widget->deleteButton()->setProperty(
		"curve_name", QVariant(qname));

	connect(channel_widget, SIGNAL(enabled(bool)),
		SLOT(onChannelWidgetEnabled(bool)));
	connect(channel_widget, SIGNAL(selected(bool)),
		SLOT(onChannelWidgetSelected(bool)));
	connect(channel_widget, SIGNAL(menuToggled(bool)),
		SLOT(onChannelWidgetMenuToggled(bool)));
	connect(channel_widget, SIGNAL(deleteClicked()),
		SLOT(onChannelWidgetDeleteClicked()));
	connect(channel_widget, &ChannelWidget::menuToggled,
		[=](bool on){
		if (!on)
			ch_ui->btnEditMath->setChecked(false);
	});

	ui->channelsList->addWidget(channel_widget);

	channels_group->addButton(channel_widget->nameButton());
	ui->settings_group->addButton(channel_widget->menuButton());

	QLabel *label= new QLabel(this);
		label->setText(vertMeasureFormat.format(
			plot.VertUnitsPerDiv(curve_id), "V/div", 3));
		label->setStyleSheet(QString("QLabel {"
				"color: %1;"
				"font-weight: bold;"
				"}").arg(plot.getLineColor(curve_id).name()));
		ui->chn_scales->addWidget(label);

	plot.Curve(curve_id)->setAxes(
			QwtAxisId(QwtPlot::xBottom, 0),
			QwtAxisId(QwtPlot::yLeft, curve_id));
	plot.addZoomer(curve_id);
	plot.replot();

	/* We added a Math channel that is enabled by default,
	 * so enable the Run button */
	updateRunButton(true);

	// Default hysteresis levels for measurements of the new channel
	plot.setPeriodDetectHyst(curve_id, 1.0 / 5);

	// Keep the current selected channels curve on top of the other ones
	if (isVisible())
		plot.bringCurveToFront(current_channel);

	if (nb_math_channels == MAX_MATH_CHANNELS ){
		if (ui->btnAddMath->isChecked()){
			ui->btnAddMath->setChecked(false);
		}
		ui->btnAddMath->hide();
		menuOrder.removeOne(ui->btnAddMath);
	}

	ui->cmb_x_channel->addItem(channel_widget->fullName());
	ui->cmb_y_channel->addItem(channel_widget->fullName());
	if(xy_is_visible)
		setup_xy_channels();

	plot.showYAxisWidget(curve_id, false);
}

void Oscilloscope::onChannelWidgetDeleteClicked()
{
	if (nb_math_channels - 1 < MAX_MATH_CHANNELS){
		ui->btnAddMath->show();
	}

	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	QAbstractButton *delBtn = cw->deleteButton();
	QString qname = delBtn->property("curve_name").toString();
	unsigned int curve_id = cw->id();

	probe_attenuation.removeAt(curve_id);
	if (curve_id == current_ch_widget &&
			cw->menuButton()->isChecked()) {
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (static_cast<CustomPushButton*>(cw->menuButton()), true));
		toggleRightMenu(static_cast<CustomPushButton*>(cw->menuButton()), false);
	}
	menuOrder.removeOne(static_cast<CustomPushButton*>(cw->menuButton()));

	/*If there are no more channels enabled, we should
	disable the measurements.*/
	bool shouldDisable = true;
	for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
			ui->channelsList->itemAt(i)->widget());
		if (curve_id == cw->id())
			continue;
		if (cw->enableButton()->isChecked())
			shouldDisable = false;
	}

	if (shouldDisable)
		measure_settings->disableDisplayAll();

	measure_settings->onChannelRemoved(curve_id);
	plot.unregisterSink(qname.toStdString());

	exportSettings->removeChannel(curve_id);
	exportConfig.remove(curve_id);

	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();
	locked = true;
	if(xy_is_visible) {
		ui->cmb_x_channel->blockSignals(true);
		ui->cmb_y_channel->blockSignals(true);
		ui->cmb_x_channel->removeItem(curve_id);
		ui->cmb_y_channel->removeItem(curve_id);
		if(index_x >= curve_id)
			ui->cmb_x_channel->setCurrentIndex(curve_id-1);
		if(index_y >= curve_id)
			ui->cmb_y_channel->setCurrentIndex(curve_id-1);
		ui->cmb_x_channel->blockSignals(false);
		ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();
	}
	/* Disconnect the blocks from the running flowgraph */
	auto pair = math_sinks.take(qname);
	for (unsigned int i = 0; i < nb_channels; i++) {
		if(subBlocks.at(i) != nullptr) {
			iio->disconnect(subBlocks.at(i), 0, pair.first, i);
		} else {
			iio->disconnect(adc_samp_conv_block, i, pair.first, i);
		}
	}
	iio->disconnect(pair.first, 0, pair.second, 0);

	if(xy_is_visible)
		setup_xy_channels();

	locked = false;
	if (started)
		iio->unlock();

	for (unsigned int i = curve_id + 1;
			i < nb_channels + nb_math_channels; i++) {
		ChannelWidget *w = static_cast<ChannelWidget *> (
			ui->channelsList->itemAt(i)->widget());

		/* Update the IDs */
		w->setId(i - 1);
	}
	nb_math_channels--;

	/* Remove the math channel from the bottom list of channels */
	ui->channelsList->removeWidget(cw);
	delete cw;

	if (curve_id < current_channel) {
		/*If the deleted math channel is before the current channel,
		we will need to update the current channel to its new value.*/
		current_channel -= 1;
		current_ch_widget -= 1;
		Q_EMIT selectedChannelChanged(current_channel);
		update_measure_for_channel(current_channel);
	} else if (curve_id == current_channel) {
		/*If the deleted channel is the current channel, check if we have
		enabled channels and select the first one we find
		else update the plots axis and zoomer properties from channel 0.*/
		bool channelsEnabled = false;
		for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
				ui->channelsList->itemAt(i)->widget());
			if (cw->enableButton()->isChecked()) {
				channelsEnabled = true;
				Q_EMIT selectedChannelChanged(0);
				update_measure_for_channel(0);
				cw->nameButton()->setChecked(true);
				Q_EMIT selectedChannelChanged(cw->id());
				update_measure_for_channel(cw->id());
				break;
			}
		}
		if (channelsEnabled) {
			if (curve_id < current_ch_widget)
				current_ch_widget -= 1;
		} else {
			Q_EMIT selectedChannelChanged(0);
		}
	} else if (curve_id < current_ch_widget) {
		current_ch_widget -= 1;
	}

	plot.setActiveVertAxis(current_channel);

	/* If the removed channel is before the current axis, we update the
	 * current axis to account for the index change */
	int current_axis = plot.activeVertAxis();
	if (current_axis > curve_id)
		plot.setActiveVertAxis(current_axis - 1);

	/* Before removing the axis, remove the offset widgets */
	plot.removeOffsetWidgets(curve_id);

	/* Remove the axis that corresponds to the curve we drop */
	plot.removeLeftVertAxis(curve_id);

	/* Remove scale label */
	QWidget *scale_lbl = ui->chn_scales->itemAt(curve_id)->widget();
	ui->chn_scales->removeWidget(scale_lbl);
	delete scale_lbl;

	for (unsigned int i = nb_channels;
			i < nb_channels + nb_math_channels; i++) {
		/* Update the curve-to-axis map */
		plot.Curve(i)->setAxes(
				QwtAxisId(QwtPlot::xBottom, 0),
				QwtAxisId(QwtPlot::yLeft, i));
	}
	plot.removeZoomer(curve_id);
	updateRunButton(false);
	plot.replot();
}

void Oscilloscope::clearMathChannels()
{
	while (nb_math_channels)
		channelWidgetAtId(2)->deleteButton()->click();
}

void Oscilloscope::on_actionClose_triggered()
{
	this->close();
}

void Oscilloscope::toggle_blockchain_flow(bool en)
{
	if (en) {
		if(active_sample_count < fft_size && fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->start(fft_ids[i]);

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->start(ids[i]);
		if (hist_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->start(hist_ids[i]);

		if(active_sample_count >= fft_size && fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->start(fft_ids[i]);

	} else {
		if(active_sample_count > fft_size && fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->stop(fft_ids[i]);

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->stop(ids[i]);
		if (hist_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->stop(hist_ids[i]);

		if(active_sample_count <= fft_size && fft_is_visible)
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->stop(fft_ids[i]);
	}
}

void Oscilloscope::runStopToggled(bool checked)
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	setDynamicProperty(btn, "running", checked);

	// When switching between continuous run and single or vice versa there
	// is no need to reconfigure anything besides the GUI of the buttons
	if (btn == ui->pushButtonSingle && ui->pushButtonRunStop->isChecked()) {
		ui->pushButtonRunStop->blockSignals(true);
		ui->pushButtonRunStop->setChecked(false);
		ui->pushButtonRunStop->blockSignals(false);
		setDynamicProperty(ui->pushButtonRunStop, "running",
			false);
		return;
	} else if (btn == ui->pushButtonRunStop &&
			ui->pushButtonSingle->isChecked()) {
		ui->pushButtonSingle->blockSignals(true);
		ui->pushButtonSingle->setChecked(false);
		ui->pushButtonSingle->blockSignals(false);
		setDynamicProperty(ui->pushButtonSingle, "running",
			false);
		return;
	}

	Q_EMIT activateExportButton();

	if (checked) {
		writeAllSettingsToHardware();

		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);
		if(fft_is_visible) {
			setFFT_params();
		}

		last_set_sample_count = active_sample_count;

		if (active_trig_sample_count !=
				trigger_settings.triggerDelay()) {
			trigger_settings.setTriggerDelay(
				active_trig_sample_count);
			last_set_time_pos = active_time_pos;
		}

		if (timePosition->value() != active_time_pos)
			timePosition->setValue(active_time_pos);

		toggle_blockchain_flow(true);
	} else {
		toggle_blockchain_flow(false);
		trigger_settings.setAdcRunningState(false);
	}

	// Update trigger status
	triggerUpdater->setEnabled(checked);
}

void Oscilloscope::setFFT_params(bool force)
{
	if(fft_plot.sampleRate() != adc->sampleRate() || force) {
		fft_plot.setSampleRate(adc->sampleRate(), 1, "");
		double start = 0;
		double stop =  adc->sampleRate() / 2;
		fft_plot.setAxisScale(QwtPlot::xBottom, start, stop);
		fft_plot.setAxisScale(QwtPlot::yLeft, -200, 0, 10);
		fft_plot.zoomBaseUpdate();
	}
}

void Oscilloscope::setChannelWidgetIndex(int chnIdx)
{
	current_ch_widget = chnIdx;
	plot.bringCurveToFront(chnIdx);
	for (unsigned int i = 0; i < nb_channels + nb_math_channels; ++i) {
		if (i == chnIdx)
			continue;
		plot.showYAxisWidget(i, false);
	}
}

void Oscilloscope::onFFT_view_toggled(bool visible)
{
	/* Lock the flowgraph if we are already started */
	bool started = iio->started();
	if (started)
		iio->lock();

	if (visible) {
		qt_fft_block->set_nsamps(fft_size);
		if (fft_is_visible) {
			for (unsigned int i = 0; i < nb_channels; i++)
				iio->disconnect(fft_ids[i]);
		}

		setFFT_params();
		for (unsigned int i = 0; i < nb_channels; i++) {
			auto fft = gnuradio::get_initial_sptr(
					new fft_block(false, fft_size));

			auto ctm = blocks::complex_to_mag_squared::make(1);

			/** GNU Radio flow: iio(i) ->  fft -> ctm -> qt_fft_block */
			fft_ids[i] = iio->connect(fft, i, 0, true);
			iio->connect(fft, 0, ctm, 0);
			iio->connect(ctm, 0, qt_fft_block, i);

			if (ui->pushButtonRunStop->isChecked())
				iio->start(fft_ids[i]);
		}

		for (unsigned int i = 0; i < nb_channels; i++) {
			iio->set_buffer_size(fft_ids[i], fft_size);
		}

		ui->container_fft_plot->show();
	} else {
		ui->container_fft_plot->hide();

		if (fft_is_visible) {
			for (unsigned int i = 0; i < nb_channels; i++) {
				iio->disconnect(fft_ids[i]);
			}
		}
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
	if (started && !locked)
		iio->lock();

	if (visible) {
		if(xy_is_visible && index_x == ui->cmb_x_channel->currentIndex()
				&& index_y == ui->cmb_y_channel->currentIndex())
		{
			xy_is_visible = visible;
			if (started && !locked)
				iio->unlock();
			return;
		}

		boost::shared_ptr<adc_sample_conv> block =
		dynamic_pointer_cast<adc_sample_conv>(
						adc_samp_conv_block);
		if (m2k_adc) {
			block->setCorrectionGain(0,
				m2k_adc->chnCorrectionGain(0));
			block->setCorrectionGain(1,
				m2k_adc->chnCorrectionGain(1));
		}

		if(!ftc)
			ftc = blocks::float_to_complex::make(1);

		if(xy_channels.size() > 0) {
			iio->disconnect(xy_channels.at(index_x).first,
					xy_channels.at(index_x).second,
					ftc, 0);
			iio->disconnect(xy_channels.at(index_y).first,
					xy_channels.at(index_y).second,
					ftc, 1);
			xy_channels.clear();
		}

		index_x = ui->cmb_x_channel->currentIndex();
		index_y = ui->cmb_y_channel->currentIndex();

		if(xy_channels.size() == 0) {
			for(unsigned int i = 0; i < nb_channels; i++)
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      adc_samp_conv_block, i));
			for(auto p : math_sinks) {
				auto math = p.first;
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      math, 0));
			}
		}

		iio->connect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
			     ftc, 0);
		iio->connect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
			     ftc, 1);

		if(!xy_is_visible)
			iio->connect(ftc, 0, this->qt_xy_block, 0);

		ui->xy_plot_container->show();
	} else {
		ui->xy_plot_container->hide();
		// Disconnect the XY section from the running flowgraph
		iio->disconnect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
				ftc, 0);
		iio->disconnect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
				ftc, 1);

		iio->disconnect(ftc, 0, this->qt_xy_block, 0);

		xy_channels.clear();
	}

	xy_is_visible = visible;

	if (started && !locked)
		iio->unlock();
}

void adiscope::Oscilloscope::on_boxCursors_toggled(bool on)
{
	plot.setHorizCursorsEnabled(
			on ? cr_ui->vCursorsEnable->isChecked() : false);
	plot.setVertCursorsEnabled(
			on ? cr_ui->hCursorsEnable->isChecked() : false);

	// Set the visibility of the cursor readouts owned by the Oscilloscope
	if (on) {
		plot.setCursorReadoutsVisible(!ui->boxMeasure->isChecked());
	} else {
		if (ui->btnCursors->isChecked())
			ui->btnCursors->setChecked(false);

		menuOrder.removeOne(ui->btnCursors);
	}

	measure_panel_ui->cursorReadouts->setVisible(on);
}

void adiscope::Oscilloscope::on_boxMeasure_toggled(bool on)
{
	if (on) {
		update_measure_for_channel(current_channel);
	} else {
		if (ui->btnMeasure->isChecked())
			ui->btnMeasure->setChecked(false);

		menuOrder.removeOne(ui->btnMeasure);
	}

	measurePanel->setVisible(on);
	statisticsPanel->setVisible(on && statistics_enabled);

	// Set the visibility of the cursor readouts owned by the plot
	if (ui->boxCursors->isChecked())
		plot.setCursorReadoutsVisible(!on);
}

void Oscilloscope::onTriggerSourceChanged(int chnIdx)
{
	plot.levelTriggerA()->setMobileAxis(QwtAxisId(QwtPlot::yLeft, chnIdx));
	trigger_settings.setChannelAttenuation(probe_attenuation[chnIdx]);
	if (chnAcCoupled.at(chnIdx)) {
		deactivateAcCouplingTrigger();
		activateAcCouplingTrigger(chnIdx);
	}

}

void Oscilloscope::onTimeTriggerDelayChanged(double value)
{
	if (timePosition->value() != value)
		Q_EMIT triggerPositionChanged(value);
}

void Oscilloscope::onTriggerLevelChanged(double value)
{
	int trigger_chn = trigger_settings.currentChannel();

	if (trigger_chn > -1)
		plot.setPeriodDetectLevel(trigger_chn, value);
}

void Oscilloscope::comboBoxUpdateToValue(QComboBox *box, double value, std::vector<double>list)
{
	int i = find_if( list.begin(), list.end(),
				[&value](const double element) {return element == value;} ) - list.begin();
	if (i < list.size())
		box->setCurrentIndex(i);
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
	run_button->setEnabled(ch_enabled);
	ui->pushButtonSingle->setEnabled(ch_enabled);

	if (!ch_enabled) {
		ui->pushButtonRunStop->setChecked(false);
		run_button->setChecked(false);
		ui->pushButtonSingle->setChecked(false);
	}
}

void adiscope::Oscilloscope::onChannelWidgetEnabled(bool en)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();

	if (en) {
		plot.AttachCurve(id);
		fft_plot.AttachCurve(id);
		plot.showYAxisWidget(id, en);
		bool shouldActivate = true;

		for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
				ui->channelsList->itemAt(i)->widget());
			if (id == cw->id())
				continue;
			if (cw->enableButton()->isChecked())
				shouldActivate = false;
		}

		if (shouldActivate) {
			Q_EMIT selectedChannelChanged(id);
			update_measure_for_channel(id);
			measure_settings->activateDisplayAll();
		}

	} else {
		plot.DetachCurve(id);
		fft_plot.DetachCurve(id);
		plot.showYAxisWidget(id, en);
		bool shouldDisable = true;

		for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
				ui->channelsList->itemAt(i)->widget());
			if (id == cw->id())
				continue;
			if (cw->enableButton()->isChecked())
				shouldDisable = false;
		}

		if (shouldDisable)
			measure_settings->disableDisplayAll();

		if (current_channel == id) {
			for (int i = 0; i < nb_channels + nb_math_channels; i++) {
				ChannelWidget *cw = static_cast<ChannelWidget *>(
					ui->channelsList->itemAt(i)->widget());
				if (cw->enableButton()->isChecked()) {
					cw->nameButton()->setChecked(true);
					break;
				}
			}
		}

		menuOrder.removeOne(static_cast<CustomPushButton *>(
			w->menuButton()));
	}

	plot.setOffsetWidgetVisible(id, en);

	plot.replot();
	fft_plot.replot();
	updateRunButton(en);
}

void adiscope::Oscilloscope::onChannelWidgetSelected(bool checked)
{
	if (!checked) {
		return;
	}

	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();

	if (id != current_channel) {
		current_channel = id;
		Q_EMIT selectedChannelChanged(id);
		plot.bringCurveToFront(id);
		plot.setActiveVertAxis(id);
		plot.setDisplayScale(probe_attenuation[id]);
	}

	if (plot.measurementsEnabled()) {
		update_measure_for_channel(id);
	}
}

void adiscope::Oscilloscope::onChannelWidgetMenuToggled(bool checked)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	current_ch_widget = cw->id();

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(cw->menuButton()), checked);
}

void Oscilloscope::cancelZoom()
{
	zoom_level = 0;
	plot.cancelZoom();

	for (int i = 0; i < nb_channels + nb_math_channels; ++i) {
		QLabel *label = static_cast<QLabel *>(
					ui->chn_scales->itemAt(i)->widget());
		double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
		label->setText(vertMeasureFormat.format(value, "V/div", 3));
	}
}

void adiscope::Oscilloscope::onChannelCouplingChanged(bool en)
{
	if (en && chnAcCoupled.at(current_ch_widget))
		return;
	configureAcCoupling(current_ch_widget, en);
}

void adiscope::Oscilloscope::onVertScaleValueChanged(double value)
{
	cancelZoom();
	if (value != plot.VertUnitsPerDiv(current_ch_widget)) {
		plot.setVertUnitsPerDiv(value, current_ch_widget);
		plot.replot();
		plot.zoomBaseUpdate();
	}
	voltsPosition->setStep(value / 10);

	// TO DO: refactor this once the source of the X and Y axes can be configured
	if (current_ch_widget == index_x) {
		xy_plot.setHorizUnitsPerDiv(value);
	}
	if (current_ch_widget == index_y) {
		xy_plot.setVertUnitsPerDiv(value, QwtPlot::yLeft);
	}
	xy_plot.replot();
	xy_plot.zoomBaseUpdate();

	if (current_ch_widget < adc->getTrigger()->numChannels()) {
		trigger_settings.setTriggerLevelStep(current_ch_widget, value);
		trigger_settings.setTriggerHystStep(current_ch_widget, value / 10);
	}

	// Send scale information to the measure object
	plot.setPeriodDetectHyst(current_ch_widget, value / 5);

	QLabel *label = static_cast<QLabel *>(
			ui->chn_scales->itemAt(current_ch_widget)->widget());
	double labelValue = probe_attenuation[current_ch_widget]
			* plot.VertUnitsPerDiv(current_ch_widget);
	label->setText(vertMeasureFormat.format(labelValue, "V/div", 3));

	// Switch between high and low gain modes only for the M2K channels
	if (m2k_adc && current_ch_widget < nb_channels) {
		updateGainMode();
		setChannelHwOffset(current_ch_widget,
			voltsPosition->value());
		trigger_settings.updateHwVoltLevels(current_ch_widget);
	}
}

void adiscope::Oscilloscope::onHorizScaleValueChanged(double value)
{
	cancelZoom();
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
	plot.zoomBaseUpdate();

	if (zoom_level == 0) {
		noZoomXAxisWidth = plot.axisInterval(QwtPlot::xBottom).width();
	}

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	bool started = iio->started();
	if (started)
		iio->lock();
	setAllSinksSampleCount(active_sample_count);

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);
		last_set_sample_count = active_sample_count;

		adc->setSampleRate(active_sample_rate);
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;

		// Time base changes can limit the time position value
		if (timePosition->value() != -params.timePos)
			timePosition->setValue(-params.timePos);
	}

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], active_sample_count);
	}

	/* timeout = how long a buffer capture takes + transmission latency. The
	latter is a guessed value. If we could get a feedback from hardware that
	the acquisition has been made and it's on the way then we can drop this
	approach. */
	iio->set_device_timeout((active_sample_count / active_sample_rate) * 1000 + 100);

	if (started)
		iio->unlock();

	for(int ch = 0; ch < nb_channels; ch++) {
		configureAcCoupling(ch, chnAcCoupled.at(ch));
	}


	// Compute the appropriate value for fft_size
	double power = ceil(log2(active_sample_count));
	fft_size = pow(2, power);
	onFFT_view_toggled(fft_is_visible);

	// Change the sensitivity of time position control
	timePosition->setStep(value / 10);

	updateBufferPreviewer();
}

void adiscope::Oscilloscope::onVertOffsetValueChanged(double value)
{
	if (value != -plot.VertOffset(current_ch_widget)) {
		plot.setVertOffset(-value, current_ch_widget);
		plot.replot();
	}

	// Switch between high and low gain modes only for the M2K channels
	if (m2k_adc && current_ch_widget < nb_channels) {
		if (ui->pushButtonRunStop->isChecked())
			toggle_blockchain_flow(false);

		updateGainMode();
		setChannelHwOffset(current_ch_widget, value);

		trigger_settings.updateHwVoltLevels(current_ch_widget);

		if (ui->pushButtonRunStop->isChecked())
			toggle_blockchain_flow(true);
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

	if (zoom_level == 0)
		plot.zoomBaseUpdate();

	if (started) {
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;
	}

	updateBufferPreviewer();

	if (active_sample_rate == adc->sampleRate() &&
			(active_sample_count == oldSampleCount))
		return;

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	auto chnCoupled = chnAcCoupled;
	for(int ch = 0; ch < nb_channels; ch++) {
		if (chnAcCoupled.at(ch)) {
			configureAcCoupling(ch, !chnAcCoupled.at(ch));
		}
	}
	if (started)
		iio->lock();
	setAllSinksSampleCount(active_sample_count);

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);

		last_set_sample_count = active_sample_count;

		adc->setSampleRate(active_sample_rate);
	}

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], active_sample_count);
	}

	if (started)
		iio->unlock();

	for(int ch = 0; ch < nb_channels; ch++) {
		if (chnCoupled.at(ch)) {
			configureAcCoupling(ch, chnCoupled.at(ch));
		}
	}

	// Compute the appropriate value for fft_size
	double power = ceil(log2(active_sample_count));
	fft_size = pow(2, power);
	onFFT_view_toggled(fft_is_visible);
}

void adiscope::Oscilloscope::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void adiscope::Oscilloscope::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int id = btn->property("id").toInt();

	if (id != -ui->stackedWidget->indexOf(ui->generalSettings)){
		if (!menuOrder.contains(btn)){
			menuOrder.push_back(btn);
		} else {
			menuOrder.removeOne(btn);
			menuOrder.push_back(btn);
		}
	}

	if (checked)
		settings_panel_update(id);

	if (id >= 0) {
		plot.setActiveVertAxis(id, false);
		if (checked) {
			update_chn_settings_panel(id);
		}
	}

	// Update Settings button state
	if(!ui->btnGeneralSettings->isChecked())
		ui->btnSettings->setChecked(!!ui->settings_group->checkedButton());

	ui->rightMenu->toggleMenu(checked);
}

void Oscilloscope::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
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
	voltsPosition->setValue(-plot.VertOffset(current_ch_widget));
}

ChannelWidget * Oscilloscope::channelWidgetAtId(int id)
{
	ChannelWidget *w = nullptr;
	bool found = false;

	for (unsigned int i = 0; !found &&
				i < nb_channels + nb_math_channels; i++) {

		w = static_cast<ChannelWidget *>(
			ui->channelsList->itemAt(i)->widget());
		found = w->id() == id;
	}

	return w;
}

void Oscilloscope::update_chn_settings_panel(int id)
{
	ChannelWidget *chn_widget = channelWidgetAtId(id);
	if (!chn_widget)
		return;

	disconnect(voltsPerDiv, SIGNAL(valueChanged(double)), this,
		SLOT(onVertScaleValueChanged(double)));
	voltsPerDiv->setValue(plot.VertUnitsPerDiv(id));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
	voltsPosition->setValue(-plot.VertOffset(id));

	QString name = chn_widget->fullName();
	ch_ui->label_channelName->setText(name);
	QString stylesheet = QString("border: 2px solid %1"
					).arg(plot.getLineColor(id).name());
	ch_ui->line_channelColor->setStyleSheet(stylesheet);
	int cmbIdx = (int)(plot.getLineWidthF(id) / 0.5) - 1;
	ch_ui->cmbChnLineWidth->setCurrentIndex(cmbIdx);

	if (chn_widget->isMathChannel()) {
		ch_ui->math_settings_widget->setVisible(true);
		ch_ui->function_2->setText(chn_widget->function());
		ch_ui->function_2->setText(chn_widget->function());
		ch_ui->wCoupling->setVisible(false);
	} else {
		ch_ui->math_settings_widget->setVisible(false);
		ch_ui->wCoupling->setVisible(true);
		if (ch_ui->btnCoupled->isChecked() != chnAcCoupled.at(id)) {
			ch_ui->btnCoupled->setChecked(chnAcCoupled.at(id));
		}
	}

	if (chn_widget->isMathChannel()) {
		ch_ui->probe_attenuation->setVisible(false);
		ch_ui->probe_label->setVisible(false);
	} else {
		int index = 0;
		double value = probe_attenuation[id];
		while (value > 0.1) {
			value /= 10;
			index++;
		}
		ch_ui->probe_attenuation->setCurrentIndex(index);
		ch_ui->probe_attenuation->setVisible(true);
		ch_ui->probe_label->setVisible(true);
	}

	voltsPerDiv->setDisplayScale(probe_attenuation[id]);
	voltsPosition->setDisplayScale(probe_attenuation[id]);
}

void Oscilloscope::openEditMathPanel(bool on)
{
	ChannelWidget *chn_widget = channelWidgetAtId(current_ch_widget);

	if (on) {
		addChannel = false;
		triggerRightMenuToggle(
			static_cast<CustomPushButton* >(chn_widget->menuButton()), false);
		math_pair->first.btnAddChannel->setText("Save");
		math_pair->second->setFunction(chn_widget->function());
		triggerRightMenuToggle(
			static_cast<CustomPushButton* >(ui->btnAddMath), true);
	}
}

void Oscilloscope::editMathChannelFunction(int id, const std::string& new_function)
{
	ChannelWidget *chn_widget = channelWidgetAtId(id);
	int current_x = index_x;
	int current_y = index_y;

	triggerRightMenuToggle(
		static_cast<CustomPushButton* >(ui->btnAddMath), false);
	triggerRightMenuToggle(
		static_cast<CustomPushButton* >(chn_widget->menuButton()), true);
	math_pair->first.btnAddChannel->setText("Add Channel");
	math_pair->second->setFunction("");
	addChannel = true;
	ch_ui->btnEditMath->setChecked(false);

	if (chn_widget->function().toStdString() == new_function)
		return;

	QString qname = chn_widget->deleteButton()->property("curve_name").toString();
	std::string name = qname.toStdString();

	auto math = iio::iio_math::make(new_function, nb_channels);

	bool started = iio->started();
	if (started)
		iio->lock();
	locked = true;
	if(xy_is_visible) {
		ui->cmb_x_channel->blockSignals(true);
		ui->cmb_y_channel->blockSignals(true);
		if(index_x == id)
			ui->cmb_x_channel->setCurrentIndex(id-1);
		if(index_y == id)
			ui->cmb_y_channel->setCurrentIndex(id-1);
		ui->cmb_x_channel->blockSignals(false);
		ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();
	}
	auto pair = math_sinks.value(qname);
	for (unsigned int i = 0; i < nb_channels; ++i) {
		if(subBlocks.at(i) != nullptr) {
			iio->disconnect(subBlocks.at(i), 0, pair.first, i);
		} else {
			iio->disconnect(adc_samp_conv_block, i, pair.first, i);
		}
	}
	iio->disconnect(pair.first, 0, pair.second, 0);

	auto math_pair = QPair<gr::basic_block_sptr, gr::basic_block_sptr>(
				math, pair.second);

	math_sinks.insert(qname, math_pair);

	for (unsigned int i = 0; i < nb_channels; ++i) {
		if(subBlocks.at(i) != nullptr) {
			iio->connect(subBlocks.at(i), 0, math, i);
		} else {
			iio->connect(adc_samp_conv_block, i, math, i);
		}
	}
	iio->connect(math, 0, pair.second, 0);

	if(xy_is_visible) {
		ui->cmb_x_channel->blockSignals(true);
		ui->cmb_y_channel->blockSignals(true);
		ui->cmb_x_channel->setCurrentIndex(current_x);
		ui->cmb_y_channel->setCurrentIndex(current_y);
		ui->cmb_x_channel->blockSignals(false);
		ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();
	}
	locked = false;
	if (started)
		iio->unlock();

	// If the oscilloscope is not running, create a single run situation only for
	// the channel that is edited .
//	else {
//		QList<QString> keys = math_sinks.keys();
//		for (const QString &key : keys)
//			if (key != qname) {
//				pair = math_sinks.value(key);
//				for (unsigned int i = 0; i < nb_channels; ++i)
//					iio->disconnect(adc_samp_conv_block, i, pair.first, i);
//				iio->disconnect(pair.first, 0, pair.second, 0);
//			}

//		ui->pushButtonSingle->setChecked(true);

//		for (const QString &key : keys)
//			if (key != qname) {
//				pair = math_sinks.value(key);
//				for (unsigned int i = 0; i < nb_channels; ++i)
//					iio->connect(adc_samp_conv_block, i, pair.first, i);
//				iio->connect(pair.first, 0, pair.second, 0);
//			}
//	}

	chn_widget->setFunction(QString::fromStdString(new_function));
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
	ChannelWidget *chn_widget = channelWidgetAtId(ch_idx);

	measure_settings->setChannelName(chn_widget->fullName());
	measure_settings->setChannelUnderlineColor(chn_widget->color());
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

	QGridLayout *gLayout = new QGridLayout(container);

	gLayout->setContentsMargins(0, 0, 0, 0);
	gLayout->setVerticalSpacing(5);
	gLayout->setHorizontalSpacing(5);
	int max_rows = 4;
	int nb_meas_added = 0;

	for (int i = 0; i < measurements_data.size(); i++) {

		int channel = measurements_data[i]->channel();

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
		measurements_gui[i]->update(*(measurements_data[i]),
					    probe_attenuation[channel]);
		measurements_gui[i]->setLabelsColor(plot.getLineColor(channel));

		nb_meas_added++;
	}
}

void Oscilloscope::measureUpdateValues()
{
	for (int i = 0; i < measurements_data.size(); i++) {
		int channel = measurements_data[i]->channel();
		measurements_gui[i]->update(*(measurements_data[i]),
					    probe_attenuation[channel]);
	}
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

	ui->btnMeasure->setProperty("id", QVariant(-measure_panel));

	connect(ui->boxMeasure, SIGNAL(toggled(bool)),
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

	connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
		measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setRange);

	connect(measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::valueChanged,
		measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::setValue);
	connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
		measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setValue);

	connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
		[=](double v1, double v2){
		measure_panel_ui->scrollArea_2->widget()->setFixedWidth(measure_panel_ui->scrollAreaWidgetContents->width());
	});

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
	measure_panel_ui->scrollArea->setMinimumHeight(label->height() * 4);

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

	if (!ui->boxMeasure->isChecked())
		statisticsPanel->hide();
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
		if (statistics_enabled) // Avoid flickers when panel is visible
			statisticsPanel->hide(); // Avoid flickers
		statistics_panel_ui->statistics->layout()->removeWidget(w);
		if (statistics_enabled) // Avoid flickers when panel is visible
			statisticsPanel->show();
		delete w;

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
	Q_EMIT activateExportButton();
	if (ui->pushButtonSingle->isChecked()){
		ui->pushButtonSingle->setChecked(false);
		Q_EMIT isRunning(false);
	}

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
		dataInterval.setMinValue(triggerSamples / adc->sampleRate());
		dataInterval.setMaxValue((triggerSamples + totalSamples)
			/ adc->sampleRate());
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

void Oscilloscope::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;

	if (checked && !menuOrder.isEmpty()) {
		btn = menuOrder.back();
		menuOrder.pop_back();
	} else {
		btn = static_cast<CustomPushButton *>(
			ui->settings_group->checkedButton());
	}

	btn->setChecked(checked);
}

void Oscilloscope::updateGainMode()
{
	QwtInterval hw_input_itv(-2.5, 2.5);
	QwtInterval plot_vert_itv = plot.axisScaleDiv(
		QwtAxisId(QwtPlot::yLeft, current_ch_widget)).interval();

	// If max signal span that can be captured is smaller than the plot
	// screen try to increase the range (switch to low gain mode)
	if (plot_vert_itv.minValue() < hw_input_itv.minValue() ||
		plot_vert_itv.maxValue() > hw_input_itv.maxValue()) {
		if (high_gain_modes[current_ch_widget]) {
			high_gain_modes[current_ch_widget] = false;
			bool running = ui->pushButtonRunStop->isChecked();

			if (running)
				toggle_blockchain_flow(false);
			setGainMode(current_ch_widget, M2kAdc::LOW_GAIN_MODE);
			if (running)
				toggle_blockchain_flow(true);

			auto adc_range = m2k_adc->inputRange(
					M2kAdc::LOW_GAIN_MODE);
			trigger_settings.setTriggerLevelRange(current_ch_widget,
				adc_range);
			auto hyst_range = QPair<double, double>(
				0, adc_range.second / 10);
			trigger_settings.setTriggerHystRange(current_ch_widget,
				hyst_range);
		}
	} else {
		if (!high_gain_modes[current_ch_widget]) {
			high_gain_modes[current_ch_widget] = true;
			bool running = ui->pushButtonRunStop->isChecked();

			if (running)
				toggle_blockchain_flow(false);
			setGainMode(current_ch_widget, M2kAdc::HIGH_GAIN_MODE);
			if (running)
				toggle_blockchain_flow(true);

			auto adc_range = m2k_adc->inputRange(
					M2kAdc::LOW_GAIN_MODE);
			trigger_settings.setTriggerLevelRange(current_ch_widget,
				adc_range);
			auto hyst_range = QPair<double, double>(
				0, adc_range.second / 10);
			trigger_settings.setTriggerHystRange(current_ch_widget,
				hyst_range);
		}
	}
}

void Oscilloscope::setGainMode(uint chnIdx, M2kAdc::GainMode gain_mode)
{
	if (ui->pushButtonRunStop->isChecked())
		m2k_adc->setChnHwGainMode(chnIdx, gain_mode);

	boost::shared_ptr<adc_sample_conv> block =
	dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);

	block->setHardwareGain(chnIdx, m2k_adc->gainAt(gain_mode));
	trigger_settings.updateHwVoltLevels(chnIdx);
}

void Oscilloscope::setChannelHwOffset(uint chnIdx, double offset)
{
	channel_offset[current_channel] = offset;
	if (ui->pushButtonRunStop->isChecked())
		m2k_adc->setChnHwOffset(chnIdx, offset);

	// Compensate the offset set in hardware
	boost::shared_ptr<adc_sample_conv> block =
		dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);
	block->setOffset(chnIdx, -offset);
}

void Oscilloscope::setAllSinksSampleCount(unsigned long sample_count)
{
	this->qt_time_block->set_nsamps(sample_count);
	this->qt_xy_block->set_nsamps(sample_count);

	auto it = math_sinks.constBegin();
	while (it != math_sinks.constEnd()) {
		scope_sink_f::sptr math_sink = dynamic_pointer_cast<
				scope_sink_f>(it.value().second);
		math_sink->set_nsamps(sample_count);
		++it;
	}
	this->qt_fft_block->set_nsamps(fft_size);
}

void Oscilloscope::writeAllSettingsToHardware()
{
	// Sample Rate
	if (active_sample_rate != adc->sampleRate())
		adc->setSampleRate(active_sample_rate);

	// Offset and Gain
	if (m2k_adc) {
		for (uint i = 0; i < nb_channels; i++) {
			m2k_adc->setChnHwOffset(i, channel_offset[i]);

			M2kAdc::GainMode mode = high_gain_modes[i] ?
				M2kAdc::HIGH_GAIN_MODE : M2kAdc::LOW_GAIN_MODE;
			m2k_adc->setChnHwGainMode(i, mode);
		}

		iio_device_attr_write_longlong(adc->iio_adc_dev(),
			"oversampling_ratio", 1);
	}

	// Writes all trigger settings to hardware
	trigger_settings.setAdcRunningState(true);
}

void Oscilloscope::on_xyPlotLineType_toggled(bool checked)
{
	if (checked) {
		xy_plot.setLineStyle(0, Qt::NoPen);
		xy_plot.setLineMarker(0, QwtSymbol::Ellipse);
	} else {
		xy_plot.setLineStyle(0, Qt::SolidLine);
		xy_plot.setLineMarker(0, QwtSymbol::NoSymbol);
	}
	xy_plot.replot();
}

void Oscilloscope::setup_xy_channels()
{
	int x = ui->cmb_x_channel->currentIndex();
	int y = ui->cmb_y_channel->currentIndex();
	QWidget *xsw = xy_plot.axisWidget(QwtPlot::xBottom);
	xsw->setStyleSheet(
		QString("color: %1").arg(plot.getLineColor(x).name()));
	QWidget *ysw = xy_plot.axisWidget(QwtPlot::yLeft);
	ysw->setStyleSheet(
		QString("color: %1").arg(plot.getLineColor(y).name()));

	// If XY visible, reconnect the data flow
	if(xy_is_visible)
		onXY_view_toggled(true);

	xy_plot.setHorizUnitsPerDiv(plot.VertUnitsPerDiv(x));
	xy_plot.setVertUnitsPerDiv(plot.VertUnitsPerDiv(y), QwtPlot::yLeft);
}

void Oscilloscope::on_btnAddMath_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnMeasure_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnTrigger_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnGeneralSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if(checked)
		ui->btnSettings->setChecked(!checked);
}

/*
 * class Oscilloscope_API
 */

bool Oscilloscope_API::hasCursors() const
{
	return osc->ui->boxCursors->isChecked();
}

void Oscilloscope_API::setCursors(bool en)
{
	osc->ui->boxCursors->setChecked(en);
}

bool Oscilloscope_API::hasMeasure() const
{
	return osc->ui->boxMeasure->isChecked();
}

void Oscilloscope_API::setMeasure(bool en)
{
	osc->ui->boxMeasure->setChecked(en);
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

bool Oscilloscope_API::horizontalCursors() const
{
	osc->cr_ui->hCursorsEnable->isChecked();
}

void Oscilloscope_API::setHorizontalCursors(bool en)
{
	osc->cr_ui->hCursorsEnable->setChecked(en);
}

bool Oscilloscope_API::verticalCursors() const
{
	osc->cr_ui->vCursorsEnable->isChecked();
}

void Oscilloscope_API::setVerticalCursors(bool en)
{
	osc->cr_ui->vCursorsEnable->setChecked(en);
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

bool Oscilloscope_API::internalTrigger() const
{
	return osc->trigger_settings.ui->intern_en->isChecked();
}

void Oscilloscope_API::setInternalTrigger(bool en)
{
	osc->trigger_settings.ui->intern_en->setChecked(en);
}

bool Oscilloscope_API::externalTrigger() const
{
	return osc->trigger_settings.ui->extern_en->isChecked();
}

void Oscilloscope_API::setExternalTrigger(bool en)
{
	osc->trigger_settings.ui->extern_en->setChecked(en);
}

int Oscilloscope_API::triggerSource() const
{
	return osc->trigger_settings.ui->cmb_source->currentIndex();
}

void Oscilloscope_API::setTriggerSource(int idx)
{
	osc->trigger_settings.ui->cmb_source->setCurrentIndex(idx);
}

double Oscilloscope_API::getTriggerLevel() const
{
	return osc->trigger_settings.ui->trigger_level->value();
}

void Oscilloscope_API::setTriggerLevel(double level)
{
	osc->trigger_settings.ui->trigger_level->setValue(level);
}

double Oscilloscope_API::getTriggerHysteresis() const
{
	return osc->trigger_settings.ui->trigger_hysteresis->value();
}

void Oscilloscope_API::setTriggerHysteresis(double hyst)
{
	osc->trigger_settings.ui->trigger_hysteresis->setValue(hyst);
}

int Oscilloscope_API::internalCondition() const
{
	return osc->trigger_settings.ui->cmb_condition->currentIndex();
}

void Oscilloscope_API::setInternalCondition(int cond)
{
	osc->trigger_settings.ui->cmb_condition->setCurrentIndex(cond);
}

int Oscilloscope_API::externalCondition() const
{
	return osc->trigger_settings.ui->cmb_extern_condition->currentIndex();
}

void Oscilloscope_API::setExternalCondition(int cond)
{
	osc->trigger_settings.ui->cmb_extern_condition->setCurrentIndex(cond);
}

int Oscilloscope_API::internExtern() const
{
	return osc->trigger_settings.ui->cmb_analog_extern->currentIndex();
}

void Oscilloscope_API::setInternExtern(int option)
{
	osc->trigger_settings.ui->cmb_analog_extern->setCurrentIndex(option);
}

QList<QString> Oscilloscope_API::getMathChannels() const
{
	QList<QString> list;

	for (unsigned int i = 0; i < osc->nb_math_channels; i++) {
		QWidget *obj = osc->ui->channelsList->itemAt(
				osc->nb_channels + i)->widget();
		list.append(obj->property("function").toString());
	}

	return list;
}

void Oscilloscope_API::setMathChannels(const QList<QString>& list)
{
	osc->clearMathChannels();
	for (unsigned int i = 0; i < list.size(); i++)
		osc->add_math_channel(list.at(i).toStdString());
}

double Oscilloscope_API::getTimePos() const
{
	return osc->timePosition->value();
}

void Oscilloscope_API::setTimePos(double value)
{
	osc->timePosition->setValue(value);
}

double Oscilloscope_API::getTimeBase() const
{
	return osc->timeBase->value();
}

void Oscilloscope_API::setTimeBase(double value)
{
	osc->timeBase->setValue(value);
}

bool Oscilloscope_API::running() const
{
	return osc->ui->pushButtonRunStop->isChecked();
}

void Oscilloscope_API::run(bool en)
{
	osc->ui->pushButtonRunStop->setChecked(en);
}

QList<int> Oscilloscope_API::measureEn() const
{
	QList<int> list;

	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = 0;

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			if (measurements[j]->enabled())
				mask |= 1 << j;
		}

		list.append(mask);
	}

	return list;
}

void Oscilloscope_API::setMeasureEn(const QList<int>& list)
{
	if (list.size() != osc->nb_channels)
		return;

	osc->measure_settings->m_selectedMeasurements.clear();

	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = list.at(i);

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			measurements[j]->setEnabled(!!(mask & (1 << j)));
			osc->measure_settings->onMeasurementActivated(
					i, j, !!(mask & (1 << j)));
		}
	}

	osc->measure_settings->loadMeasurementStatesFromData();
	osc->onMeasurementSelectionListChanged();
}

QList<int> Oscilloscope_API::statisticEn() const
{
	QList<int> list;
	auto statistics = osc->measure_settings->statisticSelection();

	for (int i = 0; i < osc->nb_channels; i++) {
		int mask = 0;
		list.append(mask);
	}
	for (unsigned int i = 0; i < statistics.size(); i++)
		list[statistics[i].channel_id()] |= 1 << statistics[i].id();

	return list;
}

void Oscilloscope_API::setStatisticEn(const QList<int>& list)
{
	if (list.size() != osc->nb_channels)
		return;

	osc->measure_settings->deleteAllStatistics();
	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = list.at(i);

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			if (!!(mask & (1 << j)))
				osc->measure_settings->addStatistic(j, i);
		}
	}

	osc->measure_settings->loadStatisticStatesForChannel(
		osc->current_channel);
	osc->onStatisticSelectionListChanged();
}

void Oscilloscope::channelLineWidthChanged(int id)
{
	qreal width = 0.5 * (id + 1);

	if (width != plot.getLineWidthF(current_ch_widget)) {
		plot.setLineWidthF(current_ch_widget, width);
		plot.replot();
	}
}

int Oscilloscope_API::getCurrentChannel() const
{
	return osc->current_channel;
}

void Oscilloscope_API::setCurrentChannel(int chn_id)
{
	ChannelWidget *chn_widget = osc->channelWidgetAtId(chn_id);
	if (!chn_widget)
		return;

	osc->setChannelWidgetIndex(chn_id);

	chn_widget->nameButton()->setChecked(true);
}

bool Oscilloscope_API::getFftEn() const
{
	return osc->fft_is_visible;
}

void Oscilloscope_API::setFftEn(bool en)
{
	osc->gsettings_ui->FFT_view->setChecked(en);
}

bool Oscilloscope_API::getXyEn() const
{
	return osc->xy_is_visible;
}

void Oscilloscope_API::setXyEn(bool en)
{
	osc->gsettings_ui->XY_view->setChecked(en);
}

bool Oscilloscope_API::getExportAll() const
{
	return osc->exportSettings->getExportAllButton()->isChecked();
}

void Oscilloscope_API::setExportAll(bool en)
{
	osc->exportSettings->getExportAllButton()->setChecked(en);
}

int Oscilloscope_API::getCursorsPosition() const
{
	if (!hasCursors()) {
		return 0;
	}
	auto currentPos = osc->plot.d_cursorReadouts->getCurrentPosition();
	switch (currentPos) {
	case CustomPlotPositionButton::ReadoutsPosition::topLeft:
	default:
		return 0;
	case CustomPlotPositionButton::ReadoutsPosition::topRight:
		return 1;
	case CustomPlotPositionButton::ReadoutsPosition::bottomLeft:
		return 2;
	case CustomPlotPositionButton::ReadoutsPosition::bottomRight:
		return 3;
	}
}

void Oscilloscope_API::setCursorsPosition(int val)
{
	if (!hasCursors()) {
		return;
	}
	enum CustomPlotPositionButton::ReadoutsPosition types[] = {
		CustomPlotPositionButton::ReadoutsPosition::topLeft,
		CustomPlotPositionButton::ReadoutsPosition::topRight,
		CustomPlotPositionButton::ReadoutsPosition::bottomLeft,
		CustomPlotPositionButton::ReadoutsPosition::bottomRight
	};
	osc->cursorsPositionButton->setPosition(types[val]);
	osc->plot.replot();
}

int Oscilloscope_API::getCursorsTransparency() const
{
	if (!hasCursors()) {
		return 0;
	}
	return osc->cr_ui->horizontalSlider->value();
}

void Oscilloscope_API::setCursorsTransparency(int val)
{
	if (!hasCursors()) {
		return;
	}
	osc->cr_ui->horizontalSlider->setValue(val);
}

QVariantList Oscilloscope_API::getChannels()
{
	QVariantList list;

	for (Channel_API *each : osc->channels_api)
		list.append(QVariant::fromValue(each));

	return list;
}

/*
 * Channel_API
 */

bool Channel_API::channelEn() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	ChannelWidget *w = osc->channelWidgetAtId(index);

	return w->enableButton()->isChecked();
}

void Channel_API::setChannelEn(bool en)
{
	int index = osc->channels_api.indexOf(this);
	ChannelWidget *w = osc->channelWidgetAtId(index);

	w->enableButton()->setChecked(en);
}

double Channel_API::getVoltsPerDiv() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	return osc->plot.VertUnitsPerDiv(index);
}

void Channel_API::setVoltsPerDiv(double val)
{
	int index = osc->channels_api.indexOf(this);
	osc->plot.setVertUnitsPerDiv(val, index);

	QLabel *label = static_cast<QLabel *>(
			osc->ui->chn_scales->itemAt(index)->widget());
	label->setText(osc->vertMeasureFormat.format(val, "V/div", 3));

	// TO DO: refactor this once the source of the X and Y axes can be configured
	if (index == osc->index_x) {
		osc->xy_plot.setHorizUnitsPerDiv(val);
	}
	if (index == osc->index_y) {
		osc->xy_plot.setVertUnitsPerDiv(val, QwtPlot::yLeft);
	}
	osc->xy_plot.replot();
	osc->xy_plot.zoomBaseUpdate();
}

double Channel_API::getVOffset() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	return osc->plot.VertOffset(index);
}

void Channel_API::setVOffset(double val)
{
	int index = osc->channels_api.indexOf(this);
	osc->plot.setVertOffset(val, index);
}

double Channel_API::getLineThickness() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));

	return osc->plot.getLineWidthF(index);
}

void Channel_API::setLineThickness(double val)
{
	int index = osc->channels_api.indexOf(this);
	if (index == osc->current_ch_widget) {
		int cmbIdx = (int)(val / 0.5) - 1;
		osc->ch_ui->cmbChnLineWidth->setCurrentIndex(cmbIdx);
	}
	osc->plot.setLineWidthF(index, val);
}

double Channel_API::getProbeAttenuation() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));

	return osc->probe_attenuation[index];
}

void Channel_API::setProbeAttenuation(double val)
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));

	osc->probe_attenuation[index] = val;

}

bool Channel_API::getAcCoupling() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	return osc->chnAcCoupled[index];
}

void Channel_API::setAcCoupling(bool val)
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	if (osc->current_channel == index) {
		osc->ch_ui->btnCoupled->setChecked(val);
	} else {
		osc->configureAcCoupling(index, val);
	}
}

#define DECLARE_MEASURE(m, t) \
	double Channel_API::measured_ ## m () const\
	{\
		int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));\
		auto measData = osc->plot.measurement(Measure::t, index);\
		return measData->value();\
	}
DECLARE_MEASURE(period, PERIOD)
DECLARE_MEASURE(frequency, FREQUENCY)
DECLARE_MEASURE(min, MIN)
DECLARE_MEASURE(max, MAX)
DECLARE_MEASURE(peak_to_peak, PEAK_PEAK)
DECLARE_MEASURE(mean, MEAN)
DECLARE_MEASURE(cycle_mean, CYCLE_MEAN)
DECLARE_MEASURE(rms, RMS)
DECLARE_MEASURE(cycle_rms, CYCLE_RMS)
DECLARE_MEASURE(ac_rms, AC_RMS)
DECLARE_MEASURE(area, AREA)
DECLARE_MEASURE(cycle_area, CYCLE_AREA)
DECLARE_MEASURE(low, LOW)
DECLARE_MEASURE(high, HIGH)
DECLARE_MEASURE(amplitude, AMPLITUDE)
DECLARE_MEASURE(middle, MIDDLE)
DECLARE_MEASURE(pos_overshoot, P_OVER)
DECLARE_MEASURE(neg_overshoot, N_OVER)
DECLARE_MEASURE(rise, RISE)
DECLARE_MEASURE(fall, FALL)
DECLARE_MEASURE(pos_width, P_WIDTH)
DECLARE_MEASURE(neg_width, N_WIDTH)
DECLARE_MEASURE(pos_duty, P_DUTY)
DECLARE_MEASURE(neg_duty, N_DUTY)
