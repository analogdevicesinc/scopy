/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "adc_sample_conv.hpp"
#include <libm2k/analog/m2kanalogin.hpp>

using namespace gr;
using namespace adiscope;
using namespace libm2k::analog;

adc_sample_conv::adc_sample_conv(int nconnections,
				 M2kAnalogIn* adc,
				 bool inverse) :
	gr::sync_block("adc_sample_conv",
			gr::io_signature::make(nconnections, nconnections, sizeof(float)),
			gr::io_signature::make(nconnections, nconnections, sizeof(float))),
	d_nconnections(nconnections),
	inverse(inverse),
	m2k_adc(adc)
{
	for (int i = 0; i < d_nconnections; i++) {
		d_correction_gains.push_back(1.0);
		d_filter_compensations.push_back(1.0);
		d_offsets.push_back(0.0);
		d_hardware_gains.push_back(0.02);
	}
}

adc_sample_conv::~adc_sample_conv()
{
}

double adc_sample_conv::conversionWrapper(unsigned int chn_idx, double sample, bool raw_to_volts)
{
	try {
		if (raw_to_volts) {
			return m2k_adc->convertRawToVolts(chn_idx, (short)sample);
		} else {
			return m2k_adc->convertVoltsToRaw(chn_idx, sample);
		}
	} catch (std::exception &e) {
		return 0;
	}
}

int adc_sample_conv::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	gr::thread::scoped_lock lock(d_setlock);

	for (unsigned int i = 0; i < input_items.size(); i++) {
		const float* in = static_cast<const float *>(input_items[i]);
		float *out = static_cast<float *>(output_items[i]);

		if (inverse) {
			for (int j = 0; j < noutput_items; j++) {
				out[j] = m2k_adc->convertVoltsToRaw(i, in[j]);
			}
		} else {

			for (int j = 0; j < noutput_items; j++)
				out[j] = m2k_adc->convertRawToVolts(i, in[j]);
		}
	}

	return noutput_items;
}

void adc_sample_conv::setCorrectionGain(int connection, float gain)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_correction_gains[connection] != gain) {
		gr::thread::scoped_lock lock(d_setlock);
		d_correction_gains[connection] = gain;
	}
}

float adc_sample_conv::correctionGain(int connection)
{
	if (connection >= 0 && connection < d_nconnections)
		return d_correction_gains[connection];

	return 0.0;
}

void adc_sample_conv::setFilterCompensation(int connection, float val)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_filter_compensations[connection] != val) {
		gr::thread::scoped_lock lock(d_setlock);
		d_filter_compensations[connection] = val;
	}
}

float adc_sample_conv::filterCompensation(int connection)
{
	if (connection >= 0 && connection < d_nconnections)
		return d_filter_compensations[connection];

	return 0.0;
}

void adc_sample_conv::setOffset(int connection, float offset)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_offsets[connection] != offset) {
		gr::thread::scoped_lock lock(d_setlock);
		d_offsets[connection] = offset;
	}
}

float adc_sample_conv::offset(int connection) const
{
	if (connection >= 0 && connection < d_nconnections)
		return d_offsets[connection];

	return 0;
}

void adc_sample_conv::setHardwareGain(int connection, float gain)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_hardware_gains[connection] != gain) {
		gr::thread::scoped_lock lock(d_setlock);
		d_hardware_gains[connection] = gain;
	}
}

float adc_sample_conv::hardwareGain(int connection) const
{
	if (connection >= 0 && connection < d_nconnections)
		return d_hardware_gains[connection];

	return 0.0;
}
