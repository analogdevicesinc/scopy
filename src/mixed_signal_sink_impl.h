/*
 * Copyright (c) 2020 Analog Devices Inc.
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

#ifndef MIXED_SIGNAL_SINK_IMPL_H
#define MIXED_SIGNAL_SINK_IMPL_H

#include "mixed_signal_sink.h"

class mixed_signal_sink_impl : public mixed_signal_sink
{
public:
	mixed_signal_sink_impl(adiscope::logic::LogicAnalyzer *logicAnalyzer,
			       adiscope::TimeDomainDisplayPlot *oscPlot,
			       int bufferSize);
	int work(int noutput_items,
		 gr_vector_const_void_star &input_items,
		 gr_vector_void_star &output_items) override;
	void clean_buffers() override;
	void set_nsamps(int newsize) override;

private:
	void _adjust_tags(int adj);
	void _test_trigger_tags(int nitems);
	void _reset();

private:
	adiscope::logic::LogicAnalyzer *d_logic_analyzer;
	adiscope::TimeDomainDisplayPlot *d_osc_plot;

	std::vector<float*> d_analog_buffer;
	std::vector<double*> d_analog_plot_buffers;
	uint16_t *d_digital_buffer;

	int d_size;
	int d_buffer_size;
	int d_index;
	int d_end;
	int d_start;

	std::vector< std::vector<gr::tag_t> > d_tags;
	pmt::pmt_t d_trigger_tag_key;
	bool d_triggered;

	gr::high_res_timer_type d_update_time;
	gr::high_res_timer_type d_last_time;
};

#endif // MIXED_SIGNAL_SINK_IMPL_H
