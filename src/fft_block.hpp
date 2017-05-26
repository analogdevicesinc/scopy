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

#ifndef FFT_BLOCK_HPP
#define FFT_BLOCK_HPP

#include <gnuradio/hier_block2.h>

namespace adiscope {
	class fft_block : public gr::hier_block2
	{
	public:
		fft_block(bool use_complex, size_t fft_size,
				unsigned int nbthreads = 1);
		~fft_block();

		bool set_window(const std::vector<float>& window);

	private:
		bool d_complex;
		gr::basic_block_sptr d_fft;
	};
}

#endif /* FFT_BLOCK_HPP */
