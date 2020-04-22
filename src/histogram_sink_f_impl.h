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

#ifndef M2K_HISTOGRAM_SINK_F_IMPL_H
#define M2K_HISTOGRAM_SINK_F_IMPL_H

#include "HistogramDisplayPlot.h"
#include "histogram_sink_f.h"

#include <gnuradio/high_res_timer.h>

namespace adiscope {

class histogram_sink_f_impl : public histogram_sink_f
{
private:
	void initialize();

	int d_size;
	int d_bins;
	double d_xmin, d_xmax;
	std::string d_name;
	int d_nconnections;

	int d_index;
	std::vector<double*> d_residbufs;

	HistogramDisplayPlot* plot;

	gr::high_res_timer_type d_update_time;
	gr::high_res_timer_type d_last_time;

public:
	histogram_sink_f_impl(int size, int bins, double xmin, double xmax, const std::string& name, int nconnections,
			      QObject* plot = NULL);
	~histogram_sink_f_impl();

	bool check_topology(int ninputs, int noutputs);

	void exec_();

	void set_update_time(double t);
	void set_nsamps(const int newsize);
	void set_bins(const int bins);

	int nsamps() const;
	int bins() const;
	void reset();

	int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items);
};

} /* namespace adiscope */

#endif /* M2K_HISTOGRAM_SINK_F_IMPL_H */
