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
#include "osc_adc.h"
#include "hardware_trigger.hpp"
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
		std::shared_ptr<GenericAdc> adc_dev,
		QPushButton *runButton, QJSEngine *engine,
		ToolLauncher *parent) :
	Tool(ctx, runButton, new NetworkAnalyzer_API(this), "Network Analyzer", parent),
	ui(new Ui::NetworkAnalyzer),
	adc_dev(adc_dev)
{
	iio = iio_manager::get_instance(ctx,
			filt->device_name(TOOL_NETWORK_ANALYZER, 2));

	adc = filt->find_device(ctx, TOOL_NETWORK_ANALYZER, 2);
	dac1 = filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 0, true);
	dac2 = filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 1, true);
	if (!dac1 || !dac2)
		throw std::runtime_error("Unable to find channels in filter file");

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

	ui->maxFreq->setMaxValue((double) max_samplerate / 2.5 - 1.0);

	connect(ui->minFreq, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));
	connect(ui->maxFreq, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));
	connect(ui->samplesCount, SIGNAL(valueChanged(double)),
			this, SLOT(updateNumSamples()));

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_NETWORK_ANALYZER)));

	ui->xygraph->enableZooming(ui->btnZoomIn, ui->btnZoomOut);

	api->load(*settings);
	api->js_register(engine);
}

NetworkAnalyzer::~NetworkAnalyzer()
{
	ui->run_button->setChecked(false);
	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete ui;
}

void NetworkAnalyzer::updateNumSamples()
{
	if (!ui->run_button->isChecked())
		return;

	unsigned int num_samples = (unsigned int) ui->samplesCount->value();

	ui->dbgraph->setNumSamples(num_samples);
	ui->phasegraph->setNumSamples(num_samples);
	ui->xygraph->setNumSamples(num_samples);
	ui->nicholsgraph->setNumSamples(num_samples);
}

void NetworkAnalyzer::run()
{
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

	// Adjust the gain of the ADC channels based on sweep settings
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
	if (m2k_adc) {
		double sweep_ampl = ui->amplitude->value();
		QPair<double, double> range = m2k_adc->inputRange(
				M2kAdc::HIGH_GAIN_MODE);
		double threshold = range.second - range.first;
		M2kAdc::GainMode gain_mode;
		if (sweep_ampl > threshold) {
			gain_mode = M2kAdc::LOW_GAIN_MODE;
		} else {
			gain_mode = M2kAdc::HIGH_GAIN_MODE;
		}
		for (int chn = 0; chn < m2k_adc->numAdcChannels(); chn++) {
			m2k_adc->setChnHwGainMode(chn, gain_mode);
		}
	}

	unsigned int steps = (unsigned int) ui->samplesCount->value();
	double min_freq = ui->minFreq->value();
	double max_freq = ui->maxFreq->value();
	double log10_min_freq = log10(min_freq);
	double log10_max_freq = log10(max_freq);
	double step;

	bool is_log = ui->isLog->isChecked();
	if (is_log)
		step = (log10_max_freq - log10_min_freq) / (double)(steps - 1);
	else
		step = (max_freq - min_freq) / (double)(steps - 1);

	for (unsigned int i = 0; !stop && i < steps; i++) {
		double frequency;

		if (is_log) {
			frequency = pow(10.0,
					log10_min_freq + (double) i * step);
		} else {
			frequency = min_freq + (double) i * step;
		}

		unsigned long rate = get_best_sample_rate(dev1, frequency);
		size_t samples_count = get_sin_samples_count(
				dev1, rate, frequency);
		unsigned long adc_rate;

		double amplitude = ui->amplitude->value();
		double offset = ui->offset->value();

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

		adc_rate = get_best_sample_rate(adc, frequency);
		iio_device_attr_write_longlong(adc,
				"sampling_frequency", adc_rate);

		/* Lock the flowgraph if we are already started */
		bool started = iio->started();
		if (started)
			iio->lock();

		size_t buffer_size = get_sin_samples_count(
				adc, adc_rate, frequency);

		auto f2c1 = blocks::float_to_complex::make();
		auto f2c2 = blocks::float_to_complex::make();
		auto id1 = iio->connect(f2c1, 0, 0, true,
				buffer_size);
		auto id2 = iio->connect(f2c2, 1, 0, true,
				buffer_size);

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

		auto avg1 = blocks::moving_average_cc::make(buffer_size,
				2.0 / buffer_size, buffer_size);
		auto skiphead3 = blocks::skiphead::make(sizeof(gr_complex),
				buffer_size - 1);
		auto c2m1 = blocks::complex_to_mag_squared::make();

		iio->connect(mult1, 0, avg1, 0);
		iio->connect(avg1, 0, skiphead3, 0);
		iio->connect(skiphead3, 0, c2m1, 0);
		iio->connect(skiphead3, 0, conj, 0);
		iio->connect(c2m1, 0, signal, 0);

		auto avg2 = blocks::moving_average_cc::make(buffer_size,
				2.0 / buffer_size, buffer_size);
		auto skiphead4 = blocks::skiphead::make(sizeof(gr_complex),
				buffer_size - 1);
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
			QCoreApplication::processEvents();
			QThread::msleep(10);

			if (!ui->run_button->isChecked())
				break;
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

		if (!got_it) /* Process was cancelled */
			return;

		double mag;
		if (ui->refCh1->isChecked()) {
			phase = -phase;
			mag = 10.0 * log10(mag2) - 10.0 * log10(mag1);
		} else {
			mag = 10.0 * log10(mag1) - 10.0 * log10(mag2);
		}

		qDebug() << "Frequency" << frequency << "Hz," <<
			adc_rate << "SPS," << buffer_size << "samples," <<
			mag << "Mag," << phase << "Deg";

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

	if (amp1 && amp2) {
		/* FIXME: TODO: Move this into a HW class / lib M2k */
		iio_channel_attr_write_bool(amp1, "powerdown", !pressed);
		iio_channel_attr_write_bool(amp2, "powerdown", !pressed);
	}

	if (pressed) {
		ui->dbgraph->reset();
		ui->phasegraph->reset();
		ui->xygraph->reset();
		ui->nicholsgraph->reset();
		updateNumSamples();
		configHwForNetworkAnalyzing();
		thd = QtConcurrent::run(this, &NetworkAnalyzer::run);
	} else {
		thd.waitForFinished();
	}

	setDynamicProperty(ui->run_button, "running", pressed);
}

size_t NetworkAnalyzer::get_sin_samples_count(const struct iio_device *dev,
		unsigned long rate, double frequency)
{
	size_t max_buffer_size = 4 * 1024 * 1024 /
		(size_t) iio_device_get_sample_size(dev);
	double ratio = (double) rate / frequency;
	size_t size;

	if (ratio < 2.5)
		return 0; /* rate too low */

	if (rate <= 10000)
		max_buffer_size = rate / 2; /* 500ms */

	size = (size_t) SignalGenerator::get_best_ratio(ratio,
			(double) (max_buffer_size / 4), nullptr);

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

unsigned long NetworkAnalyzer::get_best_sample_rate(
		const struct iio_device *dev, double frequency)
{
	QVector<unsigned long> values =
		SignalGenerator::get_available_sample_rates(dev);

	/* Return the best sample rate that we can create a buffer for */
	for (unsigned long rate : values) {
		size_t buf_size = get_sin_samples_count(dev, rate, frequency);
		if (buf_size)
			return rate;

		qDebug() << QString("Rate %1 too high, trying lower")
			.arg(rate);
	}

	throw std::runtime_error("Unable to calculate best sample rate");
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
			frequency, amplitude / 2.0, offset);

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

void NetworkAnalyzer::configHwForNetworkAnalyzing()
{
	auto trigger = adc_dev->getTrigger();
	if (trigger) {
		for (uint i = 0; i < trigger->numChannels(); i++)
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
	}

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
	if (m2k_adc) {
		iio_device_attr_write_longlong(m2k_adc->iio_adc_dev(),
			"oversampling_ratio", 1);
	}
}

double NetworkAnalyzer_API::getMinFreq() const
{
	return net->ui->minFreq->value();
}

double NetworkAnalyzer_API::getMaxFreq() const
{
	return net->ui->maxFreq->value();
}

double NetworkAnalyzer_API::getSamplesCount() const
{
	return net->ui->samplesCount->value();
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

void NetworkAnalyzer_API::setSamplesCount(double step)
{
	net->ui->samplesCount->setValue(step);
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

bool NetworkAnalyzer_API::isLogFreq() const
{
	return net->ui->isLog->isChecked();
}

void NetworkAnalyzer_API::setLogFreq(bool is_log)
{
	if (is_log)
		net->ui->isLog->setChecked(true);
	else
		net->ui->isLinear->setChecked(true);
}

int NetworkAnalyzer_API::getRefChannel() const
{
	if (net->ui->refCh1->isChecked())
		return 1;
	else
		return 2;
}

void NetworkAnalyzer_API::setRefChannel(int chn)
{
	if (chn == 1)
		net->ui->refCh1->setChecked(true);
	else
		net->ui->refCh2->setChecked(true);
}
