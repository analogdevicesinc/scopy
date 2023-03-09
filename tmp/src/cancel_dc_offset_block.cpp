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
#include "cancel_dc_offset_block.h"

#include <gnuradio/blocks/moving_average.h>
#include <gnuradio/blocks/repeat.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/copy.h>
#include <gnuradio/blocks/keep_one_in_n.h>

using namespace adiscope;
using namespace gr;

cancel_dc_offset_block::cancel_dc_offset_block(size_t buffer_size, bool enabled):
	hier_block2("DCOFFSET",
		    io_signature::make(1, 1, sizeof(float)),
		    io_signature::make(1, 1, sizeof(float))),
	QObject(),
	d_enabled(enabled),
	d_buffer_size(buffer_size),
	d_dc_offset(0.0),
	d_signal(std::make_shared<signal_sample>())
{
	_build_and_connect_blocks();

	QObject::connect(&*d_signal, &signal_sample::triggered, [=](std::vector<float> samples){
		d_dc_offset = samples[0];
	});
}

cancel_dc_offset_block::~cancel_dc_offset_block()
{
}

void cancel_dc_offset_block::set_enabled(bool enabled)
{
	if (d_enabled != enabled) {
		d_enabled = enabled;
		_build_and_connect_blocks();
	}
}

void cancel_dc_offset_block::set_buffer_size(size_t buffer_size)
{
	if (d_buffer_size != buffer_size) {
		d_buffer_size = buffer_size;
		_build_and_connect_blocks();
	}
}

float cancel_dc_offset_block::get_dc_offset() const
{
	return d_dc_offset;
}

void cancel_dc_offset_block::_build_and_connect_blocks()
{
	// Remove all connections
	hier_block2::disconnect_all();

	if (d_enabled) {
		if(d_buffer_size == 0)
			d_buffer_size = 1;
		auto avg = gr::blocks::moving_average_ff::make(d_buffer_size, 1.0 / d_buffer_size, d_buffer_size);
		auto keep = gr::blocks::keep_one_in_n::make(sizeof(float), d_buffer_size);
		auto repeat = gr::blocks::repeat::make(sizeof(float), d_buffer_size);
		auto sub = gr::blocks::sub_ff::make();

		hier_block2::connect(this->self(), 0, avg, 0);
		hier_block2::connect(avg, 0, keep, 0);
		hier_block2::connect(keep, 0, d_signal, 0);
		hier_block2::connect(keep, 0, repeat, 0);
		hier_block2::connect(this->self(), 0, sub, 0);
		hier_block2::connect(repeat, 0, sub, 1);
		hier_block2::connect(sub, 0, this->self(), 0);
	} else {
		auto avg = gr::blocks::moving_average_ff::make(d_buffer_size, 1.0 / d_buffer_size, d_buffer_size);
		auto keep = gr::blocks::keep_one_in_n::make(sizeof(float), d_buffer_size);

		hier_block2::connect(this->self(), 0, avg, 0);
		hier_block2::connect(avg, 0, keep, 0);
		hier_block2::connect(keep, 0, d_signal, 0);

		auto copy = gr::blocks::copy::make(sizeof(float));
		hier_block2::connect(this->self(), 0, copy, 0);
		hier_block2::connect(copy, 0, this->self(), 0);
	}
}
