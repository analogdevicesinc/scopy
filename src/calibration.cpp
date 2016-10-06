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
#include "adc_sample_conv.hpp"

#include <QDebug>
#include <QtGlobal>
#include <iio.h>
#include<QThread>

using namespace adiscope;

RxCalibration::RxCalibration(struct iio_context *ctx):
	m_ctx(ctx),
	m_initialized(false)
{
}

RxCalibration::~RxCalibration()
{
}

bool RxCalibration::initialize()
{
	m_initialized = false;

	if (!m_ctx)
		return false;

	m_m2k_adc = iio_context_find_device(m_ctx, "m2k-adc");
	if (!m_m2k_adc)
		return false;

	m_m2k_fabric = iio_context_find_device(m_ctx, "m2k-fabric");
	if (!m_m2k_fabric)
		return false;

	m2k_ad5625 = iio_context_find_device(m_ctx, "ad5625");
	if (!m2k_ad5625)
		return false;

	m_adc_channel0 = iio_device_find_channel(m_m2k_adc, "voltage0", false);
	if (!m_adc_channel0)
		return false;

	m_adc_channel1 = iio_device_find_channel(m_m2k_adc, "voltage1", false);
	if (!m_adc_channel1)
		return false;

	m_ad5625_channel2 = iio_device_find_channel(m2k_ad5625, "voltage2", true);
	if (!m_ad5625_channel2)
		return false;

	m_ad5625_channel3 = iio_device_find_channel(m2k_ad5625, "voltage3", true);
	if (!m_ad5625_channel3)
		return false;

	m_adc_ch0_gain = 1;
	m_adc_ch1_gain = 1;

	// Make sure hardware triggers are disabled before calibrating
	struct iio_device *trigg_dev = iio_context_find_device(m_ctx, "m2k-adc-trigger");
	m_trigger0_mode.clear();
	m_trigger1_mode.clear();

	if (trigg_dev) {
		char buf[4096];

		struct iio_channel *trigger0Mode = iio_device_find_channel(trigg_dev, "voltage4", false);
		struct iio_channel *trigger1Mode = iio_device_find_channel(trigg_dev, "voltage5", false);

		if (trigger0Mode) {
			iio_channel_attr_read(trigger0Mode, "mode", buf, sizeof(buf));
			m_trigger0_mode.assign(buf);
			iio_channel_attr_write(trigger0Mode, "mode", "always");
		}
		if (trigger1Mode) {
			iio_channel_attr_read(trigger1Mode, "mode", buf, sizeof(buf));
			m_trigger1_mode.assign(buf);
			iio_channel_attr_write(trigger1Mode, "mode", "always");
		}
	}

	m_initialized = true;

	return m_initialized;
}

bool RxCalibration::isInitialized()
{
	return m_initialized;
}

bool RxCalibration::calibrateOffset()
{
	bool calibrated = false;

	if (!m_initialized) {
		qDebug() << "Rx path is not initialized for calibration.";
		return false;
	}

	qDebug() << "Starting ADC OFFSET CALIBRATION";

	// Ground ADC inputs
	iio_device_attr_write(m_m2k_fabric, "calibration_mode", "adc_gnd");

	QThread::msleep(100); // Do we need this? or calibration_mode gets set to adc_gnd instantly?

	// Set DAC channels to middle scale
	iio_channel_attr_write_longlong(m_ad5625_channel2, "raw", 2048);
	iio_channel_attr_write_longlong(m_ad5625_channel3, "raw", 2048);

	const unsigned int num_samples = 1e5;
	int16_t dataCh0[num_samples];
	int16_t dataCh1[num_samples];

	bool ret = adc_data_capture(dataCh0, dataCh1, num_samples);
		if (!ret) {
		qDebug() << "failed to get samples";
		return false;
	}

	double ch0_avg = qAbs(average(dataCh0, num_samples));
	double ch1_avg = qAbs(average(dataCh1, num_samples));
	double voltage0 = adc_sample_conv::convSampleToVolts(ch0_avg);
	double voltage1 = adc_sample_conv::convSampleToVolts(ch1_avg);

	double gain = 0.026;
	double range = 3.192;

	m_adc_ch0_offset = (int)((voltage0 * 4096 * gain) / range + 2048);
	m_adc_ch1_offset = (int)((voltage1 * 4096 * gain) / range + 2048);

	qDebug() << "Before Fine-Tunning";
	qDebug() << "ADC channel 0 offset(raw):" << m_adc_ch0_offset;
	qDebug() << "ADC channel 1 offset(raw):" << m_adc_ch1_offset;

	fine_tune(20, m_adc_ch0_offset, m_adc_ch1_offset, num_samples);

	calibrated = true;

	return calibrated;
}

bool RxCalibration::calibrateGain()
{
	bool calibrated = false;

	if (!m_initialized) {
	qDebug() << "Rx path is not initialized for calibration.";
	return false;
	}

	qDebug() << "Starting ADC GAIN CALIBRATION";

	iio_device_attr_write(m_m2k_fabric, "calibration_mode", "adc_ref1");

	QThread::msleep(100); // Do we need this? or calibration_mode gets set to adc_ref1 instantly?

	double vref1 = 23.01;
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

	avg0 = qAbs(average(dataCh0, num_samples));
	avg1 = qAbs(average(dataCh1, num_samples));

	avg0 = adc_sample_conv::convSampleToVolts(avg0);
	avg1 = adc_sample_conv::convSampleToVolts(avg1);

	m_adc_ch0_gain = vref1 / avg0;
	m_adc_ch1_gain = vref1 / avg1;

	qDebug() << "Gain for channel0: " << m_adc_ch0_gain;
	qDebug() << "Gain for channel1: " << m_adc_ch1_gain;

	iio_device_attr_write(m_m2k_fabric, "calibration_mode", "none");

	calibrated = true;

	return calibrated;
}

int RxCalibration::adcOffsetChannel0()
{
	return m_adc_ch0_offset;
}

int RxCalibration::adcOffsetChannel1()
{
	return m_adc_ch1_offset;
}

double RxCalibration::adcGainChannel0()
{
	return m_adc_ch0_gain;
}

double RxCalibration::adcGainChannel1()
{
	return m_adc_ch1_gain;
}

bool RxCalibration::resetSettings()
{
	if (!m_initialized) {
		qDebug() << "Rx path is not initialized for calibration.";
		return false;
	}

	iio_device_attr_write(m_m2k_fabric, "calibration_mode", "none");

	m_adc_ch0_offset = 2048;
	m_adc_ch1_offset = 2048;

	m_adc_ch0_gain = 1;
	m_adc_ch1_gain = 1;

	iio_channel_attr_write_double(m_ad5625_channel2, "raw", m_adc_ch0_offset);
	iio_channel_attr_write_double(m_ad5625_channel3, "raw", m_adc_ch1_offset);

	return true;
}

void RxCalibration::restoreTriggerSetup()
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
}

void RxCalibration::setChannelEnableState(struct iio_channel *chn, bool en)
{
	if (en)
		iio_channel_enable(chn);
	else
		iio_channel_disable(chn);
}

double RxCalibration::average(int16_t *data, size_t numElements)
{
	double sum = 0;

	for (size_t i = 0; i < numElements; i++)
		sum += data[i];

	return (sum / (double)numElements);
}

bool RxCalibration::adc_data_capture(int16_t *dataCh0, int16_t *dataCh1, size_t num_sampl_per_chn)
{
	if (!dataCh0 && !dataCh1) {
		qDebug() << "At least one channels needs to be activated. Aborting calibration.";
		return false;
	}

	// Store channels enable state
	bool channel0Enabled = iio_channel_is_enabled(m_adc_channel0);
	bool channel1Enabled = iio_channel_is_enabled(m_adc_channel1);

	// Enable the required channels
	setChannelEnableState(m_adc_channel0, !!dataCh0);
	setChannelEnableState(m_adc_channel1, !!dataCh1);

	struct iio_buffer *buffer = iio_device_create_buffer(m_m2k_adc, num_sampl_per_chn, false);

	if (!buffer) {
		qDebug() << "Could not create m2k-adc buffer!" << strerror(errno) << "Aborting calibration.";
		setChannelEnableState(m_adc_channel0, channel0Enabled);
		setChannelEnableState(m_adc_channel1, channel1Enabled);
		return false;
	}

	int ret = iio_buffer_refill(buffer);

	if (ret < 0) {
		qDebug() << "Could not refill m2k-adc buffer! Error:" << ret << "Aborting calibration";
		iio_buffer_destroy(buffer);
		setChannelEnableState(m_adc_channel0, channel0Enabled);
		setChannelEnableState(m_adc_channel1, channel1Enabled);
		return false;
	}

	ptrdiff_t p_inc = iio_buffer_step(buffer);
	uintptr_t p_dat;
	uintptr_t p_end = (uintptr_t)iio_buffer_end(buffer);
	unsigned int i;
	for (i = 0, p_dat = (uintptr_t)iio_buffer_first(buffer, m_adc_channel0); p_dat < p_end; p_dat += p_inc, i++)
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

bool RxCalibration::fine_tune(size_t span, int16_t centerVal0, int16_t centerVal1, size_t num_samples)
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
		iio_channel_attr_write_double(m_ad5625_channel2, "raw", offset0);
		iio_channel_attr_write_double(m_ad5625_channel3, "raw", offset1);
		offset0++;
		offset1++;

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

	iio_device_attr_write(m_m2k_fabric, "calibration_mode", "none");

	m_adc_ch0_offset = candidateOffsets0[i0];
	m_adc_ch1_offset = candidateOffsets1[i1];

	qDebug() << "After Fine-Tunning";
	qDebug() << "ADC channel 0 offset(raw):" << m_adc_ch0_offset;
	qDebug() << "ADC channel 1 offset(raw):" << m_adc_ch1_offset;

	iio_channel_attr_write_longlong(m_ad5625_channel2, "raw", m_adc_ch0_offset);
	iio_channel_attr_write_longlong(m_ad5625_channel3, "raw", m_adc_ch1_offset);

out_cleanup:
	delete[] candidateOffsets0;
	delete[] candidateOffsets1;
	delete[] averagesCh0;
	delete[] averagesCh1;
	delete[] dataCh0;
	delete[] dataCh1;
	return ret;
}
