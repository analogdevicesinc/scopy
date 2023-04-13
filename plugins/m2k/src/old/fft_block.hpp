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

#ifndef FFT_BLOCK_HPP
#define FFT_BLOCK_HPP

#include <gnuradio/hier_block2.h>

namespace scopy {
	class fft_block : public gr::hier_block2
	{
	public:
		fft_block(bool use_complex, size_t fft_size,
				unsigned int nbthreads = 1);
		~fft_block();

		void set_window(const std::vector<float>& window);
		void set_overlap_factor(double overlap_factor);

	private:
		bool d_complex;
		gr::basic_block_sptr d_fft;
		gr::basic_block_sptr d_s2v_overlap;
	};
}

#endif /* FFT_BLOCK_HPP */
