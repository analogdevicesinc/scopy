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

#ifndef M2K_SCOPE_SINK_F_IMPL_H
#define M2K_SCOPE_SINK_F_IMPL_H

#include <gnuradio/high_res_timer.h>

#include "scope_sink_f.h"
#include "TimeDomainDisplayPlot.h"
#include "FftDisplayPlot.h"

namespace adiscope {

    class scope_sink_f_impl : public scope_sink_f
    {
    private:
      void initialize();

      int d_size, d_buffer_size;
      double d_samp_rate;
      std::string d_name;
      int d_nconnections;

      int d_index, d_start, d_end;
      std::vector<float*> d_fbuffers;
      std::vector<double*> d_buffers;
      std::vector< std::vector<gr::tag_t> > d_tags;

      QObject *plot;

      gr::high_res_timer_type d_update_time;
      gr::high_res_timer_type d_last_time;

      // Members used for triggering scope
      trigger_mode d_trigger_mode;
      int d_trigger_channel;
      pmt::pmt_t d_trigger_tag_key;
      bool d_triggered;

      bool d_displayOneBuffer;
      bool d_cleanBuffers;

      void _reset();
      void _npoints_resize();
      void _adjust_tags(int adj);
      void _test_trigger_tags(int nitems);

    public:
      scope_sink_f_impl(int size, double samp_rate,
		       const std::string &name,
		       int nconnections,
		       QObject *plot = NULL);
      ~scope_sink_f_impl();

      bool check_topology(int ninputs, int noutputs);

      void exec_();

      void set_update_time(double t);
      void set_nsamps(const int size);
      void set_samp_rate(const double samp_rate);
      void set_trigger_mode(trigger_mode mode, int channel,
			    const std::string &tag_key="");

      void set_displayOneBuffer(bool);

      int nsamps() const;
      std::string name() const;
      void reset();
      void clean_buffers();


      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

} /* namespace adiscope */

#endif /* M2K_SCOPE_SINK_F_IMPL_H */
