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

#include "signal_sample.hpp"

using namespace adiscope;

signal_sample::signal_sample()
	: gr::sync_block("signal_sample",
			 gr::io_signature::make(1, -1, sizeof(float)),
			 gr::io_signature::make(0, 0, 0))
	, QObject() {
	qRegisterMetaType<std::vector<float>>();
	set_max_noutput_items(1);
}

signal_sample::~signal_sample() {}

int signal_sample::work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items) {
	std::vector<float> values;

	for (unsigned int i = 0; i < input_items.size(); i++) {
		const float *vect = (const float *)input_items[i];
		values.push_back(*vect);
	}

	Q_EMIT triggered(values);

	consume_each(1);
	return 0;
}
