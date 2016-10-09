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

/* Local includes */
#include "adc_sample_conv.hpp"
#include "math.hpp"
#include "oscilloscope.hpp"
#include "dynamicWidget.hpp"

/* Generated UI */
#include "ui_math_panel.h"
#include "ui_channel.h"
#include "ui_channel_math.h"
#include "ui_channel_settings.h"
#include "ui_cursors_settings.h"
#include "ui_osc_general_settings.h"
#include "ui_measure_settings.h"
#include "ui_measure_panel.h"
#include "ui_oscilloscope.h"
#include "ui_trigger.h"

using namespace adiscope;
using namespace gr;
using namespace std;

const unsigned long Oscilloscope::maxBufferSize = 32768;

Oscilloscope::Oscilloscope(struct iio_context *ctx,
		Filter *filt, QPushButton *runButton,
		float gain_ch1, float gain_ch2, QWidget *parent) :
	QWidget(parent),
	adc(ctx, filt),
	nb_channels(Oscilloscope::adc.numChannels()),
	sampling_rates(adc.availSamplRates()),
	active_sample_rate(adc.sampleRate()),
	nb_math_channels(0),
	ui(new Ui::Oscilloscope),
	trigger_settings(ctx),
	plot(parent, 16, 10),
	fft_plot(nb_channels, parent),
	xy_plot(nb_channels / 2, parent),
	hist_plot(nb_channels, parent),
	ids(new iio_manager::port_id[nb_channels]),
	fft_ids(new iio_manager::port_id[nb_channels]),
	hist_ids(new iio_manager::port_id[nb_channels]),
	xy_ids(new iio_manager::port_id[nb_channels & ~1]),
	fft_is_visible(false), hist_is_visible(false), xy_is_visible(false),
	triggerDelay(0),
	selectedChannel(-1),
	menuOpened(false), current_channel(0), math_chn_counter(0),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	menuRunButton(runButton)
{
	ui->setupUi(this);
	int triggers_panel = ui->stackedWidget->insertWidget(-1, &trigger_settings);
	settings_group->setExclusive(true);

	fft_size = 1024;

	/* Gnuradio Blocks */

	int num_samples = plot.axisInterval(QwtPlot::xBottom).width() *
				adc.sampleRate();

	this->qt_time_block = adiscope::scope_sink_f::make(
			num_samples,
			adc.sampleRate(), "Osc Time", nb_channels, (QObject *)&plot);
	trigger_settings.setPlotNumSamples(num_samples);

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

	plot.registerSink(qt_time_block->name(), nb_channels,
			plot.axisInterval(QwtPlot::xBottom).width() *
			adc.sampleRate());
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
	QWidget *chn0_widget = channelWidgetAtId(0);
	if (chn0_widget) {
		QPushButton *name = chn0_widget->findChild<QPushButton *>("name");
		name->setChecked(true);
	}


	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	/* Cursors Settings */
	QWidget *cursor_widget = new QWidget(this);
	Ui::Channel cursor_ui;

	cursor_ui.setupUi(cursor_widget);
	ui->cursors_settings->addWidget(cursor_widget);

	settings_group->addButton(cursor_ui.btn);
	cursor_ui.btn->setProperty("id", QVariant(-1));
	cursor_ui.name->setText("Cursors");
	cursor_ui.box->setChecked(false);
	QString stylesheet(cursor_ui.box->styleSheet());
	stylesheet += QString("\nQCheckBox::indicator {"
				"border-color: rgb(74, 100, 255);"
				"border-radius: 4px;"
				"}"
				"QCheckBox::indicator:checked {"
				"background-color: rgb(74, 100, 255);"
				"}");
	cursor_ui.box->setStyleSheet(stylesheet);
	connect(cursor_ui.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));
	connect(cursor_ui.box, SIGNAL(toggled(bool)), this,
			SLOT(onCursorsToggled(bool)));

	/* Measurements Settings */
	int measure_panel = ui->stackedWidget->indexOf(ui->measureSettings);

	Ui::MeasureSettings *msettings_ui = new Ui::MeasureSettings();
	msettings_ui->setupUi(ui->measureSettings);

	QWidget *measure_widget = new QWidget(this);
	Ui::Channel measure_ui;

	measure_ui.setupUi(measure_widget);
	ui->measure_settings->addWidget(measure_widget);
	settings_group->addButton(measure_ui.btn);
	measure_ui.btn->setProperty("id", QVariant(-measure_panel));
	measure_ui.name->setText("Measure");
	measure_ui.box->setChecked(false);
	measure_ui.box->setStyleSheet(stylesheet);

	connect(measure_ui.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));
	connect(measure_ui.box, SIGNAL(toggled(bool)), this,
			SLOT(onMeasureToggled(bool)));
	connect(measure_ui.box, SIGNAL(toggled(bool)),
		&plot, SLOT(setMeasuremensEnabled(bool)));

	/* Trigger Settings */
	QWidget *trig_widget = new QWidget(this);
	Ui::Trigger trig_ui;

	trig_ui.setupUi(trig_widget);
	ui->trigger_settings->addWidget(trig_widget);
	settings_group->addButton(trig_ui.btn);
	trig_ui.btn->setProperty("id", QVariant(-triggers_panel));
	connect(trig_ui.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

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
	measurePanel = new QWidget(this);
	measure_panel_ui = new Ui::MeasurementsPanel();
	measure_panel_ui->setupUi(measurePanel);
	measurePanel->hide();

	connect(&plot, SIGNAL(measurementsAvailable()),
		SLOT(onMeasuremetsAvailable()));

	/* Plot layout */

	/* Top transparent widget */

	ui->gridLayoutPlot->addWidget(measurePanel, 0, 1, 1, 1);
	ui->gridLayoutPlot->addWidget(plot.topArea(), 1, 0, 1, 3);
	ui->gridLayoutPlot->addWidget(plot.leftHandlesArea(), 1, 0, 3, 1);
	ui->gridLayoutPlot->addWidget(&plot, 2, 1, 1, 1);
	ui->gridLayoutPlot->addWidget(plot.rightHandlesArea(), 1, 2, 3, 1);
	ui->gridLayoutPlot->addWidget(plot.bottomHandlesArea(), 3, 0, 1, 3);

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

	/* General Settings Menu */
	gsettings_ui = new Ui::OscGeneralSettings();
	gsettings_ui->setupUi(ui->generalSettings);
	settings_group->addButton(ui->btnGeneralSettings);

	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

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
		SLOT(onHorizOffsetValueChanged(double)));
	connect(voltsPosition, SIGNAL(valueChanged(double)),
		SLOT(onVertOffsetValueChanged(double)));

	connect(&plot, SIGNAL(channelOffsetChanged(double)),
		SLOT(onChannelOffsetChanged(double)));

	connect(this, SIGNAL(selectedChannelChanged(int)),
		&plot, SLOT(setSelectedChannel(int)));

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

	// Trigger Delay
	connect(&trigger_settings, SIGNAL(delayChanged(double)),
			SLOT(onTriggerSettingsDelayChanged(double)));
	connect(&plot, SIGNAL(timeTriggerValueChanged(double)),
			SLOT(onTimeTriggerDelayChanged(double)));

	connect(this, SIGNAL(triggerDelayChanged(double)), this,
			SLOT(updateTriggerSpinbox(double)));
	connect(this, SIGNAL(triggerDelayChanged(double)), this,
			SLOT(updatePlotHorizDelay(double)));

	Ui::CursorsSettings cr_ui;
	cr_ui.setupUi(ui->cursorsSettings);
	connect(cr_ui.vCursorsEnanble, SIGNAL(toggled(bool)),
		&plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui.hCursorsEnanble, SIGNAL(toggled(bool)),
		&plot, SLOT(setHorizCursorsEnabled(bool)));

	if (nb_channels < 2)
		gsettings_ui->XY_view->hide();
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

	delete[] xy_ids;
	delete[] hist_ids;
	delete[] fft_ids;
	delete[] ids;
	delete ch_ui;
	delete gsettings_ui;
	delete measure_panel_ui;
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
}

void Oscilloscope::del_math_channel()
{
	QPushButton *delBtn = static_cast<QPushButton *>(QObject::sender());
	QWidget *parent = delBtn->parentWidget();
	QPushButton *btn = parent->findChild<QPushButton *>("btn");
	unsigned int curve_id = btn->property("id").toUInt();
	QString qname = delBtn->property("curve_name").toString();

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
		btn->setChecked(false);
		active_settings_btn = nullptr;
		ui->rightMenu->toggleMenu(false);
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
	QPushButton *btn = ui->pushButtonRunStop;

	if (checked) {
		btn->setText("Stop");

		plot.setSampleRate(active_sample_rate, 1, "");

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
		btn->setText("Run");

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
			btn->setChecked(false);
			toggleRightMenu(btn);
		}
	}

	plot.setMeasurementCursorsEnabled(on);
}

void adiscope::Oscilloscope::onMeasureToggled(bool on)
{
	QCheckBox *box = static_cast<QCheckBox *>(QObject::sender());
	QPushButton *btn = box->parentWidget()->findChild<QPushButton *>("btn");

	if (!on) {
		if (btn->isChecked()) {
			settings_group->setExclusive(false);
			btn->setChecked(false);
			toggleRightMenu(btn);
		}
	}
	measurePanel->setVisible(on);
}

void Oscilloscope::updateTriggerSpinbox(double value)
{
	trigger_settings.setDelay(value);
}

void Oscilloscope::updatePlotHorizDelay(double value)
{
	plot.setHorizDelay(value);
	plot.updateAxes();
}

void Oscilloscope::onTriggerSettingsDelayChanged(double value)
{
	if (triggerDelay != value) {
		triggerDelay = value;
		emit triggerDelayChanged(value);
	}
}

void Oscilloscope::onTimeTriggerDelayChanged(double value)
{
	double delay = value;

	if (triggerDelay != delay) {
		triggerDelay = delay;
		emit triggerDelayChanged(delay);
	}
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
			btn->setChecked(false);
			toggleRightMenu(btn);
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
		emit selectedChannelChanged(id);
	}
}

void adiscope::Oscilloscope::onVertScaleValueChanged(double value)
{
	if (value != plot.VertUnitsPerDiv(plot.activeVertAxis())) {
		plot.setVertUnitsPerDiv(value, plot.activeVertAxis());
		plot.replot();
	}
	voltsPosition->setStep(value / 10);
}

void adiscope::Oscilloscope::onHorizScaleValueChanged(double value)
{
	if (value != plot.HorizUnitsPerDiv()) {
		plot.setHorizUnitsPerDiv(value);
		plot.replot();
	}

	timePosition->setStep(value / 10);

	double old_sample_rate = active_sample_rate;
	double plotTimeSpan = value * plot.xAxisNumDiv();
	double newSampleRate = pickSampleRateFor(plotTimeSpan, maxBufferSize);
	double newSampleCount = plotTimeSpan * newSampleRate;
	active_sample_rate = newSampleRate;

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	bool started = iio->started();
	if (started)
		iio->lock();
	this->qt_time_block->set_nsamps(newSampleCount);

	// Apply amplitude corrections when using different sample rates
	if (newSampleRate != old_sample_rate) {
		boost::shared_ptr<adc_sample_conv> block =
			dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);
		block->setFilterCompensation(0, adc.compTable(newSampleRate));
		block->setFilterCompensation(1, adc.compTable(newSampleRate));
	}

	adc.setSampleRate(newSampleRate);
	if (started)
		plot.setSampleRate(newSampleRate, 1, "");
	trigger_settings.setPlotNumSamples(newSampleCount);

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->set_buffer_size(ids[i], newSampleCount);

	if (started)
		iio->unlock();
}

void adiscope::Oscilloscope::onVertOffsetValueChanged(double value)
{
	if (value != plot.VertOffset(plot.activeVertAxis())) {
		plot.setVertOffset(value, plot.activeVertAxis());
		plot.replot();
	}
}

void adiscope::Oscilloscope::onHorizOffsetValueChanged(double value)
{
	if (value != plot.HorizOffset()) {
		plot.setHorizOffset(value);
		plot.replot();
	}
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
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
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

double Oscilloscope::pickSampleRateFor(double timeSpanSecs, double desiredBufferSize)
{
	double idealSampleRate = desiredBufferSize / timeSpanSecs;
	int srIdx = 0;

	// Pick the highest sample rate that we can set, that is lower or equal to
	// the idealSampleRate.
	for (int i = 0; i < sampling_rates.size(); i++) {
		if (idealSampleRate >= sampling_rates[i])
			srIdx = i;
	}

	idealSampleRate = sampling_rates[srIdx];

	return idealSampleRate;
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
	TimePrefixFormatter *hf = &horizMeasureFormat;
	MetricPrefixFormatter *vf = &vertMeasureFormat;

	measure_panel_ui->label_period_val->setText(
		hf->format(plot.measuredPeriod(), "", 3));

	measure_panel_ui->label_freq_val->setText(
		vf->format(plot.measuredFreq(), "Hz", 3));

	measure_panel_ui->label_min_val->setText(
		vf->format(plot.measuredMin(), "V", 3));

	measure_panel_ui->label_max_val->setText(
		vf->format(plot.measuredMax(), "V", 3));

	measure_panel_ui->label_ppeak_val->setText(
		vf->format(plot.measuredPkToPk(), "V", 3));

	measure_panel_ui->label_mean_val->setText(
		vf->format(plot.measuredMean(), "V", 3));

	measure_panel_ui->label_rms_val->setText(
		vf->format(plot.measuredRms(), "V", 3));

	measure_panel_ui->label_ac_rms_val->setText(
		vf->format(plot.measuredRmsAC(), "V", 3));

	measure_panel_ui->label_ampl_val->setText(
		vf->format(plot.measuredAmplitude(), "V", 3));

	measure_panel_ui->label_low_val->setText(
		vf->format(plot.measuredLow(), "V", 3));

	measure_panel_ui->label_high_val->setText(
		vf->format(plot.measuredHigh(), "V", 3));

	measure_panel_ui->label_middle_val->setText(
		vf->format(plot.measuredMiddle(), "V", 3));

	measure_panel_ui->label_overshoot_p_val->setText(
		vf->format(plot.measuredPosOvershoot(), "", 2) + "%");

	measure_panel_ui->label_overshoot_n_val->setText(
		vf->format(plot.measuredNegOvershoot(), "", 2) + "%");

	measure_panel_ui->label_rise_val->setText(
		hf->format(plot.measuredRiseTime(), "", 3));

	measure_panel_ui->label_fall_val->setText(
		hf->format(plot.measuredFallTime(), "", 3));

	measure_panel_ui->label_width_p_val->setText(
		hf->format(plot.measuredPosWidth(), "", 3));

	measure_panel_ui->label_width_n_val->setText(
		hf->format(plot.measuredNegWidth(), "", 3));

	measure_panel_ui->label_duty_p_val->setText(
		vf->format(plot.measuredPosDuty(), "", 2) + "%");

	measure_panel_ui->label_duty_n_val->setText(
		vf->format(plot.measuredNegDuty(), "", 2) + "%");
}
