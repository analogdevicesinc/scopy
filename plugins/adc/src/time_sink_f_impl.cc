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

#include "qdebug.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <gnuradio/buffer.h>
#include <gnuradio/prefs.h>
#include <string.h>
#include <volk/volk.h>
#include <gnuradio/fft/fft.h>
#include <qwt_symbol.h>

#include "time_sink_f_impl.h"

using namespace gr;

namespace scopy {

time_sink_f::sptr
time_sink_f::make(int size, const std::string &name,
		  int nconnections )
{
	return gnuradio::get_initial_sptr
	    (new time_sink_f_impl(size, name, nconnections));
}

time_sink_f_impl::time_sink_f_impl(int size,  const std::string &name,
				   int nconnections )
    : sync_block("time_sink_f",
		 io_signature::make(nconnections, nconnections, sizeof(float)),
		 io_signature::make(0, 0, 0)),
      m_size(size), m_name(name), m_nconnections(nconnections)
{
	for (int i = 0; i < m_nconnections;i++) {
		m_buffers.push_back(std::deque<float>());
	}

	m_tags = std::vector< std::vector<gr::tag_t> >(m_nconnections);

	set_update_time(1/60.0);
}

time_sink_f_impl::~time_sink_f_impl() {

}

bool time_sink_f_impl::check_topology(int ninputs, int noutputs) {
	return ninputs == m_nconnections;
}

void time_sink_f_impl::set_update_time(double t) {
	//convert update time to ticks
	gr::high_res_timer_type tps = gr::high_res_timer_tps();
	d_update_time = t * tps;
	d_last_time = 0;
}

void time_sink_f_impl::set_nsamps(const int size) {
	m_size = size;
}

int time_sink_f_impl::nsamps() const {
	return m_size;
}

std::string time_sink_f_impl::name() const {
	return m_name;
}

//void time_sink_f_impl::copy_data(int ch, float *dest, size_t size) {


//}



int time_sink_f_impl::work(int noutput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items) {
	gr::thread::scoped_lock lock(d_setlock);

	for(int i = 0; i < m_nconnections; i++) {
		for(int j = 0; j < noutput_items; j++) {
			if(m_buffers.size() > m_size) {
				m_buffers[j].pop_back();
			}
//			m_buffers[j].push_front(input_items[i][j]);

		}
	}


	return noutput_items;

}
} /* namespace gr */
