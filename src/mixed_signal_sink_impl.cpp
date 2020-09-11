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

#include "mixed_signal_sink_impl.h"

#include <volk/volk.h>
#include <qapplication.h>

using namespace gr;


mixed_signal_sink::sptr mixed_signal_sink::make(adiscope::logic::LogicAnalyzer *logicAnalyzer,
						adiscope::TimeDomainDisplayPlot *oscPlot,
						int bufferSize)
{
	return gnuradio::get_initial_sptr(
				new mixed_signal_sink_impl(logicAnalyzer, oscPlot, bufferSize));
}

mixed_signal_sink_impl::mixed_signal_sink_impl(adiscope::logic::LogicAnalyzer *logicAnalyzer,
					       adiscope::TimeDomainDisplayPlot *oscPlot,
					       int bufferSize)
	: sync_block("mixed_signal_sink",
		     io_signature::make3(3, 3, sizeof(float), sizeof(float), sizeof(unsigned short)),
		     io_signature::make(0, 0, 0))
	, d_logic_analyzer(logicAnalyzer)
	, d_osc_plot(oscPlot)
	, d_size(bufferSize)
	, d_buffer_size(2 * bufferSize)
	, d_index(0)
	, d_end(d_size)
	, d_tags(std::vector< std::vector<gr::tag_t> >(3))
	, d_trigger_tag_key(pmt::intern("buffer_start"))
	, d_triggered(false)
	, d_update_time(0.1 * gr::high_res_timer_tps())
	, d_last_time(0)
{
	d_digital_buffer = static_cast<uint16_t*>(volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment()));
	memset(d_digital_buffer, 0, d_buffer_size * sizeof(uint16_t));

	for (int i = 0; i < 2; ++i) {
		d_analog_buffer.push_back(
					static_cast<float*>(volk_malloc(d_buffer_size * sizeof(float), volk_get_alignment())));
		memset(d_analog_buffer[i], 0, d_buffer_size * sizeof(float));

		d_analog_plot_buffers.push_back(
					static_cast<double*>(volk_malloc(d_buffer_size * sizeof(double), volk_get_alignment())));
		memset(d_analog_plot_buffers[i], 0, d_buffer_size * sizeof(double));
	}
}

int mixed_signal_sink_impl::work(int noutput_items,
				 gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	gr::thread::scoped_lock lock(d_setlock);

	// space left in buffer
	const int nfill = d_end - d_index;
	// num items we can put in the buffer
	const int nitems = std::min(noutput_items, nfill);

	// look for trigger tag
	_test_trigger_tags(nitems);

	for (int i = 0; i < 2; ++i) {
		const float *in = static_cast<const float*>(input_items[i]);
		memcpy(&d_analog_buffer[i][d_index], in, sizeof(float) * nitems);
	}

	const uint16_t *in = static_cast<const uint16_t *>(input_items[2]);
	memcpy(d_digital_buffer + d_index, in, sizeof(uint16_t) * nitems);

	d_index += nitems;

	if (d_triggered && (d_index == d_end) && d_end != 0) {
		if (gr::high_res_timer_now() - d_last_time > d_update_time) {
			d_last_time = gr::high_res_timer_now();

			for (int i = 0; i < 2; ++i) {
				volk_32f_convert_64f(d_analog_plot_buffers[i], &d_analog_buffer[i][d_start], d_size);
			}

			d_logic_analyzer->setData(d_digital_buffer + d_start, d_size);
			qApp->postEvent(d_osc_plot,
					new IdentifiableTimeUpdateEvent(d_analog_plot_buffers,
									d_size,
									d_tags,
									"Osc Time"));
		}

		_reset();
	}


	if (d_index == d_end) {
		_reset();
	}

	return nitems;
}

void mixed_signal_sink_impl::clean_buffers()
{
	gr::thread::scoped_lock lock(d_setlock);

	memset(d_digital_buffer, 0, d_buffer_size * sizeof(uint16_t));

	for (int i = 0; i < 2; ++i) {
		memset(d_analog_buffer[i], 0, d_buffer_size * sizeof(float));
		memset(d_analog_plot_buffers[i], 0, d_buffer_size * sizeof(double));
	}

	_reset();
}

void mixed_signal_sink_impl::set_nsamps(int newsize)
{
	if (newsize != d_size) {
		gr::thread::scoped_lock lock(d_setlock);

		// set new size
		d_size = newsize;
		d_buffer_size = 2 * d_size;

		// free old buffers
		volk_free(d_digital_buffer);
		for (int i = 0; i < 2; ++i) {
			volk_free(d_analog_buffer[i]);
			volk_free(d_analog_plot_buffers[i]);

		}
		d_analog_buffer.clear();
		d_analog_plot_buffers.clear();

		// create new buffers
		d_digital_buffer = static_cast<uint16_t*>(volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment()));
		memset(d_digital_buffer, 0, d_buffer_size * sizeof(uint16_t));

		for (int i = 0; i < 2; ++i) {
			d_analog_buffer.push_back(
						static_cast<float*>(volk_malloc(d_buffer_size * sizeof(float), volk_get_alignment())));
			memset(d_analog_buffer[i], 0, d_buffer_size * sizeof(float));
			d_analog_plot_buffers.push_back(
						static_cast<double*>(volk_malloc(d_buffer_size * sizeof(double), volk_get_alignment())));
			memset(d_analog_plot_buffers[i], 0, d_buffer_size * sizeof(double));
		}

		_reset();
	}
}

void mixed_signal_sink_impl::_adjust_tags(int adj)
{
	for (size_t n = 0; n < d_tags.size(); ++n) {
		for (size_t t = 0; t < d_tags[n].size(); ++t) {
			d_tags[n][t].offset += adj;
		}
	}
}

void mixed_signal_sink_impl::_test_trigger_tags(int nitems)
{
	int trigger_index;

	uint64_t nr = nitems_read(0);
	std::vector<gr::tag_t> tags;
	get_tags_in_range(tags, 0, nr, nr + nitems + 1, d_trigger_tag_key);
	if (tags.size() > 0) {
		d_triggered = true;
		trigger_index = tags[0].offset - nr;
		d_start = d_index + trigger_index;
		d_end = d_start + d_size;
		_adjust_tags(-d_start);
	}
}

void mixed_signal_sink_impl::_reset()
{
	for (int i = 0; i < d_tags.size(); ++i) {
		d_tags[i].clear();
	}

	d_start = 0;
	d_index = 0;
	d_end = d_size;

	d_triggered = false;
}
