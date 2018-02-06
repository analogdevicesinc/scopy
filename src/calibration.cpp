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

#include "calibration.hpp"
#include "osc_adc.h"
#include "hw_dac.h"

#include <errno.h>
#include <QDebug>
#include <QtGlobal>
#include <iio.h>
#include <QThread>

using namespace adiscope;

Calibration::Calibration(struct iio_context *ctx, QJSEngine *engine,
			 std::shared_ptr<M2kAdc> adc,
			 std::shared_ptr<M2kDac> dac_a,
			 std::shared_ptr<M2kDac> dac_b):
	m_api(new Calibration_API(this)),
	m_ctx(ctx),
	m_dac_a_buffer(NULL),
	m_dac_b_buffer(NULL),
	m_initialized(false),
	m2k_adc(adc),
	m2k_dac_a(dac_a),
	m2k_dac_b(dac_b),
	m_cancel(false)
{
	m_api->setObjectName("calib");
	m_api->js_register(engine);
}

Calibration::~Calibration()
{
	if (m_dac_a_buffer)
		iio_buffer_destroy(m_dac_a_buffer);
	if (m_dac_b_buffer)
		iio_buffer_destroy(m_dac_b_buffer);
	delete m_api;
}

bool Calibration::initialize()
{
	m_initialized = false;

	if (!m_ctx)
		return false;

	// IIO devices
	m_m2k_adc = iio_context_find_device(m_ctx, "m2k-adc");
	if (!m_m2k_adc)
		return false;
	m_m2k_dac_a = iio_context_find_device(m_ctx, "m2k-dac-a");
	if (!m_m2k_dac_a)
		return false;
	m_m2k_dac_b = iio_context_find_device(m_ctx, "m2k-dac-b");
	if (!m_m2k_dac_b)
		return false;
	m_m2k_fabric = iio_context_find_device(m_ctx, "m2k-fabric");
	if (!m_m2k_fabric)
		return false;
	m2k_ad5625 = iio_context_find_device(m_ctx, "ad5625");
	if (!m2k_ad5625)
		return false;

	// IIO channels
	m_adc_channel0 = iio_device_find_channel(m_m2k_adc, "voltage0", false);
	if (!m_adc_channel0)
		return false;
	m_adc_channel1 = iio_device_find_channel(m_m2k_adc, "voltage1", false);
	if (!m_adc_channel1)
		return false;
	m_dac_a_channel = iio_device_find_channel(m_m2k_dac_a, "voltage0", true);
	if (!m_dac_a_channel)
		return false;
	m_dac_b_channel = iio_device_find_channel(m_m2k_dac_b, "voltage0", true);
	if (!m_dac_b_channel)
		return false;
	m_ad5625_channel0 = iio_device_find_channel(m2k_ad5625, "voltage0", true);
	if (!m_ad5625_channel0)
		return false;
	m_ad5625_channel1 = iio_device_find_channel(m2k_ad5625, "voltage1", true);
	if (!m_ad5625_channel1)
		return false;
	m_ad5625_channel2 = iio_device_find_channel(m2k_ad5625, "voltage2", true);
	if (!m_ad5625_channel2)
		return false;
	m_ad5625_channel3 = iio_device_find_channel(m2k_ad5625, "voltage3", true);
	if (!m_ad5625_channel3)
		return false;
	m_dac_a_fabric = iio_device_find_channel(m_m2k_fabric, "voltage0", true);
	if (!m_dac_a_fabric)
		return false;
	m_dac_b_fabric = iio_device_find_channel(m_m2k_fabric, "voltage1", true);
	if (!m_dac_b_fabric)
		return false;

	m_adc_ch0_gain = 1;
	m_adc_ch1_gain = 1;
	m_dac_a_ch_vlsb = 0;
	m_dac_b_ch_vlsb = 0;

	m_initialized = true;

	return m_initialized;
}

bool Calibration::isInitialized() const
{
	return m_initialized;
}

void Calibration::setHardwareInCalibMode()
{
	// Make sure hardware triggers are disabled before calibrating
	struct iio_device *trigg_dev = iio_context_find_device(m_ctx,
		"m2k-adc-trigger");
	m_trigger0_mode.clear();
	m_trigger1_mode.clear();

	if (trigg_dev) {
		char buf[4096];

		struct iio_channel *trigger0Mode = iio_device_find_channel(
			trigg_dev, "voltage4", false);
		struct iio_channel *trigger1Mode = iio_device_find_channel(
			trigg_dev, "voltage5", false);

		if (trigger0Mode) {
			iio_channel_attr_read(trigger0Mode, "mode", buf,
				sizeof(buf));
			m_trigger0_mode.assign(buf);
			iio_channel_attr_write(trigger0Mode, "mode", "always");
		}
		if (trigger1Mode) {
			iio_channel_attr_read(trigger1Mode, "mode", buf,
				sizeof(buf));
			m_trigger1_mode.assign(buf);
			iio_channel_attr_write(trigger1Mode, "mode", "always");
		}
	}

	/* Save the previous values for sampling frequency and oversampling ratio */
	iio_device_attr_read_double(m_m2k_adc, "sampling_frequency",
		&adc_sampl_freq);
	iio_device_attr_read_double(m_m2k_adc, "oversampling_ratio",
		&adc_oversampl);
	iio_device_attr_read_double(m_m2k_dac_a, "sampling_frequency",
		&dac_a_sampl_freq);
	iio_device_attr_read_double(m_m2k_dac_a, "oversampling_ratio",
		&dac_a_oversampl);
	iio_device_attr_read_double(m_m2k_dac_b, "sampling_frequency",
		&dac_b_sampl_freq);
	iio_device_attr_read_double(m_m2k_dac_b, "oversampling_ratio",
		&dac_b_oversampl);
}

void Calibration::configHwSamplerate()
{
	// Make sure we calibrate at the highest sample rate
	m2k_adc->setSampleRate(1e8);
	iio_device_attr_write_longlong(m2k_adc->iio_adc_dev(), "oversampling_ratio", 1);
	m2k_dac_a->setSampleRate(75E6);
	iio_device_attr_write_longlong(m2k_dac_a->iio_dac_dev(), "oversampling_ratio", 1);
	m2k_dac_b->setSampleRate(75E6);
	iio_device_attr_write_longlong(m2k_dac_b->iio_dac_dev(), "oversampling_ratio", 1);
}

double Calibration::getDac_b_ch_vlsb() const
{
	return m_dac_b_ch_vlsb;
}

void Calibration::setDac_b_ch_vlsb(double dac_b_ch_vlsb)
{
	m_dac_b_ch_vlsb = dac_b_ch_vlsb;
}

double Calibration::getDac_a_ch_vlsb() const
{
	return m_dac_a_ch_vlsb;
}

void Calibration::setDac_a_ch_vlsb(double dac_a_ch_vlsb)
{
	m_dac_a_ch_vlsb = dac_a_ch_vlsb;
}

double Calibration::getAdc_ch1_gain() const
{
	return m_adc_ch1_gain;
}

void Calibration::setAdc_ch1_gain(double adc_ch1_gain)
{
	m_adc_ch1_gain = adc_ch1_gain;
}

double Calibration::getAdc_ch0_gain() const
{
	return m_adc_ch0_gain;
}

void Calibration::setAdc_ch0_gain(double adc_ch0_gain)
{
	m_adc_ch0_gain = adc_ch0_gain;
}

int Calibration::getDac_b_ch_offset() const
{
	return m_dac_b_ch_offset;
}

void Calibration::setDac_b_ch_offset(int dac_b_ch_offset)
{
	m_dac_b_ch_offset = dac_b_ch_offset;
}

int Calibration::getDac_a_ch_offset() const
{
	return m_dac_a_ch_offset;
}

void Calibration::setDac_a_ch_offset(int dac_a_ch_offset)
{
	m_dac_a_ch_offset = dac_a_ch_offset;
}

int Calibration::getAdc_ch1_offset() const
{
	return m_adc_ch1_offset;
}

void Calibration::setAdc_ch1_offset(int adc_ch1_offset)
{
	m_adc_ch1_offset = adc_ch1_offset;
}

int Calibration::getAdc_ch0_offset() const
{
	return m_adc_ch0_offset;
}

void Calibration::setAdc_ch0_offset(int adc_ch0_offset)
{
	m_adc_ch0_offset = adc_ch0_offset;
}

void Calibration::restoreHardwareFromCalibMode()
{
	struct iio_device *trigg_dev = iio_context_find_device(m_ctx,
							       "m2k-adc-trigger");
	struct iio_channel *trigger0Mode;
	struct iio_channel *trigger1Mode;

	if (trigg_dev) {
		trigger0Mode = iio_device_find_channel(trigg_dev, "voltage4",
							false);
		trigger1Mode = iio_device_find_channel(trigg_dev, "voltage5",
							false);

		if (trigger0Mode && !m_trigger0_mode.empty()) {
			iio_channel_attr_write(trigger0Mode, "mode",
						m_trigger0_mode.c_str());
		}
		if (trigger1Mode) {
			iio_channel_attr_write(trigger1Mode, "mode",
						m_trigger1_mode.c_str());
		}
	}

	/* Restore the previous values for sampling frequency and oversampling ratio */
	iio_device_attr_write_double(m_m2k_adc, "sampling_frequency",
		adc_sampl_freq);
	iio_device_attr_write_double(m_m2k_adc, "oversampling_ratio",
		adc_oversampl);
	iio_device_attr_write_double(m_m2k_dac_a, "sampling_frequency",
		dac_a_sampl_freq);
	iio_device_attr_write_double(m_m2k_dac_a, "oversampling_ratio",
		dac_a_oversampl);
	iio_device_attr_write_double(m_m2k_dac_b, "sampling_frequency",
		dac_b_sampl_freq);
	iio_device_attr_write_double(m_m2k_dac_b, "oversampling_ratio",
		dac_b_oversampl);
}

bool Calibration::calibrateADCoffset()
{
	bool calibrated = false;

	if (!m_initialized) {
		qDebug() << "Rx path is not initialized for calibration.";
		return false;
	}

	qDebug() << "Starting ADC OFFSET CALIBRATION";

	// Ground ADC inputs
	setCalibrationMode(ADC_GND);

	// Set DAC channels to middle scale
	iio_channel_attr_write_longlong(m_ad5625_channel2, "raw", 2048);
	iio_channel_attr_write_longlong(m_ad5625_channel3, "raw", 2048);

	// Allow some time for the voltage to settle
	QThread::msleep(50);

	const unsigned int num_samples = 1e5;
	int16_t dataCh0[num_samples];
	int16_t dataCh1[num_samples];

	bool ret = adc_data_capture(dataCh0, dataCh1, num_samples);
		if (!ret) {
		qDebug() << "failed to get samples";
		return false;
	}

	int16_t ch0_avg = average(dataCh0, num_samples);
	int16_t ch1_avg = average(dataCh1, num_samples);

	// Convert from raw format to signed raw
	int16_t tmp;

	tmp = ch0_avg;
	iio_channel_convert(m_adc_channel0, (void *)&ch0_avg,
		(const void *)&tmp);
	tmp = ch1_avg;
	iio_channel_convert(m_adc_channel1, (void *)&ch1_avg,
		(const void *)&tmp);

	double voltage0 = convSampleToVolts(ch0_avg);
	double voltage1 = convSampleToVolts(ch1_avg);

	double gain = 1.3;
	double range = 3.192;

	m_adc_ch0_offset = (int)(2048 - ((voltage0 * 4096 * gain) / range));
	m_adc_ch1_offset = (int)(2048 - ((voltage1 * 4096 * gain) / range));

	qDebug() << "Before Fine-Tunning";
	qDebug() << "ADC channel 0 offset(raw):" << m_adc_ch0_offset;
	qDebug() << "ADC channel 1 offset(raw):" << m_adc_ch1_offset;

	fine_tune(20, m_adc_ch0_offset, m_adc_ch1_offset, num_samples);

	calibrated = true;

	return calibrated;
}

bool Calibration::calibrateADCgain()
{
	bool calibrated = false;

	if (!m_initialized) {
	qDebug() << "Rx path is not initialized for calibration.";
	return false;
	}

	qDebug() << "Starting ADC GAIN CALIBRATION";

	setCalibrationMode(ADC_REF1);

	double vref1 = 0.4615;
	const unsigned int num_samples = 1e5;
	int16_t dataCh0[num_samples];
	int16_t dataCh1[num_samples];
	double avg0, avg1;
	bool ret;

	ret = adc_data_capture(dataCh0, dataCh1, num_samples);
	if (!ret) {
		qDebug() << "failed to get samples";
		return false;
	}

	avg0 = average(dataCh0, num_samples);
	avg1 = average(dataCh1, num_samples);

	// Convert from raw format to signed raw
	int16_t tmp;

	tmp = avg0;
	iio_channel_convert(m_adc_channel0, (void *)&avg0, (const void *)&tmp);
	tmp = avg1;
	iio_channel_convert(m_adc_channel1, (void *)&avg0, (const void *)&tmp);

	avg0 = convSampleToVolts(avg0);
	avg1 = convSampleToVolts(avg1);

	m_adc_ch0_gain = vref1 / avg0;
	m_adc_ch1_gain = vref1 / avg1;

	qDebug() << "Gain for channel0: " << m_adc_ch0_gain;
	qDebug() << "Gain for channel1: " << m_adc_ch1_gain;

	setCalibrationMode(NONE);

	calibrated = true;

	return calibrated;
}

int Calibration::adcOffsetChannel0() const
{
	return m_adc_ch0_offset;
}

int Calibration::adcOffsetChannel1() const
{
	return m_adc_ch1_offset;
}

double Calibration::adcGainChannel0() const
{
	return m_adc_ch0_gain;
}

double Calibration::adcGainChannel1() const
{
	return m_adc_ch1_gain;
}

void Calibration::updateCorrections()
{
	iio_channel_attr_write_double(m_ad5625_channel2, "raw",
				      m_adc_ch0_offset);
	iio_channel_attr_write_double(m_ad5625_channel3, "raw",
				      m_adc_ch1_offset);

	iio_channel_attr_write_double(m_ad5625_channel0, "raw",
				      m_dac_a_ch_offset);
	iio_channel_attr_write_double(m_ad5625_channel1, "raw",
				      m_dac_b_ch_offset);

	if(m2k_adc) {
		m2k_adc->setChnCorrectionOffset(0, adcOffsetChannel0());
		m2k_adc->setChnCorrectionOffset(1, adcOffsetChannel1());
		m2k_adc->setChnCorrectionGain(0, adcGainChannel0());
		m2k_adc->setChnCorrectionGain(1, adcGainChannel1());
	}

	if(m2k_dac_a)
		m2k_dac_a->setVlsb(dacAvlsb());
	if(m2k_dac_b)
		m2k_dac_b->setVlsb(dacBvlsb());
}

bool Calibration::resetCalibration()
{
	if (!m_initialized) {
		qDebug() << "Rx path is not initialized for calibration.";
		return false;
	}

	setCalibrationMode(NONE);

	m_adc_ch0_offset = 2048;
	m_adc_ch1_offset = 2048;

	m_dac_a_ch_offset =  2048;
	m_dac_b_ch_offset =  2048;

	m_adc_ch0_gain = 1;
	m_adc_ch1_gain = 1;

	m_dac_a_ch_vlsb = 0.0034;
	m_dac_b_ch_vlsb = 0.0034;

	updateCorrections();
	return true;
}

void Calibration::setChannelEnableState(struct iio_channel *chn, bool en)
{
	if (en)
		iio_channel_enable(chn);
	else
		iio_channel_disable(chn);
}

double Calibration::average(int16_t *data, size_t numElements)
{
	double sum = 0;

	for (size_t i = 0; i < numElements; i++)
		sum += data[i];

	return (sum / (double)numElements);
}

bool Calibration::adc_data_capture(int16_t *dataCh0, int16_t *dataCh1,
	size_t num_sampl_per_chn)
{
	if (!dataCh0 && !dataCh1) {
		qDebug() << "At least one channels needs to be activated."
			" Aborting calibration.";
		return false;
	}

	// Store channels enable state
	bool channel0Enabled = iio_channel_is_enabled(m_adc_channel0);
	bool channel1Enabled = iio_channel_is_enabled(m_adc_channel1);

	// Enable the required channels
	setChannelEnableState(m_adc_channel0, !!dataCh0);
	setChannelEnableState(m_adc_channel1, !!dataCh1);

	struct iio_buffer *buffer = iio_device_create_buffer(m_m2k_adc,
		num_sampl_per_chn, false);

	if (!buffer) {
		qDebug() << "Could not create m2k-adc buffer!" <<
			strerror(errno) << "Aborting calibration.";
		setChannelEnableState(m_adc_channel0, channel0Enabled);
		setChannelEnableState(m_adc_channel1, channel1Enabled);
		return false;
	}

	int ret = iio_buffer_refill(buffer);

	if (ret < 0) {
		qDebug() << "Could not refill m2k-adc buffer! Error:" << ret <<
			"Aborting calibration";
		iio_buffer_destroy(buffer);
		setChannelEnableState(m_adc_channel0, channel0Enabled);
		setChannelEnableState(m_adc_channel1, channel1Enabled);
		return false;
	}

	ptrdiff_t p_inc = iio_buffer_step(buffer);
	uintptr_t p_dat;
	uintptr_t p_end = (uintptr_t)iio_buffer_end(buffer);
	unsigned int i;
	for (i = 0, p_dat = (uintptr_t)iio_buffer_first(buffer, m_adc_channel0);
			p_dat < p_end; p_dat += p_inc, i++)
	{
		if (dataCh0 && dataCh1) {
			dataCh0[i] = ((int16_t*)p_dat)[0];
			dataCh1[i] = ((int16_t*)p_dat)[1];

		} else if (dataCh0) {
			dataCh0[i] = ((int16_t*)p_dat)[0];
		} else if (dataCh1) {
			dataCh1[i] = ((int16_t*)p_dat)[0];
		}
	}

	iio_buffer_destroy(buffer);

	// Restore channels enable states
	setChannelEnableState(m_adc_channel0, channel0Enabled);
	setChannelEnableState(m_adc_channel1, channel1Enabled);

	return true;
}

bool Calibration::fine_tune(size_t span, int16_t centerVal0, int16_t centerVal1,
	size_t num_samples)
{
	int16_t *candidateOffsets0 = new int16_t[span + 1];
	int16_t *candidateOffsets1 = new int16_t[span + 1];
	double *averagesCh0 = new double[span + 1];
	double *averagesCh1 = new double[span + 1];
	double minAvg0, minAvg1;
	int16_t *dataCh0 = new int16_t[num_samples];
	int16_t *dataCh1 = new int16_t[num_samples];
	int16_t offset0, offset1;
	int i, i0 = 0, i1 = 0;
	bool ret = true;

	offset0 = centerVal0 - span / 2;
	offset1 = centerVal1 - span / 2;
	for (i = 0; i < span + 1; i++) {
		candidateOffsets0[i] = offset0;
		candidateOffsets1[i] = offset1;
		iio_channel_attr_write_double(m_ad5625_channel2, "raw",
			offset0);
		iio_channel_attr_write_double(m_ad5625_channel3, "raw",
			offset1);
		offset0++;
		offset1++;

		// Allow some time for the voltage to settle
		QThread::msleep(5);

		ret = adc_data_capture(dataCh0, dataCh1, num_samples);

		if (!ret) {
			qDebug() << "failed to get samples";
			goto out_cleanup;
		}

		averagesCh0[i] = qAbs(average(dataCh0, num_samples));
		averagesCh1[i] = qAbs(average(dataCh1, num_samples));
	}

	minAvg0 = qAbs(averagesCh0[0]);
	minAvg1 = qAbs(averagesCh1[0]);

	for (int i = 1; i < span + 1; i++) {
		if (averagesCh0[i] < minAvg0) {
			minAvg0 = averagesCh0[i];
			i0 = i;
		}
		if (averagesCh1[i] < minAvg1) {
			minAvg1 = averagesCh1[i];
			i1 = i;
		}
	}

	setCalibrationMode(NONE);

	m_adc_ch0_offset = candidateOffsets0[i0];
	m_adc_ch1_offset = candidateOffsets1[i1];

	qDebug() << "After Fine-Tunning";
	qDebug() << "ADC channel 0 offset(raw):" << m_adc_ch0_offset;
	qDebug() << "ADC channel 1 offset(raw):" << m_adc_ch1_offset;

	iio_channel_attr_write_longlong(m_ad5625_channel2, "raw",
		m_adc_ch0_offset);
	iio_channel_attr_write_longlong(m_ad5625_channel3, "raw",
		m_adc_ch1_offset);

out_cleanup:
	delete[] candidateOffsets0;
	delete[] candidateOffsets1;
	delete[] averagesCh0;
	delete[] averagesCh1;
	delete[] dataCh0;
	delete[] dataCh1;
	return ret;
}

int Calibration::dacAoffset() const
{
	return m_dac_a_ch_offset;
}

int Calibration::dacBoffset() const
{
	return m_dac_b_ch_offset;
}

double Calibration::dacAvlsb() const
{
	return m_dac_a_ch_vlsb;
}
double Calibration::dacBvlsb() const
{
	return m_dac_b_ch_vlsb;
}

bool Calibration::calibrateDACoffset()
{
	bool calibrated = false;

	if (!m_initialized) {
		qDebug() << "Tx path is not initialized for calibration.";
		return false;
	}

	qDebug() << "Starting DAC OFFSET CALIBRATION";

	// connect ADC to DAC
	setCalibrationMode(DAC);

	// Set DAC offset channels to middle scale
	iio_channel_attr_write_longlong(m_ad5625_channel0, "raw", 2048);
	iio_channel_attr_write_longlong(m_ad5625_channel1, "raw", 2048);

	// write to DAC
	dacAOutputDC(0);
	dacBOutputDC(0);

	// Allow some time for the voltage to settle
	QThread::msleep(50);

	const unsigned int num_samples = 1e5;
	int16_t dataCh0[num_samples];
	int16_t dataCh1[num_samples];

	bool ret = adc_data_capture(dataCh0, dataCh1, num_samples);
		if (!ret) {
		qDebug() << "failed to get samples";
		return false;
	}

	int16_t ch0_avg = average(dataCh0, num_samples);
	int16_t ch1_avg = average(dataCh1, num_samples);

	// Convert from raw format to signed raw
	int16_t tmp;

	tmp = ch0_avg;
	iio_channel_convert(m_adc_channel0, (void *)&ch0_avg,
		(const void *)&tmp);
	tmp = ch1_avg;
	iio_channel_convert(m_adc_channel1, (void *)&ch1_avg,
		(const void *)&tmp);

	double voltage0 = convSampleToVolts(ch0_avg, m_adc_ch0_gain);
	double voltage1 = convSampleToVolts(ch1_avg, m_adc_ch1_gain);

	m_dac_a_ch_offset = (int)(2048 - ((voltage0 * 9.06 ) / 0.002658));
	m_dac_b_ch_offset = (int)(2048 - ((voltage1 * 9.06 ) / 0.002658));

	iio_channel_attr_write_longlong(m_ad5625_channel0, "raw",
		m_dac_a_ch_offset);
	iio_channel_attr_write_longlong(m_ad5625_channel1, "raw",
		m_dac_b_ch_offset);

	qDebug() << "DAC calib offset results:";
	qDebug() << "DAC channel 0 offset(raw):" << m_dac_a_ch_offset;
	qDebug() << "DAC channel 1 offset(raw):" << m_dac_b_ch_offset;

	if (m_dac_a_buffer) {
		iio_buffer_destroy(m_dac_a_buffer);
		m_dac_a_buffer = NULL;
	}

	if (m_dac_b_buffer) {
		iio_buffer_destroy(m_dac_b_buffer);
		m_dac_b_buffer = NULL;
	}

	setChannelEnableState(m_dac_a_channel, false);
	setChannelEnableState(m_dac_b_channel, false);

	setCalibrationMode(NONE);

	calibrated = true;

	return calibrated;
}

bool Calibration::calibrateDACgain()
{
	bool calibrated = false;

	// connect ADC to DAC
	setCalibrationMode(DAC);

	// Use the positive half scale point for gain calibration
	dacAOutputDC(1024);
	dacBOutputDC(1024);

	// Allow some time for the voltage to settle
	QThread::msleep(50);

	const unsigned int num_samples = 1e5;
	int16_t dataCh0[num_samples];
	int16_t dataCh1[num_samples];

	bool ret = adc_data_capture(dataCh0, dataCh1, num_samples);
		if (!ret) {
		qDebug() << "failed to get samples";
		return false;
	}

	int16_t ch0_avg = average(dataCh0, num_samples);
	int16_t ch1_avg = average(dataCh1, num_samples);

	// Convert from raw format to signed raw
	int16_t tmp;

	tmp = ch0_avg;
	iio_channel_convert(m_adc_channel0, (void *)&ch0_avg,
		(const void *)&tmp);
	tmp = ch1_avg;
	iio_channel_convert(m_adc_channel1, (void *)&ch1_avg,
		(const void *)&tmp);

	double voltage0 = convSampleToVolts(ch0_avg, m_adc_ch0_gain);
	double voltage1 = convSampleToVolts(ch1_avg, m_adc_ch1_gain);

	// Taking into account the voltage divider on the loopback path
	voltage0 *= 9.06;
	voltage1 *= 9.06;

	m_dac_a_ch_vlsb = voltage0 / 1024;
	m_dac_b_ch_vlsb = voltage1 / 1024;

	if (m_dac_a_buffer) {
		iio_buffer_destroy(m_dac_a_buffer);
		m_dac_a_buffer = NULL;
	}

	if (m_dac_b_buffer) {
		iio_buffer_destroy(m_dac_b_buffer);
		m_dac_b_buffer = NULL;
	}

	setChannelEnableState(m_dac_a_channel, false);
	setChannelEnableState(m_dac_b_channel, false);

	setCalibrationMode(NONE);

	calibrated = true;

	return calibrated;
}

void Calibration::dacOutputDC(struct iio_device *dac,
	struct iio_channel *channel, struct iio_buffer** buffer, size_t value)
{
	const size_t size = 256;
	int16_t data[size];

	value = (-value) << 4; // This should go away once channel type gets
	// changed from 'le:S16/16>>0' to 'le:S12/16>>4'
	setChannelEnableState(channel, true);

	if (*buffer)
		iio_buffer_destroy(*buffer);

	*buffer = iio_device_create_buffer(dac,
			size, true);
	if (!(*buffer)) {
		qDebug() << "Could not create buffer for: "  <<
			 iio_device_get_name(dac);
		return;
	}

	std::fill_n(data, size, value);
	iio_channel_write(channel, *buffer, data, size * sizeof(data[0]));

	iio_buffer_push(*buffer);
}

void Calibration::dacAOutputDCVolts(int16_t dac_a_volts)
{
	int dac_a_raw;
	setCalibrationMode(NONE);
	setChannelEnableState(m_dac_a_channel, true);
	iio_device_attr_write_bool(m_m2k_dac_a, "dma_sync", true);
	dac_a_raw = (dac_a_volts / m_dac_a_ch_vlsb);
	dacAOutputDC(dac_a_raw);

	iio_device_attr_write_bool(m_m2k_dac_a, "dma_sync", false);

	iio_channel_attr_write_bool(m_dac_a_fabric, "powerdown", false);
}

void Calibration::dacBOutputDCVolts(int16_t dac_b_volts)
{
	int dac_b_raw;
	setCalibrationMode(NONE);
	setChannelEnableState(m_dac_b_channel, true);
	iio_device_attr_write_bool(m_m2k_dac_b, "dma_sync", true);
	dac_b_raw = (dac_b_volts / m_dac_b_ch_vlsb);
	dacBOutputDC(dac_b_raw);

	iio_device_attr_write_bool(m_m2k_dac_b, "dma_sync", false);

	iio_channel_attr_write_bool(m_dac_b_fabric, "powerdown", false);
}

void Calibration::dacAOutputDC(int16_t value)
{
	dacOutputDC(m_m2k_dac_a, m_dac_a_channel, &m_dac_a_buffer, value);
}

void Calibration::dacOutputStop()
{
	if (m_dac_a_buffer) {
		iio_buffer_cancel(m_dac_a_buffer);
		iio_buffer_destroy(m_dac_a_buffer);
		m_dac_a_buffer = NULL;
	}

	if (m_dac_b_buffer) {
		iio_buffer_cancel(m_dac_b_buffer);
		iio_buffer_destroy(m_dac_b_buffer);
		m_dac_b_buffer = NULL;
	}
	setChannelEnableState(m_dac_a_channel, false);
	setChannelEnableState(m_dac_b_channel, false);

	/* FIXME: TODO: Move this into a HW class / lib M2k */
	iio_channel_attr_write_bool(m_dac_a_fabric, "powerdown", true);
	iio_channel_attr_write_bool(m_dac_b_fabric, "powerdown", true);

	setCalibrationMode(NONE);
}

void Calibration::dacBOutputDC(int16_t value)
{
	dacOutputDC(m_m2k_dac_b, m_dac_b_channel, &m_dac_b_buffer, value);
}

float Calibration::convSampleToVolts(float sample, float correctionGain)
{
	// TO DO: explain this formula and add methods to change gain and offset
	return ((sample * 0.78) / ((1 << 11) * 1.3) * correctionGain);
}

float Calibration::convVoltsToSample(float voltage, float correctionGain)
{
	// TO DO: explain this formula and add methods to change gain and offset
	return (voltage / correctionGain * (2048 * 1.3) / 0.78);
}

bool Calibration::calibrateAll()
{
	bool ok;
	configHwSamplerate();

	ok = calibrateADCoffset();

	if (!ok || m_cancel) {
		goto calibration_fail;
	}

	ok = calibrateADCgain();

	if (!ok || m_cancel) {
		goto calibration_fail;
	}

	if (IioUtils::hardware_revision(m_ctx) != "A") {
		QThread::msleep(750);
	}

	ok = calibrateDACoffset();

	if (!ok || m_cancel) {
		goto calibration_fail;
	}

	ok = calibrateDACgain();

	if (!ok  || m_cancel) {
		goto calibration_fail;
	}

	updateCorrections();
	return true;

calibration_fail:
	m_cancel=false;
	return false;
}

void Calibration::cancelCalibration()
{
	m_cancel=true;
}

bool Calibration::setGainMode(int ch, int mode)
{
        switch (mode) {
        case HIGH:
                m2k_adc->setChnHwGainMode(ch, M2kAdc::HIGH_GAIN_MODE);
                break;
        case LOW:
                m2k_adc->setChnHwGainMode(ch, M2kAdc::LOW_GAIN_MODE);
                break;
        default:
                return false;
        }
        return true;
}

bool Calibration::setCalibrationMode(int mode)
{
        std::string strMode;
        switch (mode) {
        case ADC_GND:
                strMode = "adc_gnd";
                break;
        case ADC_REF1:
                strMode = "adc_ref1";
                break;
        case ADC_REF2:
                strMode = "adc_ref2";
                break;
        case DAC:
                strMode = "dac";
                break;
        case NONE:
                strMode = "none";
                break;
        default:
                return false;
        }
        iio_device_attr_write(m_m2k_fabric, "calibration_mode", strMode.c_str());
        m_calibration_mode = mode;
	return true;
}

/* FIXME: TODO: Move this into a HW class / lib M2k */
double Calibration::getIioDevTemp(const QString& devName) const
{
	double temp = -273.15;

	struct iio_device *dev = iio_context_find_device(m_ctx,
		devName.toLatin1().data());

	if (dev) {
		struct iio_channel *chn = iio_device_find_channel(dev, "temp0",
			false);
		if (chn) {
			double offset;
			double raw;
			double scale;

			iio_channel_attr_read_double(chn, "offset", &offset);
			iio_channel_attr_read_double(chn, "raw", &raw);
			iio_channel_attr_read_double(chn, "scale", &scale);

			temp = (raw + offset) * scale / 1000;
		}
	}

	return temp;
}

/*
 * class Calibration_API
 */

Calibration_API::Calibration_API(Calibration *calib) : ApiObject(),
	calib(calib)
{
}

QList<double> Calibration_API::get_adc_offsets() const
{
	QList<double> offsets;

	offsets.push_back(static_cast<double>(calib->adcOffsetChannel0()));
	offsets.push_back(static_cast<double>(calib->adcOffsetChannel1()));

	return offsets;
}

QList<double> Calibration_API::get_adc_gains() const
{
	QList<double> gains = { calib->adcGainChannel0(),
		calib->adcGainChannel1() };

	return gains;
}

QList<double> Calibration_API::get_dac_offsets() const
{
	QList<double> offsets;

	offsets.push_back(static_cast<double>(calib->dacAoffset()));
	offsets.push_back(static_cast<double>(calib->dacBoffset()));

	return offsets;
}

QList<double> Calibration_API::get_dac_gains() const
{
	QList<double> gains = { calib->dacAvlsb(),
		calib->dacBvlsb() };

	return gains;
}

bool Calibration_API::calibrateAll()
{
	return calib->calibrateAll();
}

bool Calibration_API::resetCalibration()
{
	return calib->resetCalibration();
}

bool Calibration_API::setGainMode(int ch, int mode)
{
	return calib->setGainMode(ch, mode);
}

bool Calibration_API::setCalibrationMode(int mode)
{
	return calib->setCalibrationMode(mode);
}

void Calibration_API::setHardwareInCalibMode()
{
	calib->setHardwareInCalibMode();
}

void Calibration_API::dacAOutputDCVolts(int value)
{
	calib->dacAOutputDCVolts(value);
}

void Calibration_API::dacBOutputDCVolts(int value)
{
	calib->dacBOutputDCVolts(value);
}

void Calibration_API::dacOutputStop()
{
	calib->dacOutputStop();
}

void Calibration_API::restoreHardwareFromCalibMode()
{
	calib->restoreHardwareFromCalibMode();
}

double Calibration_API::devTemp(const QString& devName)
{
	return calib->getIioDevTemp(devName);
}
