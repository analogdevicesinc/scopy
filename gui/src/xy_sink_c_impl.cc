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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/prefs.h>
#include <string.h>
#include <volk/volk.h>
#include <qwt_symbol.h>

#include "xy_sink_c_impl.h"
#include"spectrumUpdateEvents.h"

using namespace gr;

namespace adiscope {

    xy_sink_c::sptr
    xy_sink_c::make(int size,
		       const std::string &name,
		       int nconnections,
		       QObject *plot)
    {
      return gnuradio::get_initial_sptr
	(new xy_sink_c_impl(size, name, nconnections, plot));
    }

    xy_sink_c_impl::xy_sink_c_impl(int size,
					 const std::string &name,
					 int nconnections,
					 QObject *plot)
      : sync_block("xy_sink_c",
		   io_signature::make(nconnections, nconnections, sizeof(gr_complex)),
		   io_signature::make(0, 0, 0)),
	d_size(size), d_buffer_size(2*size), d_name(name),
	d_nconnections(nconnections), d_index(0), d_start(0), d_end(size)
    {

      for(int i = 0; i < d_nconnections; i++) {
	d_residbufs_real.push_back((double*)volk_malloc(d_buffer_size*sizeof(double),
                                                        volk_get_alignment()));
	d_residbufs_imag.push_back((double*)volk_malloc(d_buffer_size*sizeof(double),
                                                        volk_get_alignment()));
	memset(d_residbufs_real[i], 0, d_buffer_size*sizeof(double));
	memset(d_residbufs_imag[i], 0, d_buffer_size*sizeof(double));
      }

      // Set alignment properties for VOLK
      const int alignment_multiple =
	volk_get_alignment() / sizeof(gr_complex);
      set_alignment(std::max(1,alignment_multiple));

      initialize();
      this->plot = (ConstellationDisplayPlot*)plot;
   }

    xy_sink_c_impl::~xy_sink_c_impl()
    {
      // d_main_gui is a qwidget destroyed with its parent
      for(int i = 0; i < d_nconnections; i++) {
	volk_free(d_residbufs_real[i]);
	volk_free(d_residbufs_imag[i]);
      }
    }

    bool
    xy_sink_c_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    xy_sink_c_impl::initialize()
    {
	d_qApplication = NULL;
      if(qApp != NULL) {
	d_qApplication = qApp;
      }

	  set_update_time(1/60.0);
    }

    void
    xy_sink_c_impl::exec_()
    {
      d_qApplication->exec();
    }

    void
    xy_sink_c_impl::set_update_time(double t)
    {
      //convert update time to ticks
      gr::high_res_timer_type tps = gr::high_res_timer_tps();
      d_update_time = t * tps;
      d_last_time = 0;
    }

    void
    xy_sink_c_impl::set_nsamps(const int newsize)
    {
      gr::thread::scoped_lock lock(d_setlock);

      if(newsize != d_size) {
	// Set new size and reset buffer index
	// (throws away any currently held data, but who cares?)
	d_size = newsize;
        d_buffer_size = 2*d_size;
	d_index = 0;

	// Resize residbuf and replace data
	for(int i = 0; i < d_nconnections; i++) {
	  volk_free(d_residbufs_real[i]);
	  volk_free(d_residbufs_imag[i]);
	  d_residbufs_real[i] = (double*)volk_malloc(d_buffer_size*sizeof(double),
                                                     volk_get_alignment());
	  d_residbufs_imag[i] = (double*)volk_malloc(d_buffer_size*sizeof(double),
                                                     volk_get_alignment());

	  memset(d_residbufs_real[i], 0, d_buffer_size*sizeof(double));
	  memset(d_residbufs_imag[i], 0, d_buffer_size*sizeof(double));
	}

        _reset();
      }
    }

    int
    xy_sink_c_impl::nsamps() const
    {
      return d_size;
    }

    void
    xy_sink_c_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    xy_sink_c_impl::_reset()
    {
      // Reset the start and end indices.
      d_start = 0;
      d_end = d_size;
      d_index = 0;
    }

    void
    xy_sink_c_impl::_npoints_resize()
    {
    }

    int
    xy_sink_c_impl::work(int noutput_items,
			    gr_vector_const_void_star &input_items,
			    gr_vector_void_star &output_items)
    {
      int n=0;
      const gr_complex *in;

      _npoints_resize();

      int nfill = d_end - d_index;                 // how much room left in buffers
      int nitems = std::min(noutput_items, nfill); // num items we can put in buffers

      // Copy data into the buffers.
      for(n = 0; n < d_nconnections; n++) {
        in = (const gr_complex*)input_items[n];
        volk_32fc_deinterleave_64f_x2(&d_residbufs_real[n][d_index],
                                      &d_residbufs_imag[n][d_index],
                                      &in[0], nitems);
      }
      d_index += nitems;


      // If we have a full d_size of items in the buffers, plot.
      if((d_index == d_end) && d_end  != 0) {
        // Copy data to be plotted to start of buffers.
        for(n = 0; n < d_nconnections; n++) {
          memmove(d_residbufs_real[n], &d_residbufs_real[n][d_start], d_size*sizeof(double));
          memmove(d_residbufs_imag[n], &d_residbufs_imag[n][d_start], d_size*sizeof(double));
        }

        // Plot if we are able to update
        if(gr::high_res_timer_now() - d_last_time > d_update_time) {
          d_last_time = gr::high_res_timer_now();
          if (d_qApplication)
		d_qApplication->postEvent(plot,
                                    new ConstUpdateEvent(d_residbufs_real,
							 d_residbufs_imag,
							 d_size));
        }

        // We've plotting, so reset the state
        _reset();
      }

      // If we've filled up the buffers but haven't triggered, reset.
      if(d_index == d_end) {
        _reset();
      }

      return nitems;
    }

} /* namespace adiscope */
