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

#include "dynamicWidget.hpp"
#include "signal_generator.hpp"
#include "spinbox_a.hpp"
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
#include <gnuradio/blocks/vector_sink_s.h>
#include <gnuradio/iio/device_sink.h>
#include <gnuradio/iio/math.h>

#include <iio.h>


#define NB_POINTS	32768
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

SignalGenerator::SignalGenerator(struct iio_context *_ctx, Filter *filt,
		QPushButton *runButton, QJSEngine *engine, ToolLauncher *parent) :
	Tool(_ctx, runButton, new SignalGenerator_API(this), parent),
	ui(new Ui::SignalGenerator),
	time_block_data(new adiscope::time_block_data),
	currentChannel(0), sample_rate(0),
	settings_group(new QButtonGroup(this))
{
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	this->plot = new OscilloscopePlot(this);

	QVector<struct iio_channel *> iio_channels;

	for (unsigned int dev_id = 0; ; dev_id++) {
		struct iio_device *dev;
		try {
			dev = filt->find_device(ctx,
					TOOL_SIGNAL_GENERATOR, dev_id);
		} catch (std::exception &ex) {
			break;
		}

		unsigned int nb = iio_device_get_channels_count(dev);
		unsigned long dev_sample_rate = get_max_sample_rate(dev);

		if (dev_sample_rate > sample_rate)
			sample_rate = dev_sample_rate;

		for (unsigned int i = 0; i < nb; i++) {
			struct iio_channel *ch = iio_device_get_channel(dev, i);

			if (iio_channel_is_output(ch) &&
					iio_channel_is_scan_element(ch))
				iio_channels.append(ch);
		}
	}

	/* Max amplitude by default */
	ui->amplitude->setValue(ui->amplitude->maxValue());

	/* Set max frequency according to max sample rate */
	ui->frequency->setMaxValue((sample_rate / 2) - 1);
	ui->mathFrequency->setMaxValue((sample_rate / 2) - 1);

	/* (lowest freq * 100 * 1000) frequency by default */
	ui->frequency->setValue(ui->frequency->minValue() * 100 * 1000.0);
	ui->mathFrequency->setValue(
			ui->mathFrequency->minValue() * 100 * 1000.0);

	unsigned int nb_channels = iio_channels.size();

	for (unsigned int i = 0; i < nb_channels; i++) {
		struct iio_channel *chn = iio_channels[i];

		auto ptr = QSharedPointer<signal_generator_data>(
				new signal_generator_data);
		ptr->amplitude = ui->amplitude->value();
		ptr->offset = ui->offset->value();
		ptr->frequency = ui->frequency->value();
		ptr->constant = ui->constantValue->value();
		ptr->phase = ui->phase->value();
		ptr->waveform = SG_SIN_WAVE;
		ptr->math_freq = ui->mathFrequency->value();

		ptr->type = SIGNAL_TYPE_CONSTANT;
		ptr->id = i;

		auto pair = new QPair<QWidget, Ui::Channel>;
		pair->second.setupUi(&pair->first);


		const char *ch_name = iio_channel_get_name(chn);
		std::string s = "Channel ";
		if (!ch_name) {
			s += std::to_string(i + 1);
			ch_name = s.c_str();
		}
		pair->second.box->setText(ch_name);
		pair->second.name->setVisible(false);

		pair->second.box->setProperty("id", QVariant(i));
		pair->second.btn->setProperty("id", QVariant(i));

		pair->first.setProperty("signal_generator_data",
				QVariant::fromValue(ptr));
		pair->first.setProperty("channel_name",
				QVariant(QString(ch_name)));
		pair->first.setProperty("channel",
				qVariantFromValue((void *) chn));
		iio_channel_set_data(chn, &pair->first);

		ui->channelsList->addWidget(&pair->first);

		connect(pair->second.box, SIGNAL(toggled(bool)), this,
				SLOT(channel_box_toggled(bool)));

		connect(pair->second.btn, SIGNAL(pressed()),
				this, SLOT(toggleRightMenu()));

		settings_group->addButton(pair->second.btn);
		if (i == 0)
			pair->second.btn->setChecked(true);

		channels.append(pair);

		channels_vlsb.append(QPair<struct iio_channel *, double>(chn, 0.0));
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

	plot->disableLegend();
	plot->setPaletteColor(QColor("black"));

	plot->setVertUnitsPerDiv(AMPLITUDE_VOLTS * 2.0 / 10.0);

	plot->setHorizUnitsPerDiv((double) NB_POINTS /
			((double) sample_rate * 10.0));
	plot->setHorizOffset((double) NB_POINTS /
			((double) sample_rate * 2.0));
	plot->zoomBaseUpdate();

	ui->plot->addWidget(plot, 0, 0);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_SIGNAL_GENERATOR)));
	api->load();
	api->js_register(engine);

	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	connect(ui->constantValue, SIGNAL(valueChanged(double)),
			SLOT(constantValueChanged(double)));

	connect(ui->amplitude, SIGNAL(valueChanged(double)),
			this, SLOT(amplitudeChanged(double)));
	connect(ui->offset, SIGNAL(valueChanged(double)),
			this, SLOT(offsetChanged(double)));
	connect(ui->frequency, SIGNAL(valueChanged(double)),
			this, SLOT(frequencyChanged(double)));
	connect(ui->phase, SIGNAL(valueChanged(double)),
			this, SLOT(phaseChanged(double)));

	connect(ui->mathFrequency, SIGNAL(valueChanged(double)),
			this, SLOT(mathFreqChanged(double)));

	connect(ui->type, SIGNAL(currentIndexChanged(int)),
			this, SLOT(waveformTypeChanged(int)));

	connect(ui->tabWidget, SIGNAL(currentChanged(int)),
			this, SLOT(tabChanged(int)));

	connect(ui->load_file, SIGNAL(pressed()), this, SLOT(loadFile()));
	connect(ui->mathWidget, SIGNAL(functionValid(const QString&)),
			this, SLOT(setFunction(const QString&)));

	connect(ui->run_button, SIGNAL(toggled(bool)), runButton,
			SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)), ui->run_button,
			SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)),
			this, SLOT(startStop(bool)));

	updatePreview();
}

SignalGenerator::~SignalGenerator()
{
	ui->run_button->setChecked(false);

	api->save();
	delete api;

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
	bool enabled = false;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		basic_block_sptr source;

		if ((*it)->second.box->isChecked()) {
			source = getSource(&(*it)->first, sample_rate, top);
			enabled = true;
		} else {
			source = blocks::nop::make(sizeof(float));
		}

		auto head = blocks::head::make(sizeof(float), NB_POINTS);
		top->connect(source, 0, head, 0);

		top->connect(head, 0, time_block_data->time_block, i++);
	}

	top->run();
	top->disconnect_all();

	if (ui->run_button->isChecked()) {
		if (enabled) {
			stop();
			start();
		} else {
			ui->run_button->setChecked(false);
		}
	}
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
	QVector<struct iio_channel *> enabled_channels;

	/* Avoid from being started twice */
	if (buffers.size() > 0)
		return;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if (!(*it)->second.box->isChecked())
			continue;

		void *ptr = (*it)->first.property("channel").value<void *>();
		enabled_channels.append(static_cast<struct iio_channel *>(ptr));
	}

	do {
		const struct iio_device *dev =
			iio_channel_get_device(enabled_channels[0]);

		/* First, disable all the channels of this device */
		unsigned int nb = iio_device_get_channels_count(dev);
		for (unsigned int i; i < nb; i++)
			iio_channel_disable(iio_device_get_channel(dev, i));

		/* Then enable the channels that we want */
		for (auto each : enabled_channels) {
			if (dev == iio_channel_get_device(each))
				iio_channel_enable(each);
		}

		/* Enable the (optional) DMA sync */
		iio_device_attr_write_bool(dev, "dma_sync", true);

		unsigned long best_rate = get_best_sample_rate(dev);
		size_t samples_count = get_samples_count(dev, best_rate);

		/* Create the IIO buffer */
		struct iio_buffer *buf = iio_device_create_buffer(
				dev, samples_count, true);
		if (!buf)
			throw std::runtime_error("Unable to create buffer");

		qDebug() << QString("Created buffer with %1 samples at %2 SPS for device %3")
			.arg(samples_count).arg(best_rate).arg(
					iio_device_get_name(dev) ?:
					iio_device_get_id(dev));

		for (auto each : enabled_channels) {
			if (dev != iio_channel_get_device(each))
				continue;

			enabled_channels.remove(enabled_channels.indexOf(each));

			top_block = gr::make_top_block("Signal Generator");

			void *ptr = iio_channel_get_data(each);
			QWidget *w = static_cast<QWidget *>(ptr);
			auto source = getSource(w, best_rate, top_block);

			float volts_to_raw_coef;
			double vlsb = 1;
			auto pair_it = std::find_if(channels_vlsb.begin(),
				channels_vlsb.end(),
				[&each](const QPair<struct iio_channel *,
				double>& element) {
					return element.first == each;
				}
				);
			if (pair_it != channels_vlsb.end()) {
				vlsb = (*pair_it).second;
			}

			if (vlsb == 0.0) {	// DAC_RAW = (-Vout * 2^11) / 5V
						// Multiplying with 16 because the HDL considers the DAC data as 16 bit
						// instead of 12 bit(data is shifted to the left).
				volts_to_raw_coef = -1 * (1 << (DAC_BIT_COUNT - 1)) /
					AMPLITUDE_VOLTS * 16 / INTERP_BY_100_CORR;
			} else {		// DAC_RAW = (-Vout / (voltage corresponding to a LSB));
						// Multiplying with 16 because the HDL considers the DAC data as 16 bit
						// instead of 12 bit(data is shifted to the left).
				volts_to_raw_coef = -1 * (1 / vlsb) * 16;
			}


			auto f2s = blocks::float_to_short::make(1,
					volts_to_raw_coef);

			auto head = blocks::head::make(
					sizeof(short), samples_count);

			auto vector = blocks::vector_sink_s::make();

			top_block->connect(source, 0, f2s, 0);
			top_block->connect(f2s, 0, head, 0);
			top_block->connect(head, 0, vector, 0);

			top_block->run();

			const std::vector<short> &samples = vector->data();
			const short *data = samples.data();

			iio_channel_write(each, buf, data,
					samples_count * sizeof(short));
		}

		set_sample_rate(dev, best_rate);

		qDebug() << "Pushed cyclic buffer";

		iio_buffer_push_partial(buf, samples_count);
		buffers.append(buf);

	} while (!enabled_channels.empty());

	/* Now that we pushed all the buffers, disable the (optional) DMA sync
	 * for the devices that support it. */
	for (auto buf : buffers) {
		const struct iio_device *dev = iio_buffer_get_device(buf);

		iio_device_attr_write_bool(dev, "dma_sync", false);
	}
}

void SignalGenerator::stop()
{
	for (auto each : buffers)
		iio_buffer_destroy(each);

	buffers.clear();
}

void SignalGenerator::startStop(bool pressed)
{
	if (pressed)
		start();
	else
		stop();

	setDynamicProperty(ui->run_button, "running", pressed);
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
	double phase;
	double amplitude;
	float offset;

	if (data.waveform == SG_SIN_WAVE) {
		amplitude = data.amplitude / 2.0;
		offset = data.offset;
	} else {
		amplitude = data.amplitude;
		offset = data.offset - (float) data.amplitude / 2.0;
	}

	if (data.waveform == SG_TRI_WAVE)
		phase = data.phase + 90.0;
	else if (data.waveform == SG_SQR_WAVE)
		phase = data.phase + 180.0;
	else
		phase = data.phase;

	if (inv_saw_wave) {
		waveform = analog::GR_SAW_WAVE;
		offset = -data.offset - (float) data.amplitude / 2.0;
	} else {
		waveform = static_cast<analog::gr_waveform_t>(data.waveform);
	}

	auto src = analog::sig_source_f::make(samp_rate, waveform,
			data.frequency, amplitude, offset);
	auto delay = blocks::delay::make(sizeof(float),
			samp_rate * phase / (data.frequency * 360.0));

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
		if (!ptr->function.isEmpty()) {
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
	run_button->setEnabled(enable_run);
}

void adiscope::SignalGenerator::renameConfigPanel()
{
	QString name = channels[currentChannel]->first.property(
			"channel_name").toString();

	ui->config_panel->setTitle(QString("Configuration for %1").arg(name));
}

int SignalGenerator::sg_waveform_to_idx(enum sg_waveform wave)
{
	switch (wave) {
	case SG_SIN_WAVE:
	default:
		return 0;
	case SG_SQR_WAVE:
		return 1;
	case SG_TRI_WAVE:
		return 2;
	case SG_SAW_WAVE:
		return 3;
	case SG_INV_SAW_WAVE:
		return 4;
	}
}

void adiscope::SignalGenerator::rightMenuFinished(bool opened)
{
	if (!opened && !!settings_group->checkedButton()) {
		auto ptr = getCurrentData();

		ui->constantValue->setValue(ptr->constant);
		ui->frequency->setValue(ptr->frequency);
		ui->offset->setValue(ptr->offset);
		ui->amplitude->setValue(ptr->amplitude);
		ui->phase->setValue(ptr->phase);
		ui->label_path->setText(ptr->file);
		ui->mathWidget->setFunction(ptr->function);
		ui->mathFrequency->setValue(ptr->math_freq);

		if (!ptr->file.isNull()) {
			auto info = QFileInfo(ptr->file);
			ui->label_size->setText(QString("%1 ").arg(
						info.size() / sizeof(float))
					+ tr("samples"));
		} else {
			ui->label_size->setText("");
		}

		ui->type->setCurrentIndex(sg_waveform_to_idx(ptr->waveform));

		renameConfigPanel();
		ui->tabWidget->setCurrentIndex((int) ptr->type);
		updatePreview();

		ui->rightMenu->toggleMenu(true);
	}
}

void adiscope::SignalGenerator::toggleRightMenu(QPushButton *btn)
{
	unsigned int id = btn->property("id").toUInt();
	bool open = !settings_group->checkedButton();

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

bool SignalGenerator::use_oversampling(const struct iio_device *dev)
{
	if (!iio_device_find_attr(dev, "oversampling_ratio"))
		return false;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_enabled(chn))
			continue;

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);

		switch (ptr->type) {
		case SIGNAL_TYPE_WAVEFORM:
			/* We only want oversampling for square waveforms */
			if (ptr->waveform == SG_SQR_WAVE)
				return true;
		default:
			break;
		}
	}

	return false;
}

QVector<unsigned long> SignalGenerator::get_available_sample_rates(
		const struct iio_device *dev)
{
	QVector<unsigned long> values;
	char buf[1024];
	int ret;

	ret = iio_device_attr_read(dev, "sampling_frequency_available",
			buf, sizeof(buf));
	if (ret > 0) {
		QStringList list = QString::fromUtf8(buf).split(' ');

		for (auto it = list.cbegin(); it != list.cend(); ++it)
			values.append(it->toULong());
	}

	if (values.empty()) {
		ret = iio_device_attr_read(dev, "sampling_frequency",
				buf, sizeof(buf));
		if (!ret)
			values.append(QString::fromUtf8(buf).toULong());
	}

	qSort(values.begin(), values.end(), qGreater<unsigned long>());

	return values;
}

unsigned long SignalGenerator::get_best_sample_rate(
		const struct iio_device *dev)
{
	QVector<unsigned long> values = get_available_sample_rates(dev);

	/* When using oversampling, we actually want to generate the
	 * signal with the lowest sample rate possible. */
	if (use_oversampling(dev))
		qSort(values.begin(), values.end(), qLess<unsigned long>());

	/* Return the best sample rate that we can create a buffer for */
	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(dev, rate, true);
		if (buf_size)
			return rate;

		qDebug() << QString("Rate %1 not ideal").arg(rate);
	}

	/* If we can't find a perfect sample rate, use the highest one */
	if (use_oversampling(dev))
		qSort(values.begin(), values.end(), qGreater<unsigned long>());

	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(dev, rate);
		if (buf_size)
			return rate;

		qDebug() << QString("Rate %1 not possible").arg(rate);
	}

	throw std::runtime_error("Unable to calculate best sample rate");
}

unsigned long SignalGenerator::get_max_sample_rate(const struct iio_device *dev)
{
	QVector<unsigned long> values = get_available_sample_rates(dev);

	return values.takeFirst();
}

int SignalGenerator::set_sample_rate(const struct iio_device *dev,
		unsigned long rate)
{
	if (use_oversampling(dev)) {
		/* We assume that the rate requested here will always be a
		 * divider of the max sample rate */
		unsigned int ratio = sample_rate / rate;

		int ret = iio_device_attr_write_longlong(
				dev, "oversampling_ratio", ratio);
		if (ret < 0)
			return ret;

		rate = sample_rate;
		qDebug() << QString("Using oversampling with a ratio of %1")
			.arg(ratio);
	} else {
		int ret = iio_device_attr_write_longlong(
				dev, "oversampling_ratio", 1);
		if (ret < 0)
			return ret;
	}

set_sample_rate:
	return iio_device_attr_write_longlong(dev, "sampling_frequency", rate);
}

size_t SignalGenerator::gcd(size_t a, size_t b)
{
    for (;;) {
	    if (!a)
		    return b;
	    b %= a;

	    if (!b)
		    return a;
	    a %= b;
    }
}

size_t SignalGenerator::lcm(size_t a, size_t b)
{
	size_t temp = gcd(a, b);

	return temp ? (a / temp * b) : 0;
}

double SignalGenerator::get_best_ratio(double ratio, double max, double *fract)
{
	double max_it = max / ratio;
	double best_ratio = ratio;
	double best_fract = 1.0;

	for (double i = 1.0; i < max_it; i += 1.0) {
		double integral, new_ratio = i * ratio;
		double new_fract = modf(new_ratio, &integral);

		if (new_fract < best_fract) {
			best_fract = new_fract;
			best_ratio = new_ratio;
		}

		if (new_fract == 0.0)
			break;
	}

	qDebug() << QString("Input ratio %1, ratio: %2 (fract left %3)")
		.arg(ratio).arg(best_ratio).arg(best_fract);
	if (fract)
		*fract = best_fract;
	return best_ratio;
}

size_t SignalGenerator::get_samples_count(const struct iio_device *dev,
		unsigned long rate, bool perfect)
{
	size_t max_buffer_size = 4 * 1024 * 1024 /
		(size_t) iio_device_get_sample_size(dev);
	size_t size = 1;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_enabled(chn))
			continue;

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);
		double ratio, fract;

		switch (ptr->type) {
		case SIGNAL_TYPE_WAVEFORM:
		case SIGNAL_TYPE_MATH:
			ratio = (double) rate / ptr->frequency;
			if (ptr->type == SIGNAL_TYPE_WAVEFORM
					&& ptr->waveform == SG_SIN_WAVE
					&& ratio < 2.5)
				return 0; /* rate too low */
			else if (ratio < 2.0)
				return 0; /* rate too low */

			/* The ratio must be even for square waveforms */
			if (perfect && (ptr->type == SIGNAL_TYPE_WAVEFORM)
					&& (ptr->waveform == SG_SQR_WAVE)
					&& (fmod(ratio, 2.0) != 0.0))
				return 0;

			ratio = get_best_ratio(ratio,
					(double) (max_buffer_size / 4), &fract);
			if (perfect && fract != 0.0)
				return 0;

			size = lcm(size, (size_t) ratio);
			break;
		case SIGNAL_TYPE_CONSTANT:
		case SIGNAL_TYPE_BUFFER:
		default:
			break;
		}
	}

	/* The buffer size must be a multiple of 4 */
	while (size & 0x3)
		size <<= 1;

	/* The buffer size shouldn't be too small */
	while (size < min_buffer_size)
		size <<= 1;

	if (size > max_buffer_size)
		return 0;

	return size;
}

double SignalGenerator::vlsb_of_channel(const char *channel,
	const char *dev_parent)
{
	double vlsb = -1;

	for (auto it = channels_vlsb.begin(); it != channels_vlsb.end(); ++it) {
		struct iio_channel *chn = (*it).first;
		const struct iio_device *dev = iio_channel_get_device(chn);

		if (!strcmp(iio_channel_get_id(chn), channel) &&
			!strcmp(iio_device_get_name(dev), dev_parent)) {
			vlsb = (*it).second;
			break;
		}
	}

	return vlsb;
}

void SignalGenerator::set_vlsb_of_channel(const char *channel,
	const char *dev_parent, double vlsb)
{
	for (auto it = channels_vlsb.begin(); it != channels_vlsb.end(); ++it) {
		struct iio_channel *chn = (*it).first;
		const struct iio_device *dev = iio_channel_get_device(chn);

		if (!strcmp(iio_channel_get_id(chn), channel) &&
			!strcmp(iio_device_get_name(dev), dev_parent)) {
			(*it).second = vlsb;

			break;
		}
	}
}

bool SignalGenerator_API::running() const
{
	return gen->ui->run_button->isChecked();
}

void SignalGenerator_API::run(bool en)
{
	gen->ui->run_button->setChecked(en);
}

QList<int> SignalGenerator_API::getMode() const
{
	QList<int> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(static_cast<int>(ptr->type));
	}

	return list;
}

void SignalGenerator_API::setMode(const QList<int>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->type = static_cast<enum SIGNAL_TYPE>(list.at(i));
	}

	gen->ui->tabWidget->setCurrentIndex(gen->getCurrentData()->type);
}

QList<double> SignalGenerator_API::getConstantValue() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(static_cast<double>(ptr->constant));
	}

	return list;
}

void SignalGenerator_API::setConstantValue(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->constant = static_cast<float>(list.at(i));
	}

	gen->ui->constantValue->setValue(gen->getCurrentData()->constant);
}

QList<int> SignalGenerator_API::getWaveformType() const
{
	QList<int> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(SignalGenerator::sg_waveform_to_idx(ptr->waveform));
	}

	return list;
}

void SignalGenerator_API::setWaveformType(const QList<int>& list)
{
	if (list.size() != gen->channels.size())
		return;

	enum sg_waveform types[] = {
		SG_SIN_WAVE,
		SG_SQR_WAVE,
		SG_TRI_WAVE,
		SG_SAW_WAVE,
		SG_INV_SAW_WAVE,
	};

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->waveform = types[list.at(i)];

		if (i == gen->currentChannel)
			gen->ui->type->setCurrentIndex(list.at(i));
	}
}

QList<double> SignalGenerator_API::getWaveformAmpl() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(ptr->amplitude);
	}

	return list;
}

void SignalGenerator_API::setWaveformAmpl(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->amplitude = list.at(i);
	}

	gen->ui->amplitude->setValue(gen->getCurrentData()->amplitude);
}

QList<double> SignalGenerator_API::getWaveformFreq() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(ptr->frequency);
	}

	return list;
}

void SignalGenerator_API::setWaveformFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->frequency = list.at(i);
	}

	gen->ui->frequency->setValue(gen->getCurrentData()->frequency);
}

QList<double> SignalGenerator_API::getWaveformOfft() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(static_cast<double>(ptr->offset));
	}

	return list;
}

void SignalGenerator_API::setWaveformOfft(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->offset = static_cast<float>(list.at(i));
	}

	gen->ui->offset->setValue(gen->getCurrentData()->offset);
}

QList<double> SignalGenerator_API::getWaveformPhase() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(ptr->phase);
	}

	return list;
}

void SignalGenerator_API::setWaveformPhase(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->phase = list.at(i);
	}

	gen->ui->phase->setValue(gen->getCurrentData()->phase);
}

QList<double> SignalGenerator_API::getMathFreq() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(ptr->math_freq);
	}

	return list;
}

void SignalGenerator_API::setMathFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->math_freq = list.at(i);
	}

	gen->ui->mathFrequency->setValue(gen->getCurrentData()->math_freq);
}

QList<QString> SignalGenerator_API::getMathFunction() const
{
	QList<QString> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		list.append(ptr->function);
	}

	return list;
}

void SignalGenerator_API::setMathFunction(const QList<QString>& list)
{
	if (list.size() != gen->channels.size())
		return;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(&gen->channels[i]->first);

		ptr->function = list.at(i);
	}

	if (gen->getCurrentData()->type == SIGNAL_TYPE_MATH) {
		gen->ui->mathWidget->setFunction(
				gen->getCurrentData()->function);
	}
}
