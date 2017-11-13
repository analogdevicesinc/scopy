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
#include "channel_widget.hpp"

#include <cmath>

#include <QBrush>
#include <QFileDialog>
#include <QFileInfo>
#include <QPalette>
#include <QSharedPointer>
#include <QElapsedTimer>

#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/file_source.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/int_to_float.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/blocks/multiply_const_ff.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/nop.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink_s.h>
#include <gnuradio/filter/fractional_resampler_ff.h>
#include <gnuradio/filter/rational_resampler_base_fff.h>
#include <gnuradio/filter/pfb_arb_resampler_fff.h>
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
	// SIGNAL_TYPE_CONSTANT
	float constant;
	// SIGNAL_TYPE_WAVEFORM
	double amplitude;
	float offset;
	double frequency;
	double phase;
	enum sg_waveform waveform;
	// SIGNAL_TYPE_BUFFER
	double file_sr;
	double file_amplitude;
	double file_offset;
	unsigned long file_phase;
	unsigned long file_nr_of_samples;
	QString file;
	bool file_loaded;
	// SIGNAL_TYPE_MATH
	QString function;
	double math_freq;
};
Q_DECLARE_METATYPE(QSharedPointer<signal_generator_data>);

struct adiscope::time_block_data {
	scope_sink_f::sptr time_block;
	unsigned long nb_channels;
};

static double temp_sample_rate;

SignalGenerator::SignalGenerator(struct iio_context *_ctx,
                                 QList<std::shared_ptr<GenericDac>> dacs, Filter *filt,
                                 QPushButton *runButton, QJSEngine *engine, ToolLauncher *parent) :
	Tool(_ctx, runButton, new SignalGenerator_API(this), "Signal Generator",
	     parent),
	ui(new Ui::SignalGenerator),
	time_block_data(new adiscope::time_block_data),
	dacs(dacs),
	currentChannel(0), sample_rate(0),
	settings_group(new QButtonGroup(this)),nb_points(NB_POINTS)
{
	zoomT1=0;
	zoomT2=1;
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	this->plot = new OscilloscopePlot(this);

	QVector<struct iio_channel *> iio_channels;

	for (int i = 0; i < dacs.size(); i++) {
		struct iio_device *dev = dacs[i]->iio_dac_dev();

		unsigned long dev_sample_rate = get_max_sample_rate(dev);

		if (dev_sample_rate > sample_rate) {
			sample_rate = dev_sample_rate;
		}

		QList<struct iio_channel *> dac_channels =
		        dacs[i]->dacChannelList();

		for (int j = 0; j < dac_channels.size(); j++) {
			iio_channels.append(dac_channels[j]);
			channel_dac.push_back(QPair<struct iio_channel *,
			                      std::shared_ptr<GenericDac>>(dac_channels[j],
			                                      dacs[i]));
		}
	}

	max_sample_rate = sample_rate;

	/* FIXME: TODO: Move this into a HW class / lib M2k */
	struct iio_device *fabric = iio_context_find_device(ctx, "m2k-fabric");

	if (fabric) {
		this->amp1 = iio_device_find_channel(fabric, "voltage0", true);
		this->amp2 = iio_device_find_channel(fabric, "voltage1", true);

		if (amp1 && amp2) {
			iio_channel_attr_write_bool(amp1, "powerdown", true);
			iio_channel_attr_write_bool(amp2, "powerdown", true);
		}
	}

	/* Max amplitude by default */
	ui->amplitude->setValue(ui->amplitude->maxValue());

	/* Set max frequency according to max sample rate */
	ui->fileSampleRate->setMinValue(0.001);
	ui->frequency->setMaxValue((sample_rate / 2) - 1);
	ui->mathFrequency->setMaxValue((sample_rate / 2) - 1);
	ui->fileSampleRate->setMaxValue((sample_rate / 2) - 1);

	/* (lowest freq * 100 * 1000) frequency by default */
	ui->frequency->setValue(ui->frequency->minValue() * 1000 * 1000.0);
	ui->fileSampleRate->setValue(ui->fileSampleRate->minValue() * 100 * 1000.0);
	ui->fileAmplitude->setValue(ui->fileAmplitude->maxValue()/2);
	ui->fileOffset->setValue(0);
	ui->filePhase->setValue(0);
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
		ptr->file_sr = ui->fileSampleRate->value();
		ptr->file_amplitude = ui->fileAmplitude->value();
		ptr->file_offset = ui->fileOffset->value();
		ptr->file_phase = ui->filePhase->value();
		ptr->file_loaded=false;

		ptr->type = SIGNAL_TYPE_CONSTANT;
		ptr->id = i;

		ChannelWidget *cw = new ChannelWidget(i, false, true, QColor());

		const char *ch_name = iio_channel_get_name(chn);
		std::string s = "Channel ";

		if (!ch_name) {
			s += std::to_string(i + 1);
			ch_name = s.c_str();
		}

		cw->setFullName(ch_name);
		cw->enableButton()->setText(cw->fullName());


		cw->setProperty("signal_generator_data",
		                QVariant::fromValue(ptr));
		cw->setProperty("channel",
		                qVariantFromValue((void *) chn));
		iio_channel_set_data(chn, cw);

		ui->channelsList->addWidget(cw);

		connect(cw, SIGNAL(enabled(bool)),
		        SLOT(channelWidgetEnabled(bool)));

		connect(cw, SIGNAL(menuToggled(bool)),
		        SLOT(channelWidgetMenuToggled(bool)));

		settings_group->addButton(cw->menuButton());

		channels.append(cw);

		if (i == 0) {
			cw->menuButton()->setChecked(true);
		}
	}

	time_block_data->nb_channels = nb_channels;
	time_block_data->time_block = scope_sink_f::make(
	                                      nb_points, sample_rate,
	                                      "Signal Generator", nb_channels,
	                                      static_cast<QWidget *>(plot));

	/* Attach all curves by default */
	plot->registerSink(time_block_data->time_block->name(),
	                   nb_channels, nb_points);

	/* This must be done after attaching the curves; otherwise
	 * plot->getLineColor(i) returns black. */
	for (unsigned int i = 0; i < nb_channels; i++) {
		channels[i]->setColor(plot->getLineColor(i));
	}

	plot->disableLegend();
	plot->setPaletteColor(QColor("black"));

	plot->setVertUnitsPerDiv(AMPLITUDE_VOLTS * 2.0 / 10.0);

	plot->setHorizUnitsPerDiv((double) nb_points /
	                          ((double) sample_rate * 10.0));
	plot->setHorizOffset((double) nb_points /
	                     ((double) sample_rate * 2.0));
	plot->zoomBaseUpdate();

	ui->plot->addWidget(plot, 0, 0);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_SIGNAL_GENERATOR)));
	api->load(*settings);
	api->js_register(engine);

	renameConfigPanel();
	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
	        SLOT(rightMenuFinished(bool)));

	connect(ui->constantValue, SIGNAL(valueChanged(double)),
	        SLOT(constantValueChanged(double)));

	connect(ui->amplitude, SIGNAL(valueChanged(double)),
	        this, SLOT(amplitudeChanged(double)));

	connect(ui->fileAmplitude, SIGNAL(valueChanged(double)),
		this, SLOT(fileAmplitudeChanged(double)));
	connect(ui->fileOffset, SIGNAL(valueChanged(double)),
		this, SLOT(fileOffsetChanged(double)));
	connect(ui->filePhase, SIGNAL(valueChanged(double)),
		this, SLOT(filePhaseChanged(double)));
	connect(ui->fileSampleRate, SIGNAL(valueChanged(double)),
		this, SLOT(fileSampleRateChanged(double)));
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


	connect(plot->getZoomer(),SIGNAL(zoomed(QRectF)),this,SLOT(rescale()));

	resetZoom();
}

SignalGenerator::~SignalGenerator()
{
	ui->run_button->setChecked(false);

	api->save(*settings);
	delete api;

	delete plot;
	delete ui;
	delete time_block_data;
}

void SignalGenerator::resetZoom()
{

	double period=0.0;
	unsigned int slowSignalId;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if ((*it)->enableButton()->isChecked()) {
			auto ptr = getData((*it));
			switch (ptr->type) {
			case SIGNAL_TYPE_CONSTANT:
				break;
			case SIGNAL_TYPE_BUFFER:
				if(ptr->file_loaded)
				{
					auto length=(1/ptr->file_sr)*ptr->file_nr_of_samples;
					if(period<length)
					{
						period=length;
						slowSignalId = ptr->id;
					}
				}
				break;

			case SIGNAL_TYPE_WAVEFORM:
				if (period< 1 / (ptr->frequency)) {
					period=(1/ptr->frequency);
					slowSignalId = ptr->id;
				}

				break;

			case SIGNAL_TYPE_MATH:
				if (period< 1 / (ptr->math_freq)) {
					period=(1/ptr->math_freq);
					slowSignalId = ptr->id;
				}

				break;

			default:
				break;

			}
		}
	}

	period*=2; // show 2 periods - ideally this would be an instrument preference/GUI control
	if(period==0.0) // prevent empty graph
		period=0.1;

	plot->setVertUnitsPerDiv(1);
	plot->setVertOffset(0);
	plot->setHorizUnitsPerDiv(period/10);
	plot->setHorizOffset(period/2);
	plot->zoomBaseUpdate();
	rescale();
	plot->DetachCurve(slowSignalId);
	plot->AttachCurve(slowSignalId);
}

void SignalGenerator::rescale()
{

	auto hOffset=plot->HorizOffset();
	auto hUnitsPerDiv=plot->HorizUnitsPerDiv();
	auto xDivisions=10;


	zoomT1=hOffset-hUnitsPerDiv*(xDivisions/2);
	zoomT2=hOffset+hUnitsPerDiv*(xDivisions/2);

	zoomT1OnScreen=hOffset-hUnitsPerDiv*(xDivisions/2);
	zoomT2OnScreen=hOffset+hUnitsPerDiv*(xDivisions/2);

	auto deltaT= zoomT2-zoomT1;

	long max_sample_rate=750000000;
	nb_points=NB_POINTS;
	sample_rate=nb_points/deltaT;

	if (sample_rate>max_sample_rate) {
		nb_points=nb_points / (long)(sample_rate/max_sample_rate);
		sample_rate=max_sample_rate;

	}

	long startSample=sample_rate*zoomT1;
	plot->setDataStartingPoint(startSample);
	plot->setSampleRate(sample_rate,1,"Hz");
	time_block_data->time_block->set_nsamps(nb_points);
	time_block_data->time_block->set_samp_rate(sample_rate);
	updatePreview();
	plot->replot();
}

void SignalGenerator::constantValueChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->constant != (float) value) {
		ptr->constant = (float) value;
		resetZoom();
	}
}

void SignalGenerator::amplitudeChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->amplitude != value) {
		ptr->amplitude = value;
		resetZoom();
	}
}

void SignalGenerator::fileAmplitudeChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_amplitude != value) {
		ptr->file_amplitude = value;
		resetZoom();
	}
}

void SignalGenerator::fileOffsetChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_offset!= value) {
		ptr->file_offset= value;
		resetZoom();
	}
}

void SignalGenerator::filePhaseChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_phase!= value) {
		ptr->file_phase= value;
		resetZoom();
	}
}

void SignalGenerator::offsetChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->offset != (float) value) {
		ptr->offset = (float) value;
		resetZoom();
	}
}

void SignalGenerator::fileSampleRateChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_sr != value) {
		ptr->file_sr = value;
		resetZoom();
	}
}


void SignalGenerator::frequencyChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->frequency != value) {
		ptr->frequency = value;
		resetZoom();
	}
}

void SignalGenerator::mathFreqChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->math_freq != value) {
		ptr->math_freq = value;

		resetZoom();
	}
}

void SignalGenerator::phaseChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->phase != value) {
		ptr->phase = value;
		resetZoom();
	}
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

	if (ptr->waveform != types[val]) {
		ptr->waveform = types[val];
		resetZoom();
	}
}

QSharedPointer<signal_generator_data> SignalGenerator::getCurrentData()
{
	return getData(channels[currentChannel]);
}

QSharedPointer<signal_generator_data> SignalGenerator::getData(QWidget *obj)
{
	auto var = obj->property("signal_generator_data");
	return var.value<QSharedPointer<signal_generator_data>>();
}

void SignalGenerator::tabChanged(int index)
{
	auto ptr = getCurrentData();

	if (ptr->type != (enum SIGNAL_TYPE) index) {
		ptr->type = (enum SIGNAL_TYPE) index;
		resetZoom();
	}
}

void SignalGenerator::updatePreview()
{
	gr::top_block_sptr top = make_top_block("Signal Generator Update");
	unsigned int i = 0;
	bool enabled = false;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		basic_block_sptr source;

		if ((*it)->enableButton()->isChecked()) {

			source = getSource((*it), sample_rate, top, true);
			enabled = true;
		} else {
			source = blocks::nop::make(sizeof(float));
		}

		auto head = blocks::head::make(sizeof(float), nb_points);
		top->connect(source, 0, head, 0);

		top->connect(head, 0, time_block_data->time_block, i++);
	}

	QElapsedTimer timer;
	timer.start();
	top->run();
	qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";
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
	ptr->file_nr_of_samples=info.size() / sizeof(float);
	this->ui->label_size->setText(QString("%1 ").arg(
	                                      info.size() / sizeof(float)) + tr("samples"));
	ptr->file_loaded=true;
	ptr->file_amplitude=5.0;
	ptr->file_offset=0;
	ptr->file_phase=0;
	ui->fileAmplitude->setValue(ptr->file_amplitude);
	ui->fileOffset->setValue(ptr->file_offset);
	ui->filePhase->setValue(ptr->file_phase);
	ui->filePhase->setMaxValue(ptr->file_nr_of_samples);
	resetZoom();
}

void SignalGenerator::start()
{
	QVector<struct iio_channel *> enabled_channels;

	/* Avoid from being started twice */
	if (buffers.size() > 0) {
		return;
	}

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if (!(*it)->enableButton()->isChecked()) {
			continue;
		}

		void *ptr = (*it)->property("channel").value<void *>();
		enabled_channels.append(static_cast<struct iio_channel *>(ptr));
	}

	do {
		const struct iio_device *dev =
		        iio_channel_get_device(enabled_channels[0]);

		/* First, disable all the channels of this device */
		unsigned int nb = iio_device_get_channels_count(dev);

		for (unsigned int i; i < nb; i++) {
			iio_channel_disable(iio_device_get_channel(dev, i));
		}

		/* Then enable the channels that we want */
		for (auto each : enabled_channels) {
			if (dev == iio_channel_get_device(each)) {
				iio_channel_enable(each);
			}
		}

		/* Enable the (optional) DMA sync */
		iio_device_attr_write_bool(dev, "dma_sync", true);

		unsigned long best_rate = get_best_sample_rate(dev);
		size_t samples_count = get_samples_count(dev, best_rate);

		/* Create the IIO buffer */
		struct iio_buffer *buf = iio_device_create_buffer(
		                                 dev, samples_count, true);

		if (!buf) {
			throw std::runtime_error("Unable to create buffer");
		}

		qDebug() << QString("Created buffer with %1 samples at %2 SPS for device %3")
		         .arg(samples_count).arg(best_rate).arg(
		                 iio_device_get_name(dev) ?:
		                 iio_device_get_id(dev));

		unsigned long final_rate;
		unsigned long oversampling;

		calc_sampling_params(dev, best_rate, final_rate,
		                     oversampling);

		for (auto each : enabled_channels) {
			if (dev != iio_channel_get_device(each)) {
				continue;
			}

			enabled_channels.remove(enabled_channels.indexOf(each));

			top_block = gr::make_top_block("Signal Generator");

			void *ptr = iio_channel_get_data(each);
			QWidget *w = static_cast<QWidget *>(ptr);
			auto source = getSource(w, best_rate, top_block);

			float volts_to_raw_coef;
			double vlsb = 1;
			double corr = 1; // interpolation correction
			auto pair_it = std::find_if(channel_dac.begin(),
			                            channel_dac.end(),
			                            [&each](const QPair<struct iio_channel *,
			std::shared_ptr<GenericDac>>& element) {
				return element.first == each;
			}
			                           );

			if (pair_it != channel_dac.end()) {
				std::shared_ptr<GenericDac> dac =(*pair_it).second;
				vlsb = dac->vlsb();
				auto m2k_dac = std::dynamic_pointer_cast<M2kDac>
				               (dac);

				if (m2k_dac) {
					corr = m2k_dac->compTable(final_rate);
				}
			}

			// DAC_RAW = (-Vout / (voltage corresponding to a LSB));
			// Multiplying with 16 because the HDL considers the DAC data as 16 bit
			// instead of 12 bit(data is shifted to the left)
			// Divide by corr when interpolation is used
			volts_to_raw_coef = (-1 * (1 / vlsb) * 16) / corr;

			auto f2s = blocks::float_to_short::make(1,
			                                        volts_to_raw_coef);

			auto head = blocks::head::make(
			                    sizeof(short), samples_count);

			auto vector = blocks::vector_sink_s::make();

			top_block->connect(source, 0, f2s, 0);
			top_block->connect(f2s, 0, head, 0);
			top_block->connect(head, 0, vector, 0);

			top_block->run();

			const std::vector<short>& samples = vector->data();
			const short *data = samples.data();

			iio_channel_write(each, buf, data,
			                  samples_count * sizeof(short));
		}

		if (iio_device_find_attr(dev, "oversampling_ratio")) {
			iio_device_attr_write_longlong(dev,
			                               "oversampling_ratio", oversampling);
		}

		iio_device_attr_write_longlong(dev, "sampling_frequency",
		                               final_rate);

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
	for (auto each : buffers) {
		iio_buffer_destroy(each);
	}

	buffers.clear();
}

void SignalGenerator::startStop(bool pressed)
{

	if (pressed) {
		start();
	} else {
		stop();
	}

	if (amp1 && amp2) {
		/* FIXME: TODO: Move this into a HW class / lib M2k */
		iio_channel_attr_write_bool(amp1, "powerdown", !pressed);
		iio_channel_attr_write_bool(amp2, "powerdown", !pressed);
	}

	setDynamicProperty(ui->run_button, "running", pressed);
}

void SignalGenerator::setFunction(const QString& function)
{
	auto ptr = getCurrentData();

	if (ptr->function != function) {
		ptr->function = function;
		resetZoom();
	}
}

basic_block_sptr SignalGenerator::getSignalSource(gr::top_block_sptr top,
                unsigned long samp_rate, struct signal_generator_data& data,
                double phase_correction)
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

	phase = data.phase + phase_correction;

	if (data.waveform == SG_TRI_WAVE) {
		phase = std::fmod(phase + 90.0, 360.0);
	} else if (data.waveform == SG_SQR_WAVE) {
		phase = std::fmod(phase + 180.0, 360.0);
	} else if (phase<0) {
		phase=phase+360.0;
	}

	if (inv_saw_wave) {
		waveform = analog::GR_SAW_WAVE;
		offset = -data.offset - (float) data.amplitude / 2.0;
	} else {
		waveform = static_cast<analog::gr_waveform_t>(data.waveform);
	}

	auto src = analog::sig_source_f::make(samp_rate, waveform,
	                                      data.frequency, amplitude, offset);

	uint64_t to_skip = samp_rate * phase / (data.frequency * 360.0);
	auto skip_head = blocks::skiphead::make(sizeof(float),(uint64_t)to_skip);
	top->connect(src, 0, skip_head, 0);

	if (!inv_saw_wave) {
		return skip_head;
	} else {
		auto mult = blocks::multiply_const_ff::make(-1.0);
		top->connect(skip_head, 0, mult, 0);
		//top->connect(delay, 0, mult, 0);

		return mult;
	}
}

gr::basic_block_sptr SignalGenerator::getSource(QWidget *obj,
                unsigned long samp_rate, gr::top_block_sptr top, bool preview)
{
	auto ptr = getData(obj);
	enum SIGNAL_TYPE type = ptr->type;
	double phase=0.0;

	switch (type) {
	case SIGNAL_TYPE_CONSTANT:
		return analog::sig_source_f::make(samp_rate,
		                                  analog::GR_CONST_WAVE, 0, 0,
		                                  ptr->constant);

	case SIGNAL_TYPE_WAVEFORM:
		if (preview) {
			int full_periods=(int)((double)zoomT1OnScreen*ptr->frequency);
			double phase_in_time = zoomT1OnScreen - full_periods/ptr->frequency;
			phase = (phase_in_time*ptr->frequency) * 360.0;
		}

		return getSignalSource(top, samp_rate, *ptr, phase);

	case SIGNAL_TYPE_BUFFER:
		if (!ptr->file.isNull()) {
			auto str = ptr->file.toStdString();
			auto fs = blocks::file_source::make(
				       sizeof(float), str.c_str(), true);

			auto ratio=ptr->file_sr/samp_rate;
			auto mult = blocks::multiply_const_ff::make(ptr->file_amplitude);
			top->connect(fs,0,mult,0);
			auto add = blocks::add_const_ff::make(ptr->file_offset);
			top->connect(mult,0,add,0);
			auto phase_skip = blocks::skiphead::make(sizeof(float),ptr->file_phase);
			top->connect(add,0,phase_skip,0);

			if(preview)
			{
				//auto res = blocks::throttle::make(sizeof(float),ptr->file_sr);//
				auto res = filter::fractional_resampler_ff::make(0,ratio);
				//auto res=filter::pfb_arb_resampler_fff::make(ratio,std::vector<float>(1,1));
				/*auto interp=ratio;
				auto dec=1;
				while(interp<1)
				{
					interp*=10;
					//dec*=10;
				}
				auto res=filter::rational_resampler_base_fff::make(interp,dec,{1});*/
				top->connect(phase_skip,0,res,0);
				auto buffer_freq = ptr->file_sr/(double)ptr->file_nr_of_samples;
				int full_periods=(int)((double)zoomT1OnScreen * buffer_freq);
				double phase_in_time = zoomT1OnScreen - (full_periods/buffer_freq);
				unsigned long samples_to_skip = phase_in_time * samp_rate;
				auto skip = blocks::skiphead::make(sizeof(float),samples_to_skip);
				top->connect(res,0,skip,0);
				return skip;
			}
			else
			{
				return phase_skip;
			}
		}
		break;

	case SIGNAL_TYPE_MATH:
		if (!ptr->function.isEmpty()) {
			auto str = ptr->function.toStdString();

			if (preview) {
				int full_periods=(int)((double)zoomT1OnScreen*ptr->math_freq);
				double phase_in_time = zoomT1OnScreen - full_periods/ptr->math_freq;
				phase = (phase_in_time*ptr->math_freq) * 360.0;
				auto src = iio::iio_math_gen::make(samp_rate,
				                                   ptr->math_freq, str);

				uint64_t to_skip = samp_rate * phase / (ptr->math_freq * 360.0);
				auto skip_head = blocks::skiphead::make(sizeof(float),(uint64_t)to_skip);
				top->connect(src, 0, skip_head, 0);
				return skip_head;

			} else {
				return iio::iio_math_gen::make(samp_rate,
				                               ptr->math_freq, str);
			}


		}

	default:
		break;
	}

	return blocks::nop::make(sizeof(float));
}

void adiscope::SignalGenerator::channelWidgetEnabled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	int id = cw->id();

	if (en) {
		plot->AttachCurve(id);
	} else {
		plot->DetachCurve(id);
	}

	resetZoom();
	plot->replot();

	bool enable_run = en;

	if (!en) {
		for (auto it = channels.begin();
		     !enable_run && it != channels.end(); ++it) {
			enable_run = (*it)->enableButton()->isChecked();
		}
	}

	ui->run_button->setEnabled(enable_run);
	run_button->setEnabled(enable_run);
}

void adiscope::SignalGenerator::triggerRightMenuToggle(int chIdx, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
		        QPair<int, bool>(chIdx, checked));
	} else {
		updateAndToggleMenu(chIdx, checked);
	}
}

void adiscope::SignalGenerator::channelWidgetMenuToggled(bool checked)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	currentChannel = cw->id();
	plot->setActiveVertAxis(cw->id());

	triggerRightMenuToggle(cw->id(), checked);
}

void adiscope::SignalGenerator::renameConfigPanel()
{
	ui->config_panel->setTitle(QString("Configuration for %1").arg(
	                                   channels[currentChannel]->fullName()));
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

void SignalGenerator::updateRightMenuForChn(int chIdx)
{
	auto ptr = getData(channels[chIdx]);

	ui->constantValue->setValue(ptr->constant);
	ui->frequency->setValue(ptr->frequency);
	ui->offset->setValue(ptr->offset);
	ui->amplitude->setValue(ptr->amplitude);
	ui->phase->setValue(ptr->phase);
	ui->label_path->setText(ptr->file);
	ui->mathWidget->setFunction(ptr->function);
	ui->mathFrequency->setValue(ptr->math_freq);
	ui->fileSampleRate->setValue(ptr->file_sr);
	ui->fileOffset->setValue(ptr->file_offset);
	ui->filePhase->setValue(ptr->file_phase);
	ui->fileAmplitude->setValue(ptr->file_amplitude);

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
}

void SignalGenerator::updateAndToggleMenu(int chIdx, bool open)
{
	if (open) {
		updateRightMenuForChn(chIdx);
	}

	ui->rightMenu->toggleMenu(open);
}

void adiscope::SignalGenerator::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		int chIdx = pair.first;
		bool open = pair.second;

		updateAndToggleMenu(chIdx, open);
	}
}

bool SignalGenerator::use_oversampling(const struct iio_device *dev)
{
	if (!iio_device_find_attr(dev, "oversampling_ratio")) {
		return false;
	}

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_enabled(chn)) {
			continue;
		}

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);

		switch (ptr->type) {
		case SIGNAL_TYPE_WAVEFORM:

			/* We only want oversampling for square waveforms */
			if (ptr->waveform == SG_SQR_WAVE) {
				return true;
			}
			break;
		case SIGNAL_TYPE_BUFFER:
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

		for (auto it = list.cbegin(); it != list.cend(); ++it) {
			values.append(it->toULong());
		}
	}

	if (values.empty()) {
		ret = iio_device_attr_read(dev, "sampling_frequency",
		                           buf, sizeof(buf));

		if (!ret) {
			values.append(QString::fromUtf8(buf).toULong());
		}
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

	if(sample_rate_forced(dev))
	{
		return get_forced_sample_rate(dev);
	}

	if (use_oversampling(dev)) {
		qSort(values.begin(), values.end(), qLess<unsigned long>());
	}

	/* Return the best sample rate that we can create a buffer for */
	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(dev, rate, true);

		if (buf_size) {
			return rate;
		}

		qDebug() << QString("Rate %1 not ideal").arg(rate);
	}

	/* If we can't find a perfect sample rate, use the highest one */
	if (use_oversampling(dev)) {
		qSort(values.begin(), values.end(), qGreater<unsigned long>());
	}


	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(dev, rate);

		if (buf_size) {
			return rate;
		}

		qDebug() << QString("Rate %1 not possible").arg(rate);
	}

	throw std::runtime_error("Unable to calculate best sample rate");
}

bool SignalGenerator::sample_rate_forced(const struct iio_device *dev)
{
	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_enabled(chn)) {
			continue;
		}

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);
		if(ptr->file_loaded && ptr->file_sr)
			return true;
	}
	return false;
}

unsigned long SignalGenerator::get_forced_sample_rate(const struct iio_device *dev)
{

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_enabled(chn)) {
			continue;
		}

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);
		if(ptr->file_loaded && ptr->file_sr)
			return ptr->file_sr;
	}
	return false;
}

unsigned long SignalGenerator::get_max_sample_rate(const struct iio_device *dev)
{
	QVector<unsigned long> values = get_available_sample_rates(dev);

	if (values.empty()) {
		return default_sample_rate;
	}

	return values.takeFirst();
}

void SignalGenerator::calc_sampling_params(const iio_device *dev,
                unsigned long rate, unsigned long& out_sample_rate,
                unsigned long& out_oversampling_ratio)
{
	if (use_oversampling(dev)) {
		/* We assume that the rate requested here will always be a
		 * divider of the max sample rate */
		out_oversampling_ratio = max_sample_rate / rate;
		out_sample_rate = max_sample_rate;

		qDebug() << QString("Using oversampling with a ratio of %1")
			 .arg(out_oversampling_ratio);
	} else {
		out_sample_rate = rate;
		out_oversampling_ratio = 1;
	}
}

size_t SignalGenerator::gcd(size_t a, size_t b)
{
	for (;;) {
		if (!a) {
			return b;
		}

		b %= a;

		if (!b) {
			return a;
		}

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

		if (new_fract == 0.0) {
			break;
		}
	}

	qDebug() << QString("Input ratio %1, ratio: %2 (fract left %3)")
	         .arg(ratio).arg(best_ratio).arg(best_fract);

	if (fract) {
		*fract = best_fract;
	}

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

		if (!iio_channel_is_enabled(chn)) {
			continue;
		}

		QWidget *w = static_cast<QWidget *>(iio_channel_get_data(chn));
		auto ptr = getData(w);
		double ratio, fract;

		switch (ptr->type) {
		case SIGNAL_TYPE_WAVEFORM:
		case SIGNAL_TYPE_MATH:
			if (ptr->type == SIGNAL_TYPE_WAVEFORM) {
				ratio = (double) rate / ptr->frequency;
			} else {
				ratio = (double) rate / ptr->math_freq;
			}

			// TODO: Figure out what the limit is on the generated signals
			/*if (ptr->type == SIGNAL_TYPE_WAVEFORM
					&& ptr->waveform == SG_SIN_WAVE
					&& ratio < 2.5)
				return 0; /* rate too low */
			//else
			if (ratio < 2.0) {
				return 0;        /* rate too low */
			}

			/* The ratio must be even for square waveforms */
			if (perfect && (ptr->type == SIGNAL_TYPE_WAVEFORM)
			    && (ptr->waveform == SG_SQR_WAVE)
			    && (fmod(ratio, 2.0) != 0.0)) {
				return 0;
			}

			ratio = get_best_ratio(ratio,
			                       (double)(max_buffer_size / 4), &fract);

			if (perfect && fract != 0.0) {
				return 0;
			}

			size = lcm(size, (size_t) ratio);
			break;

		case SIGNAL_TYPE_BUFFER:
			if(!ptr->file_loaded)
				return 0;
			if(perfect && rate!=ptr->file_sr)
				return 0;
			ratio = rate/ptr->file_sr;
			size=(ptr->file_nr_of_samples * ratio);
			break;
		case SIGNAL_TYPE_CONSTANT:
		default:
			break;
		}
	}

	/* The buffer size must be a multiple of 4 */
	while (size & 0x3) {
		size <<= 1;
	}

	/* The buffer size shouldn't be too small */
	while (size < min_buffer_size) {
		size <<= 1;
	}

	if (size > max_buffer_size) {
		return 0;
	}

	return size;
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
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<int>(ptr->type));
	}

	return list;
}

void SignalGenerator_API::setMode(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->type = static_cast<enum SIGNAL_TYPE>(list.at(i));
	}

	gen->ui->tabWidget->setCurrentIndex(gen->getCurrentData()->type);
}

QList<double> SignalGenerator_API::getConstantValue() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<double>(ptr->constant));
	}

	return list;
}

void SignalGenerator_API::setConstantValue(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->constant = static_cast<float>(list.at(i));
	}

	gen->ui->constantValue->setValue(gen->getCurrentData()->constant);
}

QList<int> SignalGenerator_API::getWaveformType() const
{
	QList<int> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(SignalGenerator::sg_waveform_to_idx(ptr->waveform));
	}

	return list;
}

void SignalGenerator_API::setWaveformType(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	enum sg_waveform types[] = {
		SG_SIN_WAVE,
		SG_SQR_WAVE,
		SG_TRI_WAVE,
		SG_SAW_WAVE,
		SG_INV_SAW_WAVE,
	};

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->waveform = types[list.at(i)];

		if (i == gen->currentChannel) {
			gen->ui->type->setCurrentIndex(list.at(i));
		}
	}
}

QList<double> SignalGenerator_API::getWaveformAmpl() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->amplitude);
	}

	return list;
}

void SignalGenerator_API::setWaveformAmpl(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->amplitude = list.at(i);
	}

	gen->ui->amplitude->setValue(gen->getCurrentData()->amplitude);
}

QList<double> SignalGenerator_API::getWaveformFreq() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->frequency);
	}

	return list;
}

void SignalGenerator_API::setWaveformFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->frequency = list.at(i);
	}

	gen->ui->frequency->setValue(gen->getCurrentData()->frequency);
}

QList<double> SignalGenerator_API::getWaveformOfft() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<double>(ptr->offset));
	}

	return list;
}

void SignalGenerator_API::setWaveformOfft(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->offset = static_cast<float>(list.at(i));
	}

	gen->ui->offset->setValue(gen->getCurrentData()->offset);
}

QList<double> SignalGenerator_API::getWaveformPhase() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->phase);
	}

	return list;
}

void SignalGenerator_API::setWaveformPhase(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->phase = list.at(i);
	}

	gen->ui->phase->setValue(gen->getCurrentData()->phase);
}

QList<double> SignalGenerator_API::getMathFreq() const
{
	QList<double> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->math_freq);
	}

	return list;
}

void SignalGenerator_API::setMathFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->math_freq = list.at(i);
	}

	gen->ui->mathFrequency->setValue(gen->getCurrentData()->math_freq);
}

QList<QString> SignalGenerator_API::getMathFunction() const
{
	QList<QString> list;

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->function);
	}

	return list;
}

void SignalGenerator_API::setMathFunction(const QList<QString>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (unsigned int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->function = list.at(i);
	}

	if (gen->getCurrentData()->type == SIGNAL_TYPE_MATH) {
		gen->ui->mathWidget->setFunction(
		        gen->getCurrentData()->function);
	}
}
