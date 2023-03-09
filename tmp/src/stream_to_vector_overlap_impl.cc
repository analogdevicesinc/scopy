/* -*- c++ -*- */
/*
 * Copyright 2012,2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
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

#include "stream_to_vector_overlap_impl.h"
#include <gnuradio/sync_decimator.h>

using namespace gr;
namespace adiscope {

stream_to_vector_overlap::sptr
stream_to_vector_overlap::make(size_t itemsize,
			       size_t nitems_per_block,
			       double overlap_factor)
{
	return gnuradio::get_initial_sptr(new stream_to_vector_overlap_impl(
						  itemsize,
						  nitems_per_block,
						  overlap_factor));
}

stream_to_vector_overlap_impl::stream_to_vector_overlap_impl(size_t itemsize,
							     size_t nitems_per_block,
							     double overlap_factor)
	: gr::sync_decimator("stream_to_vector_overlap",
			 io_signature::make(1, 1, itemsize),
			 io_signature::make(1, 1, itemsize * nitems_per_block),
			 nitems_per_block),
	  m_overlap_factor(overlap_factor),
	  m_itemsize(itemsize),
	  m_nitems_per_block(nitems_per_block)
{
	m_nb_overlapped_items = (size_t)(nitems_per_block * m_overlap_factor);
}

int stream_to_vector_overlap_impl::work(int noutput_items,
					gr_vector_const_void_star& input_items,
					gr_vector_void_star& output_items)
{
	size_t block_size = output_signature()->sizeof_stream_item(0);
	size_t nb_input_items = noutput_items * block_size;
	size_t nb_total_copied_items = 0;
	size_t overlap_size = m_nb_overlapped_items * m_itemsize;
	const char* in = (const char*)input_items[0];
	char* out = (char*)output_items[0];

	size_t src_index = 0;
	while (src_index + block_size <= nb_input_items) {
		memcpy(out + nb_total_copied_items, in + src_index, block_size);
		nb_total_copied_items += block_size;
		src_index = src_index + block_size - overlap_size;
	}
	return nb_total_copied_items / block_size;
}

void stream_to_vector_overlap_impl::set_overlap_factor(double factor)
{
	m_overlap_factor = factor;
	m_nb_overlapped_items = m_nitems_per_block * m_overlap_factor;
}

}
