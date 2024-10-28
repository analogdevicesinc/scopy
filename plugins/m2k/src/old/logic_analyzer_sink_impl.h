/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LOGIC_ANALYZER_SINK_IMPL_H
#define LOGIC_ANALYZER_SINK_IMPL_H

#include "logic_analyzer_sink.h"

class logic_analyzer_sink_impl : public logic_analyzer_sink
{
public:
	logic_analyzer_sink_impl(scopy::m2k::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items);

	void clean_buffers();
	void set_nsamps(int newsize);

private:
	void _adjust_tags(int adj);
	void _test_trigger_tags(int nitems);
	void _reset();

private:
	scopy::m2k::logic::LogicAnalyzer *d_logic_analyzer;
	uint16_t *d_buffer;
	uint16_t *d_buffer_temp;
	int d_size;
	int d_buffer_size;
	int d_index;
	int d_end;
	int d_start;

	std::vector<std::vector<gr::tag_t>> d_tags;
	pmt::pmt_t d_trigger_tag_key;
	bool d_triggered;

	gr::high_res_timer_type d_update_time;
	gr::high_res_timer_type d_last_time;
};

#endif // LOGIC_ANALYZER_SINK_IMPL_H
