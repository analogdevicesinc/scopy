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
#include "network_analyzer.hpp"
#include "signal_generator.hpp"
#include "spinbox_a.hpp"
#include "ui_network_analyzer.h"

#include <gnuradio/analog/sig_source_c.h>
#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/blocks/complex_to_arg.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/moving_average_cc.h>
#include <gnuradio/blocks/multiply_cc.h>
#include <gnuradio/blocks/multiply_conjugate_cc.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink_f.h>
#include <gnuradio/blocks/vector_sink_s.h>
#include <gnuradio/top_block.h>

#include <boost/make_shared.hpp>

#include <QDebug>
#include <QThread>

#include <iio.h>

/* This should go away ASAP... */
#define DAC_BIT_COUNT   12
#define INTERP_BY_100_CORR 1.168 // correction value at an interpolation by 100
#define AMPLITUDE_VOLTS	5.0

using namespace adiscope;
using namespace gr;

NetworkAnalyzer::NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
		QPushButton *runButton, QJSEngine *engine,
		QWidget *parent) : QWidget(parent),
	ui(new Ui::NetworkAnalyzer), net_api(new NetworkAnalyzer_API(this))
{
	iio = iio_manager::get_instance(ctx,
			filt->device_name(TOOL_NETWORK_ANALYZER, 2));

	adc = filt->find_device(ctx, TOOL_NETWORK_ANALYZER, 2);
	dac1 = filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 0, true);
	dac2 = filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 1, true);
	if (!dac1 || !dac2)
		throw std::runtime_error("Unable to find channels in filter file");

	ui->setupUi(this);

	connect(ui->run_button, SIGNAL(toggled(bool)),
			this, SLOT(startStop(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
			runButton, SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)),
			ui->run_button, SLOT(setChecked(bool)));
	connect(this, &NetworkAnalyzer::sweepDone,
			[=]() {
		ui->run_button->setChecked(false);
	});


	ui->rightMenu->setMaximumWidth(0);

	const struct iio_device *dev1 = iio_channel_get_device(dac1);
	unsigned long max_samplerate1 =
		SignalGenerator::get_max_sample_rate(dev1);

	const struct iio_device *dev2 = iio_channel_get_device(dac2);
	unsigned long max_samplerate2 =
		SignalGenerator::get_max_sample_rate(dev2);

	unsigned long max_samplerate =
		std::min(max_samplerate1, max_samplerate2);

	ui->maxFreq->setMaxValue(max_samplerate / 2 - 1);

	connect(ui->minFreq, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));
	connect(ui->maxFreq, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));
	connect(ui->stepSize, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));

	net_api->load();
	net_api->js_register(engine);

	updateNumSamples();
}

NetworkAnalyzer::~NetworkAnalyzer()
{
	stop = true;
	thd.waitForFinished();

	net_api->save();
	delete net_api;

	delete ui;
}

void NetworkAnalyzer::updateNumSamples()
{
	double min = ui->minFreq->value();
	double max = ui->maxFreq->value();
	double step = ui->stepSize->value();
	unsigned int num_samples = 1 + (unsigned int)((max - min) / step);

	ui->dbgraph->setNumSamples(num_samples);
	ui->phasegraph->setNumSamples(num_samples);
	ui->xygraph->setNumSamples(num_samples);
	ui->nicholsgraph->setNumSamples(num_samples);
}

void NetworkAnalyzer::run()
{
	const struct iio_device *dev = iio_channel_get_device(dac1);

	const struct iio_device *dev1 = iio_channel_get_device(dac1);
	for (unsigned int i = 0; i < iio_device_get_channels_count(dev1); i++) {
		struct iio_channel *each = iio_device_get_channel(dev1, i);

		if (each == dac1 || each == dac2)
			iio_channel_enable(each);
		else
			iio_channel_disable(each);
	}

	const struct iio_device *dev2 = iio_channel_get_device(dac2);
	for (unsigned int i = 0; i < iio_device_get_channels_count(dev2); i++) {
		struct iio_channel *each = iio_device_get_channel(dev2, i);

		if (each == dac1 || each == dac2)
			iio_channel_enable(each);
		else
			iio_channel_disable(each);
	}

	for (double frequency = ui->minFreq->value();
			!stop && frequency <= ui->maxFreq->value();
			frequency += ui->stepSize->value()) {
		unsigned long rate = get_best_sin_sample_rate(dac1, frequency);
		size_t samples_count = get_sin_samples_count(
				dac1, rate, frequency);
		unsigned long adc_rate;

		double amplitude = ui->amplitude->value();
		double offset = ui->offset->value();

		/* We want at least 8 periods. */
		size_t in_samples_count = samples_count * 8;

		if (dev1 != dev2)
			iio_device_attr_write_bool(dev1, "dma_sync", true);

		struct iio_buffer *buf_dac1 = generateSinWave(dev1,
				frequency, amplitude, offset,
				rate, samples_count);
		if (!buf_dac1) {
			qCritical() << "Unable to create DAC buffer";
			break;
		}

		struct iio_buffer *buf_dac2 = nullptr;

		if (dev1 != dev2) {
			buf_dac2 = generateSinWave(dev2, frequency, amplitude,
					offset, rate, samples_count);
			if (!buf_dac2) {
				qCritical() << "Unable to create DAC buffer";
				break;
			}

			iio_device_attr_write_bool(dev1, "dma_sync", false);
		}

		adc_rate = get_best_adc_rate(frequency);
		iio_device_attr_write_longlong(adc,
				"sampling_frequency", adc_rate);

		/* Lock the flowgraph if we are already started */
		bool started = iio->started();
		if (started)
			iio->lock();

		/* Skip some data to make sure we'll get the waveform */
		auto skiphead1 = blocks::skiphead::make(sizeof(float),
				in_samples_count);
		auto skiphead2 = blocks::skiphead::make(sizeof(float),
				in_samples_count);
		auto id1 = iio->connect(skiphead1, 0, 0, true,
				in_samples_count);
		auto id2 = iio->connect(skiphead2, 1, 0, true,
				in_samples_count);

		/* We want at least 8 periods */
		unsigned long buffer_size = 8.0 * (double) adc_rate / frequency;
		iio->set_buffer_size(id1, buffer_size);
		iio->set_buffer_size(id2, buffer_size);

		qDebug() << "Scanning frequency" << frequency << "Hz at" <<
			adc_rate << "SPS," << buffer_size << "samples";

		auto f2c1 = blocks::float_to_complex::make();
		auto f2c2 = blocks::float_to_complex::make();
		iio->connect(skiphead1, 0, f2c1, 0);
		iio->connect(skiphead2, 0, f2c2, 0);

		auto null = blocks::null_source::make(sizeof(float));
		iio->connect(null, 0, f2c1, 1);
		iio->connect(null, 0, f2c2, 1);

		auto cosine = analog::sig_source_c::make(
				(unsigned int) adc_rate,
				gr::analog::GR_COS_WAVE, -frequency, 1.0);

		auto mult1 = blocks::multiply_cc::make();
		iio->connect(f2c1, 0, mult1, 0);
		iio->connect(cosine, 0, mult1, 1);

		auto mult2 = blocks::multiply_cc::make();
		iio->connect(f2c2, 0, mult2, 0);
		iio->connect(cosine, 0, mult2, 1);

		auto signal = boost::make_shared<signal_sample>();
		auto conj = blocks::multiply_conjugate_cc::make();

		auto avg1 = blocks::moving_average_cc::make(in_samples_count,
				2.0 / in_samples_count, in_samples_count);
		auto skiphead3 = blocks::skiphead::make(sizeof(gr_complex),
				in_samples_count - 1);
		auto c2m1 = blocks::complex_to_mag_squared::make();

		iio->connect(mult1, 0, avg1, 0);
		iio->connect(avg1, 0, skiphead3, 0);
		iio->connect(skiphead3, 0, c2m1, 0);
		iio->connect(skiphead3, 0, conj, 0);
		iio->connect(c2m1, 0, signal, 0);

		auto avg2 = blocks::moving_average_cc::make(in_samples_count,
				2.0 / in_samples_count, in_samples_count);
		auto skiphead4 = blocks::skiphead::make(sizeof(gr_complex),
				in_samples_count - 1);
		auto c2m2 = blocks::complex_to_mag_squared::make();

		iio->connect(mult2, 0, avg2, 0);
		iio->connect(avg2, 0, skiphead4, 0);
		iio->connect(skiphead4, 0, c2m2, 0);
		iio->connect(skiphead4, 0, conj, 1);
		iio->connect(c2m2, 0, signal, 1);

		auto c2a = blocks::complex_to_arg::make();
		iio->connect(conj, 0, c2a, 0);
		iio->connect(c2a, 0, signal, 2);

		bool got_it = false;
		float mag1 = 0.0f, mag2 = 0.0f, phase = 0.0f;

		connect(&*signal, &signal_sample::triggered,
				[&](const std::vector<float> values) {
			mag1 = values[0];
			mag2 = values[1];
			phase = values[2];
			got_it = true;
		});

		iio->start(id1);
		iio->start(id2);

		if (started)
			iio->unlock();

		do {
			QThread::msleep(10);
		} while (!got_it);

		iio->stop(id1);
		iio->stop(id2);

		started = iio->started();
		if (started)
			iio->lock();
		iio->disconnect(id1);
		iio->disconnect(id2);
		if (started)
			iio->unlock();

		iio_buffer_destroy(buf_dac1);
		if (buf_dac2)
			iio_buffer_destroy(buf_dac2);


		double mag = 10.0 * log10(mag1) - 10.0 * log10(mag2);
		qDebug() << "Frequency:" << frequency << " Mag diff:"
			<< mag << "Phase diff:" << phase;

		double phase_deg = phase * 180.0 / M_PI;

		QMetaObject::invokeMethod(ui->dbgraph,
				 "plot",
				 Qt::QueuedConnection,
				 Q_ARG(double, frequency),
				 Q_ARG(double, mag));

		QMetaObject::invokeMethod(ui->phasegraph,
				 "plot",
				 Qt::QueuedConnection,
				 Q_ARG(double, frequency),
				 Q_ARG(double, phase_deg));

		QMetaObject::invokeMethod(ui->xygraph,
				"plot",
				Qt::QueuedConnection,
				Q_ARG(double, phase_deg),
				Q_ARG(double, mag));

		QMetaObject::invokeMethod(ui->nicholsgraph,
				 "plot",
				 Qt::QueuedConnection,
				 Q_ARG(double, phase_deg),
				 Q_ARG(double, mag));
	}

	Q_EMIT sweepDone();
}

void NetworkAnalyzer::startStop(bool pressed)
{
	stop = !pressed;

	if (pressed) {
		ui->dbgraph->reset();
		ui->phasegraph->reset();
		ui->xygraph->reset();
		ui->nicholsgraph->reset();
		thd = QtConcurrent::run(this, &NetworkAnalyzer::run);
	} else {
		thd.waitForFinished();
	}

	setDynamicProperty(ui->run_button, "running", pressed);
}

size_t NetworkAnalyzer::get_sin_samples_count(const struct iio_channel *chn,
		unsigned long rate, double frequency)
{
	const struct iio_device *dev = iio_channel_get_device(chn);
	size_t max_buffer_size = 4 * 1024 * 1024 /
		(size_t) iio_device_get_sample_size(dev);
	size_t size = rate / frequency;

	if (size < 2)
		return 0; /* rate too low */

	/* The buffer size must be a multiple of 4 */
	while (size & 0x3)
		size <<= 1;

	/* The buffer size shouldn't be too small */
	while (size < SignalGenerator::min_buffer_size)
		size <<= 1;

	if (size > max_buffer_size)
		return 0;

	return size;
}

unsigned long NetworkAnalyzer::get_best_sin_sample_rate(
		const struct iio_channel *chn, double frequency)
{
	const struct iio_device *dev = iio_channel_get_device(chn);
	QVector<unsigned long> values =
		SignalGenerator::get_available_sample_rates(dev);

	/* Return the best sample rate that we can create a buffer for */
	for (unsigned long rate : values) {
		size_t buf_size = get_sin_samples_count(chn, rate, frequency);
		if (buf_size)
			return rate;

		qDebug() << QString("Rate %1 too high, trying lower")
			.arg(rate);
	}

	throw std::runtime_error("Unable to calculate best sample rate");
}

unsigned long NetworkAnalyzer::get_best_adc_rate(double frequency)
{
	QVector<unsigned long> values =
		SignalGenerator::get_available_sample_rates(adc);

	/* Return the smallest rate that is at least twice the frequency */
	qSort(values.begin(), values.end(), qLess<unsigned long>());

	for (unsigned long rate : values) {
		if ((double) rate > 2.0 * frequency)
			return rate;
	}

	/* Or return the fastest one */
	return values.takeLast();
}

struct iio_buffer * NetworkAnalyzer::generateSinWave(
		const struct iio_device *dev, double frequency,
		double amplitude, double offset,
		unsigned long rate, size_t samples_count)
{
	/* Create the IIO buffer */
	struct iio_buffer *buf = iio_device_create_buffer(
			dev, samples_count, true);
	if (!buf)
		throw std::runtime_error("Unable to create buffer");

	auto top_block = gr::make_top_block("Signal Generator");

	auto src = analog::sig_source_f::make(rate, analog::GR_SIN_WAVE,
			frequency, amplitude, offset);

	// DAC_RAW = (-Vout * 2^11) / 5V
	// Multiplying with 16 because the HDL considers the DAC data as 16 bit
	// instead of 12 bit(data is shifted to the left).
	auto f2s = blocks::float_to_short::make(1,
			-1 * (1 << (DAC_BIT_COUNT - 1)) /
			AMPLITUDE_VOLTS * 16 / INTERP_BY_100_CORR);

	auto head = blocks::head::make(
			sizeof(short), samples_count);

	auto vector = blocks::vector_sink_s::make();

	top_block->connect(src, 0, f2s, 0);
	top_block->connect(f2s, 0, head, 0);
	top_block->connect(head, 0, vector, 0);

	top_block->run();

	const std::vector<short> &samples = vector->data();
	const short *data = samples.data();

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (iio_channel_is_enabled(chn)) {
			iio_channel_write(chn, buf, data,
					samples_count * sizeof(short));
		}
	}

	iio_device_attr_write_longlong(dev, "sampling_frequency", rate);

	iio_buffer_push(buf);

	return buf;
}

double NetworkAnalyzer_API::getMinFreq() const
{
	return net->ui->minFreq->value();
}

double NetworkAnalyzer_API::getMaxFreq() const
{
	return net->ui->maxFreq->value();
}

double NetworkAnalyzer_API::getStepSize() const
{
	return net->ui->stepSize->value();
}

double NetworkAnalyzer_API::getAmplitude() const
{
	return net->ui->amplitude->value();
}

double NetworkAnalyzer_API::getOffset() const
{
	return net->ui->offset->value();
}

void NetworkAnalyzer_API::setMinFreq(double freq)
{
	net->ui->minFreq->setValue(freq);
	net->ui->dbgraph->setXMin(freq);
	net->ui->phasegraph->setXMin(freq);
}

void NetworkAnalyzer_API::setMaxFreq(double freq)
{
	net->ui->maxFreq->setValue(freq);
	net->ui->dbgraph->setXMax(freq);
	net->ui->phasegraph->setXMax(freq);
}

void NetworkAnalyzer_API::setStepSize(double step)
{
	net->ui->stepSize->setValue(step);
}

void NetworkAnalyzer_API::setAmplitude(double amp)
{
	net->ui->amplitude->setValue(amp);
}

void NetworkAnalyzer_API::setOffset(double offset)
{
	net->ui->offset->setValue(offset);
}

double NetworkAnalyzer_API::getMinMag() const
{
	return net->ui->magMin->value();
}

double NetworkAnalyzer_API::getMaxMag() const
{
	return net->ui->magMax->value();
}

double NetworkAnalyzer_API::getMinPhase() const
{
	return net->ui->phaseMin->value();
}

double NetworkAnalyzer_API::getMaxPhase() const
{
	return net->ui->phaseMax->value();
}

void NetworkAnalyzer_API::setMinMag(double val)
{
	net->ui->magMin->setValue(val);
	net->ui->dbgraph->setYMin(val);
	net->ui->xygraph->setMin(val);
	net->ui->nicholsgraph->setYMin(val);
}

void NetworkAnalyzer_API::setMaxMag(double val)
{
	net->ui->magMax->setValue(val);
	net->ui->dbgraph->setYMax(val);
	net->ui->xygraph->setMax(val);
	net->ui->nicholsgraph->setYMax(val);
}

void NetworkAnalyzer_API::setMinPhase(double val)
{
	net->ui->phaseMin->setValue(val);
	net->ui->phasegraph->setYMin(val);
	net->ui->nicholsgraph->setXMin(val);
}

void NetworkAnalyzer_API::setMaxPhase(double val)
{
	net->ui->phaseMax->setValue(val);
	net->ui->phasegraph->setYMax(val);
	net->ui->nicholsgraph->setXMax(val);
}
