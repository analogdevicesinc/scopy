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

#include "peek_sample.hpp"

using namespace adiscope;

template <typename T>
peek_sample<T>::peek_sample() :
	gr::sync_block("peek_sample",
			gr::io_signature::make(1, 1, sizeof(T)),
			gr::io_signature::make(0, 0, 0))
{
	this->value = (T) 0.0;
}

template <typename T>
peek_sample<T>::~peek_sample()
{
}

template <typename T>
int peek_sample<T>::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const T *vect = (const T *) input_items[0];
	this->value = vect[noutput_items - 1];
	consume_each(noutput_items);
	return 0;
}

template class peek_sample<char>;
template class peek_sample<short>;
template class peek_sample<int>;
template class peek_sample<float>;
template class peek_sample<gr_complex>;
