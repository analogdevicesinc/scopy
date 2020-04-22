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

#ifndef ADC_SAMPLE_CONV_HPP
#define ADC_SAMPLE_CONV_HPP

#include <gnuradio/sync_block.h>

#include <memory>

namespace adiscope {

class M2kAdc;
class adc_sample_conv : public gr::sync_block
{
private:
	int d_nconnections;
	bool inverse;
	std::vector<float> d_correction_gains;
	std::vector<float> d_filter_compensations;
	std::vector<float> d_offsets;
	std::vector<float> d_hardware_gains;
	std::shared_ptr<M2kAdc> m2k_adc;
	void updateCorrectionGain();

public:
	explicit adc_sample_conv(int nconnections, std::shared_ptr<M2kAdc> m2k_adc, bool inverse = false);
	~adc_sample_conv();

	static float convSampleToVolts(float sample, float correctionGain = 1, float filterCompensation = 1,
				       float offset = 0, float hw_gain = 0.02);
	static float convVoltsToSample(float sample, float correctionGain = 1, float filterCompensation = 1,
				       float offset = 0, float hw_gain = 0.02);

	void setCorrectionGain(int connection, float gain);
	float correctionGain(int connection);

	void setFilterCompensation(int connection, float val);
	float filterCompensation(int connection);

	void setOffset(int connection, float offset);
	float offset(int connection) const;

	void setHardwareGain(int connection, float gain);
	float hardwareGain(int connection) const;

	int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items);
};
} // namespace adiscope

#endif /* ADC_SAMPLE_CONV_HPP */
