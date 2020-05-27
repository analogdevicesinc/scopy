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

#ifndef ADC_SAMPLE_CONV_HPP
#define ADC_SAMPLE_CONV_HPP

#include <gnuradio/sync_block.h>
#include <memory>
namespace libm2k {
namespace analog {
class M2kAnalogIn;
}
}
namespace adiscope {
	class adc_sample_conv : public gr::sync_block
	{
	private:
		int d_nconnections;
		bool inverse;
		libm2k::analog::M2kAnalogIn* m2k_adc;


		std::vector<float> d_correction_gains;
		std::vector<float> d_filter_compensations;
		std::vector<float> d_offsets;
		std::vector<float> d_hardware_gains;

	public:
		explicit adc_sample_conv(int nconnections,
					 libm2k::analog::M2kAnalogIn* m2k_adc,
					 bool inverse = false);

		~adc_sample_conv();

		void setCorrectionGain(int connection, float gain);
		float correctionGain(int connection);

		void setFilterCompensation(int connection, float val);
		float filterCompensation(int connection);

		void setOffset(int connection, float offset);
		float offset(int connection) const;

		void setHardwareGain(int connection, float gain);
		float hardwareGain(int connection) const;
		int work(int noutput_items,
				gr_vector_const_void_star &input_items,
			 gr_vector_void_star &output_items);
		double conversionWrapper(unsigned int chn_idx, double sample, bool raw_to_volts);
	};
}

#endif /* ADC_SAMPLE_CONV_HPP */
