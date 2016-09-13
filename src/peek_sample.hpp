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

#ifndef PEEK_SAMPLE_HPP
#define PEEK_SAMPLE_HPP

#include <gnuradio/sync_block.h>

namespace adiscope {
	template <typename T>
	class peek_sample : public gr::sync_block
	{
	private:
		T value;

	public:
		explicit peek_sample();
		~peek_sample();

		int work(int noutput_items,
				gr_vector_const_void_star &input_items,
				gr_vector_void_star &output_items);

		T peek_value() { return this->value; }
	};
}

#endif
