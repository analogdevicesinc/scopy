/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
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

#include "streams_to_short_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/char_to_short.h>

using namespace gr;

namespace adiscope
{
    streams_to_short_impl::streams_to_short_impl(size_t _itemsize,
						size_t _nstreams)
	: sync_block("streams_to_short",
		io_signature::make(_nstreams, _nstreams, sizeof(uint16_t)),
                io_signature::make (1, 1, sizeof(uint16_t))),
	d_nstreams(_nstreams),
        d_itemsize(_itemsize)
    {
    }

    streams_to_short::sptr
    streams_to_short::make(size_t itemsize, size_t nstreams)
    {
        return gnuradio::get_initial_sptr
		(new streams_to_short_impl(itemsize, nstreams));
    }

    int
    streams_to_short_impl::work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        uint16_t *out = (uint16_t*) output_items[0];
        uint16_t v;
        uint16_t d_value;

        for(int i = 0; i < noutput_items; i++) {
            d_value = 0;
            for(int j = 0; j < d_nstreams; j++) {
                const uint16_t *inv = (const uint16_t *) input_items[j];
                d_value |= (inv[j] << j);
            }
            out[i] = d_value;
        }
        return noutput_items;
    }
}
