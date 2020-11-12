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

#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>

#include <libm2k/m2kexceptions.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include "scopyExceptionHandler.h"

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
}

adc_sample_conv::~adc_sample_conv()
{
}

double adc_sample_conv::conversionWrapper(unsigned int chn_idx, double sample, bool raw_to_volts)
{
	try {
		if (raw_to_volts) {
			return m2k_adc->convertRawToVolts(chn_idx, (short)sample);
			//return (double)sample;
		} else {
			return m2k_adc->convertVoltsToRaw(chn_idx, sample);
		}
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
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
