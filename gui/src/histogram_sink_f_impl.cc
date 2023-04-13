/* -*- c++ -*- */
/*
 * Copyright 2013 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "histogram_sink_f_impl.h"

#include <algorithm>

#include <gnuradio/io_signature.h>
#include <gnuradio/prefs.h>
#include <string.h>
#include <volk/volk.h>
#include <qwt_symbol.h>

using namespace gr;

namespace scopy {

    histogram_sink_f::sptr
    histogram_sink_f::make(int size, int bins,
                           double xmin, double xmax,
                           const std::string &name,
                           int nconnections,
                           QObject *plot)
    {
      return gnuradio::get_initial_sptr
	(new histogram_sink_f_impl(size, bins, xmin, xmax, name,
                                   nconnections, plot));
    }

    histogram_sink_f_impl::histogram_sink_f_impl(int size, int bins,
                                                 double xmin, double xmax,
                                                 const std::string &name,
                                                 int nconnections,
                                                 QObject *plot)
      : sync_block("histogram_sink_f",
                   io_signature::make(nconnections, nconnections, sizeof(float)),
                   io_signature::make(0, 0, 0)),
	d_size(size), d_bins(bins), d_xmin(xmin), d_xmax(xmax), d_name(name),
	d_nconnections(nconnections)
    {
      d_index = 0;

      for(int i = 0; i < d_nconnections; i++) {
	d_residbufs.push_back((double*)volk_malloc(d_size*sizeof(double),
                                                   volk_get_alignment()));
	memset(d_residbufs[i], 0, d_size*sizeof(double));
      }

      // Set alignment properties for VOLK
      const int alignment_multiple =
	volk_get_alignment() / sizeof(gr_complex);
      set_alignment(std::max(1,alignment_multiple));

      this->plot = (HistogramDisplayPlot*)plot;
	  initialize();
    }

    histogram_sink_f_impl::~histogram_sink_f_impl()
    {
      for(int i = 0; i < d_nconnections; i++) {
	volk_free(d_residbufs[i]);
      }
    }

    bool
    histogram_sink_f_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    histogram_sink_f_impl::initialize()
    {
      d_qApplication = NULL;
      if(qApp != NULL) {
	d_qApplication = qApp;
      }
	  set_update_time(1/60.0);

      plot->setNumBins(d_bins);
      plot->setXaxis(d_xmin, d_xmax);

    }

    void
    histogram_sink_f_impl::exec_()
    {
      d_qApplication->exec();
    }

    void
    histogram_sink_f_impl::set_update_time(double t)
    {
      //convert update time to ticks
      gr::high_res_timer_type tps = gr::high_res_timer_tps();
      d_update_time = t * tps;
      d_last_time = 0;
    }

    void
    histogram_sink_f_impl::set_nsamps(const int newsize)
    {
      gr::thread::scoped_lock lock(d_setlock);

      if(newsize != d_size) {
	// Resize residbuf and replace data
	for(int i = 0; i < d_nconnections; i++) {
	  volk_free(d_residbufs[i]);
	  d_residbufs[i] = (double*)volk_malloc(newsize*sizeof(double),
                                                volk_get_alignment());

	  memset(d_residbufs[i], 0, newsize*sizeof(double));
	}

	// Set new size and reset buffer index
	// (throws away any currently held data, but who cares?)
	d_size = newsize;
	d_index = 0;

      }
      plot->setDataInterval(0, d_size);
    }

    void
    histogram_sink_f_impl::set_bins(const int bins)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_bins = bins;
      plot->setNumBins(d_bins);
    }

    int
    histogram_sink_f_impl::nsamps() const
    {
      return d_size;
    }

    int
    histogram_sink_f_impl::bins() const
    {
      return d_bins;
    }

    void
    histogram_sink_f_impl::reset()
    {
      d_index = 0;
    }

    int
    histogram_sink_f_impl::work(int noutput_items,
			   gr_vector_const_void_star &input_items,
			   gr_vector_void_star &output_items)
    {
      int n=0, j=0, idx=0;
      const float *in = (const float*)input_items[idx];

      for(int i=0; i < noutput_items; i+=d_size) {
	unsigned int datasize = noutput_items - i;
	unsigned int resid = d_size-d_index;
	idx = 0;

	// If we have enough input for one full plot, do it
	if(datasize >= resid) {

	  // Fill up residbufs with d_size number of items
	  for(n = 0; n < d_nconnections; n++) {
	    in = (const float*)input_items[idx++];
	    volk_32f_convert_64f_u(&d_residbufs[n][d_index],
				   &in[j], resid);
	  }

	  // Update the plot if its time
	  if(gr::high_res_timer_now() - d_last_time > d_update_time) {
	    d_last_time = gr::high_res_timer_now();
	    if (d_qApplication)
	      d_qApplication->postEvent(this->plot,
				      new HistogramUpdateEvent(d_residbufs, d_size));
	  }

	  d_index = 0;
	  j += resid;
	}
	// Otherwise, copy what we received into the residbufs for next time
	// because we set the output_multiple, this should never need to be called
	else {
	  for(n = 0; n < d_nconnections; n++) {
	    in = (const float*)input_items[idx++];
	    volk_32f_convert_64f_u(&d_residbufs[n][d_index],
				   &in[j], datasize);
	  }
	  d_index += datasize;
	  j += datasize;
	}
      }

      return j;
    }

} /* namespace scopy */
