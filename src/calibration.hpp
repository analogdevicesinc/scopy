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

#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

#include <cstdint>
#include <cstdlib>
#include <string>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace adiscope {

class Calibration
{
public:
	Calibration(struct iio_context *ctx = NULL);
	~Calibration();

	bool initialize();
	bool isInitialized() const;

	bool calibrateAll();
	bool calibrateADCoffset();
	bool calibrateADCgain();
	bool calibrateDACoffset();
	bool calibrateDACgain();

	int adcOffsetChannel0() const;
	int adcOffsetChannel1() const;
	int dacAoffset() const;
	int dacBoffset() const;
	double adcGainChannel0() const;
	double adcGainChannel1() const;
	double dacAvlsb() const;
	double dacBvlsb() const;

	bool resetSettings();
	void restoreTriggerSetup();

	static void setChannelEnableState(struct iio_channel *chn, bool en);
	static double average(int16_t *data, size_t numElements);
	static float convSampleToVolts(float sample, float correctionGain = 1);
	static float convVoltsToSample(float sample, float correctionGain = 1);

private:
	bool adc_data_capture(int16_t *dataCh0, int16_t *dataCh1,
		size_t num_sampl_per_chn);
	bool fine_tune(size_t span, int16_t centerVal0, int16_t centerVal1,
		size_t num_samples);

	void dacOutputDC(struct iio_device *dac, struct iio_channel *channel,
		struct iio_buffer** buffer, size_t value);
	void dacAOutputDC(int16_t value);
	void dacBOutputDC(int16_t value);

	struct iio_context *m_ctx;
	struct iio_device *m_m2k_adc;
	struct iio_device *m_m2k_dac_a;
	struct iio_device *m_m2k_dac_b;
	struct iio_device *m2k_ad5625;
	struct iio_device *m_m2k_fabric;

	struct iio_channel *m_adc_channel0;
	struct iio_channel *m_adc_channel1;

	struct iio_channel *m_dac_a_channel;
	struct iio_channel *m_dac_b_channel;

	struct iio_channel *m_ad5625_channel0;
	struct iio_channel *m_ad5625_channel1;
	struct iio_channel *m_ad5625_channel2;
	struct iio_channel *m_ad5625_channel3;

	struct iio_buffer *m_dac_a_buffer;
	struct iio_buffer *m_dac_b_buffer;

	int m_adc_ch0_offset;
	int m_adc_ch1_offset;
	int m_dac_a_ch_offset;
	int m_dac_b_ch_offset;
	double m_adc_ch0_gain;
	double m_adc_ch1_gain;
	double m_dac_a_ch_vlsb;
	double m_dac_b_ch_vlsb;

	std::string m_trigger0_mode;
	std::string m_trigger1_mode;

	bool m_initialized;
};

} // namespace adiscope

#endif /* CALIBRATION_HPP */
