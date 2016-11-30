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

#include "signal_generator.hpp"
#include "ui_signal_generator.h"

#include <QBrush>
#include <QFileDialog>
#include <QFileInfo>
#include <QPalette>
#include <QSharedPointer>

#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/file_source.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/int_to_float.h>
#include <gnuradio/blocks/multiply_const_ff.h>
#include <gnuradio/blocks/nop.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/iio/device_sink.h>
#include <gnuradio/iio/math.h>

#include <iio.h>

#define NB_POINTS	32768
#define SAMPLE_RATE	750000
#define DAC_BIT_COUNT   12
#define INTERP_BY_100_CORR 1.168 // correction value at an interpolation by 100

#define AMPLITUDE_VOLTS	5.0

using namespace adiscope;
using namespace gr;

enum {
	DATA_IIO_PTR,
	DATA_SIGNAL_STRUCT,
	DATA_TIME_BLOCK,
};

enum SIGNAL_TYPE {
	SIGNAL_TYPE_CONSTANT	= 0,
	SIGNAL_TYPE_WAVEFORM	= 1,
	SIGNAL_TYPE_BUFFER	= 2,
	SIGNAL_TYPE_MATH	= 3,
};

struct adiscope::signal_generator_data {
	enum SIGNAL_TYPE type;
	unsigned int id;

	float constant;
	double amplitude;
	float offset;
	double frequency;
	double math_freq;
	double phase;
	enum sg_waveform waveform;
	QString file;
	QString function;
};
Q_DECLARE_METATYPE(QSharedPointer<signal_generator_data>);

struct adiscope::time_block_data {
	scope_sink_f::sptr time_block;
	unsigned long nb_channels;
};

SignalGenerator::SignalGenerator(struct iio_context *_ctx,
		Filter *filt, QPushButton *runButton, QWidget *parent) :
	QWidget(parent), ui(new Ui::SignalGenerator),
	ctx(_ctx), dev(filt->find_device(ctx, TOOL_SIGNAL_GENERATOR)),
	time_block_data(new adiscope::time_block_data),
	menuOpened(true), currentChannel(0), sample_rate(SAMPLE_RATE),
	settings_group(new QButtonGroup(this)), menuRunButton(runButton)
{
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	this->plot = new OscilloscopePlot(parent);

	settings_group->setExclusive(true);

	/* Setup right menu */
	constantValue = new PositionSpinButton({
				{"mVolts", 1E-3},
				{"Volts", 1E0}
				}, "Value", -5e0, 5e0);

	auto layout = static_cast<QBoxLayout *>(ui->tabConstant->layout());
	layout->insertWidget(0, constantValue, 0, Qt::AlignLeft);

	connect(constantValue, SIGNAL(valueChanged(double)),
			SLOT(constantValueChanged(double)));

	amplitude = new PositionSpinButton({
				{"µVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0},
			}, "Amplitude", 1e-6, 5e0);
	offset = new PositionSpinButton({
				{"µVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0},
			}, "Offset", -5e0, 5e0);
	phase = new PositionSpinButton({
				{"°", 1E0},
			}, "Phase", 0.0, 360.0);
	frequency = new ScaleSpinButton({
				{"mHz", 1E-3},
				{"Hz", 1E0},
#if SAMPLE_RATE > 1000
				{"kHz", 1E3},
#endif
#if SAMPLE_RATE > 1000000
				{"MHz", 1E6},
#endif
			}, "Frequency", 0.001, (sample_rate / 2) - 1);

	mathFrequency = new ScaleSpinButton({
				{"mHz", 1E-3},
				{"Hz", 1E0},
#if SAMPLE_RATE > 1000
				{"kHz", 1E3},
#endif
#if SAMPLE_RATE > 1000000
				{"MHz", 1E6},
#endif
			}, "Frequency", 0.001, (sample_rate / 2) - 1);

	/* Max amplitude by default */
	amplitude->setValue(amplitude->maxValue());

	/* (lowest freq * 100 * 1000) frequency by default */
	frequency->setValue(frequency->minValue() * 100 * 1000.0);
	mathFrequency->setValue(mathFrequency->minValue() * 100 * 1000.0);

	ui->waveformGrid->addWidget(amplitude, 0, 0, Qt::AlignLeft);
	ui->waveformGrid->addWidget(offset, 0, 1, Qt::AlignLeft);
	ui->waveformGrid->addWidget(frequency, 1, 0, Qt::AlignLeft);
	ui->waveformGrid->addWidget(phase, 1, 1, Qt::AlignLeft);

	layout = static_cast<QBoxLayout *>(ui->tabMath->layout());
	layout->insertWidget(0, mathFrequency, 0);

	connect(amplitude, SIGNAL(valueChanged(double)),
			this, SLOT(amplitudeChanged(double)));
	connect(offset, SIGNAL(valueChanged(double)),
			this, SLOT(offsetChanged(double)));
	connect(frequency, SIGNAL(valueChanged(double)),
			this, SLOT(frequencyChanged(double)));
	connect(phase, SIGNAL(valueChanged(double)),
			this, SLOT(phaseChanged(double)));


	unsigned int channels_count = iio_device_get_channels_count(dev);
	unsigned int nb_channels = 0;

	for (unsigned int i = 0; i < channels_count; i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_output(chn) ||
				!iio_channel_is_scan_element(chn))
			continue;

		auto ptr = QSharedPointer<signal_generator_data>(
				new signal_generator_data);
		ptr->amplitude = amplitude->value();
		ptr->offset = offset->value();
		ptr->frequency = frequency->value();
		ptr->constant = constantValue->value();
		ptr->phase = phase->value();
		ptr->waveform = SG_SIN_WAVE;
		ptr->math_freq = mathFrequency->value();

		ptr->type = SIGNAL_TYPE_CONSTANT;
		ptr->id = nb_channels;

		auto pair = new QPair<QWidget, Ui::Channel>;
		pair->second.setupUi(&pair->first);


		const char *ch_name = iio_channel_get_name(chn);
		std::string s = "Channel ";
		if (!ch_name) {
			s += std::to_string(nb_channels + 1);
			ch_name = s.c_str();
		}
		pair->second.box->setText(ch_name);
		pair->second.name->setVisible(false);

		pair->second.box->setProperty("id", QVariant(nb_channels));
		pair->second.btn->setProperty("id", QVariant(nb_channels));

		pair->first.setProperty("signal_generator_data",
				QVariant::fromValue(ptr));
		pair->first.setProperty("channel_name",
				QVariant(QString(ch_name)));

		const char *ch_id = iio_channel_get_id(chn);
		pair->first.setProperty("channel_id",
				QVariant(QString(ch_id)));

		if (i == 0)
			pair->second.btn->setChecked(true);

		ui->channelsList->addWidget(&pair->first);

		connect(pair->second.box, SIGNAL(toggled(bool)), this,
				SLOT(channel_box_toggled(bool)));

		connect(pair->second.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

		settings_group->addButton(pair->second.btn);

		channels.append(pair);
		nb_channels++;
	}

	time_block_data->nb_channels = nb_channels;
	time_block_data->time_block = scope_sink_f::make(
			NB_POINTS, sample_rate,
			"Signal Generator", nb_channels,
			static_cast<QWidget *>(plot));

	/* Attach all curves by default */
	plot->registerSink(time_block_data->time_block->name(),
			nb_channels, NB_POINTS);

	/* This must be done after attaching the curves; otherwise
	 * plot->getLineColor(i) returns black. */
	for (unsigned int i = 0; i < nb_channels; i++) {
		QString stylesheet(channels[i]->second.box->styleSheet());
		stylesheet += QString("\nQCheckBox{spacing: 12px;\n}\nQCheckBox::indicator {\nborder-color: %1;\n}\nQCheckBox::indicator:checked {\nbackground-color: %1;\n}\n"
				).arg(plot->getLineColor(i).name());
		channels[i]->second.box->setStyleSheet(stylesheet);
	}

	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	plot->disableLegend();
	plot->setPaletteColor(QColor("black"));

	plot->setVertUnitsPerDiv(AMPLITUDE_VOLTS * 2.0 / 10.0);

	plot->setHorizUnitsPerDiv((double) NB_POINTS /
			((double) sample_rate * 10.0));
	plot->setHorizOffset((double) NB_POINTS /
			((double) sample_rate * 2.0));

	ui->plot->addWidget(plot, 0, 0);

	connect(mathFrequency, SIGNAL(valueChanged(double)),
			this, SLOT(mathFreqChanged(double)));

	connect(ui->type, SIGNAL(currentIndexChanged(int)),
			this, SLOT(waveformTypeChanged(int)));

	connect(ui->tabWidget, SIGNAL(currentChanged(int)),
			this, SLOT(tabChanged(int)));

	connect(ui->load_file, SIGNAL(pressed()), this, SLOT(loadFile()));
	connect(ui->run_button, SIGNAL(toggled(bool)),
			this, SLOT(startStop(bool)));
	connect(runButton, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
	connect(ui->mathWidget, SIGNAL(functionValid(const QString&)),
			this, SLOT(setFunction(const QString&)));

	connect(runButton, SIGNAL(toggled(bool)), ui->run_button,
			SLOT(setChecked(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)), runButton,
			SLOT(setChecked(bool)));
}

SignalGenerator::~SignalGenerator()
{
	stop();

	delete plot;
	delete ui;
	delete time_block_data;
}

void SignalGenerator::constantValueChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->constant = (float) value;

	updatePreview();
}

void SignalGenerator::amplitudeChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->amplitude = value;

	updatePreview();
}

void SignalGenerator::offsetChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->offset = (float) value;

	updatePreview();
}

void SignalGenerator::frequencyChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->frequency = value;

	updatePreview();
}

void SignalGenerator::mathFreqChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->math_freq = value;

	updatePreview();
}

void SignalGenerator::phaseChanged(double value)
{
	auto ptr = getCurrentData();
	ptr->phase = value;

	updatePreview();
}

void SignalGenerator::waveformTypeChanged(int val)
{
	enum sg_waveform types[] = {
		SG_SIN_WAVE,
		SG_SQR_WAVE,
		SG_TRI_WAVE,
		SG_SAW_WAVE,
		SG_INV_SAW_WAVE,
	};

	auto ptr = getCurrentData();
	ptr->waveform = types[val];

	updatePreview();
}

QSharedPointer<signal_generator_data> SignalGenerator::getCurrentData()
{
	return getData(&channels[currentChannel]->first);
}

QSharedPointer<signal_generator_data> SignalGenerator::getData(QWidget *obj)
{
	auto var = obj->property("signal_generator_data");
	return var.value<QSharedPointer<signal_generator_data>>();
}

void SignalGenerator::tabChanged(int index)
{
	auto ptr = getCurrentData();
	ptr->type = (enum SIGNAL_TYPE) index;

	updatePreview();
}

void SignalGenerator::updatePreview()
{
	gr::top_block_sptr top = make_top_block("Signal Generator Update");
	unsigned int i = 0;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		basic_block_sptr source;

		if ((*it)->second.box->isChecked())
			source = getSource(&(*it)->first, sample_rate, top);
		else
			source = blocks::nop::make(sizeof(float));

		auto head = blocks::head::make(sizeof(float), NB_POINTS);
		top->connect(source, 0, head, 0);

		top->connect(head, 0, time_block_data->time_block, i++);
	}

	top->run();
	top->disconnect_all();
}

void SignalGenerator::loadFile()
{
	auto ptr = getCurrentData();

	ptr->file = QFileDialog::getOpenFileName(this, tr("Open File"));
	this->ui->label_path->setText(ptr->file);

	auto info = QFileInfo(ptr->file);
	this->ui->label_size->setText(QString("%1 ").arg(
				info.size() / sizeof(float)) + tr("samples"));

	updatePreview();
}

void SignalGenerator::start()
{
	top_block = gr::make_top_block("Signal Generator Output");

	std::vector<std::string> channel_names;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if ((*it)->second.box->isChecked()) {
			QVariant var = (*it)->first.property("channel_id");
			channel_names.push_back(var.toString().toStdString());
		}
	}

	const char *dev_name = iio_device_get_name(dev);
	if (!dev_name)
		dev_name = iio_device_get_id(dev);

	auto sink = iio::device_sink::make_from(ctx, dev_name, channel_names,
			dev_name, std::vector<std::string>(), NB_POINTS);

	unsigned int channel = 0;
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if ((*it)->second.box->isChecked()) {
			auto source = getSource(&(*it)->first, sample_rate, top_block);
			// DAC_RAW = (-Vout * 2^11) / 5V
			// Multiplying with 16 because the HDL considers the DAC data as 16 bit
			// instead of 12 bit(data is shifted to the left).
			auto f2s = blocks::float_to_short::make(1,
					-1 * (1 << (DAC_BIT_COUNT - 1)) /
					AMPLITUDE_VOLTS * 16 / INTERP_BY_100_CORR);
			top_block->connect(source, 0, f2s, 0);
			top_block->connect(f2s, 0, sink, channel++);
		}
	}

	top_block->start();

	ui->run_button->setText("Stop");
}

void SignalGenerator::stop()
{
	if (top_block) {
		top_block->stop();
		top_block->wait();
		top_block->disconnect_all();
		top_block.reset();
	}

	ui->run_button->setText("Run");
}

void SignalGenerator::startStop(bool pressed)
{
	ui->config_panel->setDisabled(pressed);

	if (pressed)
		start();
	else
		stop();
}

void SignalGenerator::setFunction(const QString& function)
{
	auto ptr = getCurrentData();
	ptr->function = function;

	updatePreview();
}

basic_block_sptr SignalGenerator::getSignalSource(gr::top_block_sptr top,
		unsigned long samp_rate, struct signal_generator_data &data)
{
	bool inv_saw_wave = data.waveform == SG_INV_SAW_WAVE;
	analog::gr_waveform_t waveform;
	double amplitude;
	float offset;

	if (data.waveform == SG_SIN_WAVE) {
		amplitude = data.amplitude;
		offset = data.offset;
	} else {
		amplitude = data.amplitude * 2.0;
		offset = data.offset - (float) data.amplitude;
	}

	if (inv_saw_wave)
		waveform = analog::GR_SAW_WAVE;
	else
		waveform = static_cast<analog::gr_waveform_t>(data.waveform);

	auto src = analog::sig_source_f::make(samp_rate, waveform,
			data.frequency, amplitude, offset);
	auto delay = blocks::delay::make(sizeof(float),
			samp_rate * data.phase / (data.frequency * 360.0));

	auto skip_head = blocks::skiphead::make(sizeof(float),
			samp_rate / data.frequency);

	top->connect(src, 0, delay, 0);
	top->connect(delay, 0, skip_head, 0);

	if (!inv_saw_wave) {
		return skip_head;
	} else {
		auto mult = blocks::multiply_const_ff::make(-1.0);
		top->connect(skip_head, 0, mult, 0);

		return mult;
	}
}

gr::basic_block_sptr SignalGenerator::getSource(QWidget *obj,
		unsigned long samp_rate, gr::top_block_sptr top)
{
	auto ptr = getData(obj);
	enum SIGNAL_TYPE type = ptr->type;

	switch (type) {
	case SIGNAL_TYPE_CONSTANT:
		return analog::sig_source_f::make(samp_rate,
				analog::GR_CONST_WAVE, 0, 0,
				ptr->constant);
	case SIGNAL_TYPE_WAVEFORM:
		return getSignalSource(top, samp_rate, *ptr);
	case SIGNAL_TYPE_BUFFER:
		if (!ptr->file.isNull()) {
			auto str = ptr->file.toStdString();

			return blocks::file_source::make(
					sizeof(float), str.c_str(), true);
		}
		break;
	case SIGNAL_TYPE_MATH:
		if (!ptr->function.isNull()) {
			auto str = ptr->function.toStdString();

			return iio::iio_math_gen::make(samp_rate,
					ptr->math_freq, str);
		}
	default:
		break;
	}

	return blocks::nop::make(sizeof(float));
}

void adiscope::SignalGenerator::channel_box_toggled(bool checked)
{
	QCheckBox *box = static_cast<QCheckBox *>(QObject::sender());
	unsigned int id = box->property("id").toUInt();

	if (checked) {
		plot->AttachCurve(id);
	} else {
		if (channels[id]->second.btn->isChecked()) {
			settings_group->setExclusive(false);
			channels[id]->second.btn->setChecked(false);
			toggleRightMenu(channels[id]->second.btn);
		}

		plot->DetachCurve(id);
	}

	updatePreview();
	plot->replot();

	bool enable_run = checked;
	if (!checked) {
		for (auto it = channels.begin();
				!enable_run && it != channels.end(); ++it)
			enable_run = (*it)->second.box->isChecked();
	}

	ui->run_button->setEnabled(enable_run);
	menuRunButton->setEnabled(enable_run);
}

void adiscope::SignalGenerator::renameConfigPanel()
{
	QString name = channels[currentChannel]->first.property(
			"channel_name").toString();

	ui->config_panel->setTitle(QString("Configuration for %1").arg(name));
}

void adiscope::SignalGenerator::rightMenuFinished(bool opened)
{
	if (opened) {
		menuOpened = true;
	} else if (menuOpened) {
		auto ptr = getCurrentData();

		constantValue->setValue(ptr->constant);
		frequency->setValue(ptr->frequency);
		offset->setValue(ptr->offset);
		amplitude->setValue(ptr->amplitude);
		phase->setValue(ptr->phase);
		ui->label_path->setText(ptr->file);
		ui->mathWidget->setFunction(ptr->function);
		mathFrequency->setValue(ptr->math_freq);

		if (!ptr->file.isNull()) {
			auto info = QFileInfo(ptr->file);
			ui->label_size->setText(QString("%1 ").arg(
						info.size() / sizeof(float))
					+ tr("samples"));
		} else {
			ui->label_size->setText("");
		}

		switch (ptr->waveform) {
		case analog::GR_SIN_WAVE:
		default:
			ui->type->setCurrentIndex(0);
			break;
		case analog::GR_SQR_WAVE:
			ui->type->setCurrentIndex(1);
			break;
		case analog::GR_TRI_WAVE:
			ui->type->setCurrentIndex(2);
			break;
		case analog::GR_SAW_WAVE:
			ui->type->setCurrentIndex(3);
			break;
		}

		renameConfigPanel();
		ui->tabWidget->setCurrentIndex((int) ptr->type);
		updatePreview();

		ui->rightMenu->toggleMenu(true);
	}
}

void adiscope::SignalGenerator::toggleRightMenu(QPushButton *btn)
{
	unsigned int id = btn->property("id").toUInt();
	bool open = !menuOpened;

	settings_group->setExclusive(!btn->isChecked());

	/* Keep the right menu closed */
	if (menuOpened && currentChannel == id)
		menuOpened = false;

	currentChannel = id;

	if (open)
		renameConfigPanel();

	plot->setActiveVertAxis(id);
	ui->rightMenu->toggleMenu(open);
}

void adiscope::SignalGenerator::toggleRightMenu()
{
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}
