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

#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include "stream_to_vector_overlap.h"

#include "fft_block.hpp"

using namespace scopy;
using namespace gr;

fft_block::fft_block(bool use_complex, size_t fft_size, unsigned int nbthreads)
	: hier_block2("FFT",
		      io_signature::make(1, 1, use_complex ?
						 sizeof(gr_complex) : sizeof(float)),
		      io_signature::make(1, 1, sizeof(gr_complex))),
	  d_complex(use_complex)
{
	auto v2s = blocks::vector_to_stream::make(sizeof(gr_complex), fft_size);

	d_s2v_overlap = scopy::stream_to_vector_overlap::make(
				use_complex ? sizeof(gr_complex) : sizeof(float),
				fft_size, 0.0);

	/* We use a Hamming window for now */
	auto window = fft::window::hamming(fft_size);

	//basic_block_sptr fft;
	if (use_complex)
		d_fft = fft::fft_v<gr_complex, true>::make(fft_size,
							   window, false, nbthreads);
	else
		d_fft = fft::fft_v<float, true>::make(fft_size,
						      window, false, nbthreads);

	/* Connect everything */
	hier_block2::connect(this->self(), 0, d_s2v_overlap, 0);
	hier_block2::connect(d_s2v_overlap, 0, d_fft, 0);
	hier_block2::connect(d_fft, 0, v2s, 0);
	hier_block2::connect(v2s, 0, this->self(), 0);
}

fft_block::~fft_block()
{
}

void fft_block::set_overlap_factor(double overlap_factor)
{
	std::dynamic_pointer_cast<scopy::stream_to_vector_overlap>(
				d_s2v_overlap)->set_overlap_factor(overlap_factor);
}

void fft_block::set_window(const std::vector<float>& window)
{
	if (d_complex) {
		std::dynamic_pointer_cast<fft::fft_v<gr_complex, true>>(d_fft)->set_window(window);
	} else {
		std::dynamic_pointer_cast<fft::fft_v<float,true>>(d_fft)->set_window(window);
	}
}
