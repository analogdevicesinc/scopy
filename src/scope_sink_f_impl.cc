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

#include "scope_sink_f_impl.h"

using namespace gr;

namespace adiscope {

    scope_sink_f::sptr
    scope_sink_f::make(int size, double samp_rate,
		      const std::string &name,
		      int nconnections,
		      QObject *plot)
    {
      return gnuradio::get_initial_sptr
	(new scope_sink_f_impl(size, samp_rate, name, nconnections, plot));
    }

    scope_sink_f_impl::scope_sink_f_impl(int size, double samp_rate,
				       const std::string &name,
				       int nconnections,
				       QObject *plot)
      : sync_block("scope_sink_f",
                   io_signature::make(nconnections, nconnections, sizeof(float)),
                   io_signature::make(0, 0, 0)),
	d_size(size), d_buffer_size(2*size), d_samp_rate(samp_rate), d_name(name),
	d_nconnections(nconnections), d_index(0), d_start(0), d_end(size)
    {


      for(int n = 0; n < d_nconnections; n++) {
	d_buffers.push_back((double*)volk_malloc(d_buffer_size*sizeof(double),
                                                 volk_get_alignment()));
	memset(d_buffers[n], 0, d_buffer_size*sizeof(double));

	d_fbuffers.push_back((float*)volk_malloc(d_buffer_size*sizeof(float),
                                                  volk_get_alignment()));
	memset(d_fbuffers[n], 0, d_buffer_size*sizeof(float));

        d_displayOneBuffer = true;
        d_cleanBuffers = true;
      }

      // Set alignment properties for VOLK
      const int alignment_multiple =
	volk_get_alignment() / sizeof(float);
      set_alignment(std::max(1,alignment_multiple));

      d_tags = std::vector< std::vector<gr::tag_t> >(d_nconnections);

      initialize();
      this->plot = plot;

      auto time_plot = dynamic_cast<TimeDomainDisplayPlot *>(plot);
      if (time_plot)
	      time_plot->setSampleRate(samp_rate, 1, "");

      auto freq_plot = dynamic_cast<FftDisplayPlot *>(plot);
      if (freq_plot)
	      freq_plot->setSampleRate(samp_rate, 1, "");

      set_trigger_mode(TRIG_MODE_FREE, 0);
    }

    scope_sink_f_impl::~scope_sink_f_impl()
    {
      for(int n = 0; n < d_nconnections; n++) {
	volk_free(d_buffers[n]);
	volk_free(d_fbuffers[n]);
      }
    }

    bool
    scope_sink_f_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    scope_sink_f_impl::initialize()
    {
      d_qApplication = NULL;
      if(qApp != NULL) {
	d_qApplication = qApp;
      }

      // initialize update time to 10 times a second
      set_update_time(0.1);
    }

    void
    scope_sink_f_impl::exec_()
    {
      d_qApplication->exec();
    }

    void
    scope_sink_f_impl::set_update_time(double t)
    {
      //convert update time to ticks
      gr::high_res_timer_type tps = gr::high_res_timer_tps();
      d_update_time = t * tps;
      d_last_time = 0;
    }

    void
    scope_sink_f_impl::set_trigger_mode(trigger_mode mode, int channel, const std::string &tag_key)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_trigger_mode = mode;
      d_trigger_channel = channel;
      d_trigger_tag_key = pmt::intern(tag_key);
      d_triggered = false;

      _reset();
    }

    void
    scope_sink_f_impl::set_nsamps(const int newsize)
    {
      if(newsize != d_size) {
        gr::thread::scoped_lock lock(d_setlock);

	// Set new size and reset buffer index
	// (throws away any currently held data, but who cares?)
	d_size = newsize;
        d_buffer_size = 2*d_size;

	// Resize buffers and replace data
	for(int n = 0; n < d_nconnections; n++) {
	  volk_free(d_buffers[n]);
	  d_buffers[n] = (double*)volk_malloc(d_buffer_size*sizeof(double),
                                              volk_get_alignment());
	  memset(d_buffers[n], 0, d_buffer_size*sizeof(double));

	  volk_free(d_fbuffers[n]);
	  d_fbuffers[n] = (float*)volk_malloc(d_buffer_size*sizeof(float),
                                               volk_get_alignment());
	  memset(d_fbuffers[n], 0, d_buffer_size*sizeof(float));
	}

        _reset();
      }
    }

    void
    scope_sink_f_impl::set_samp_rate(const double samp_rate)
    {
      gr::thread::scoped_lock(d_setlock);
      d_samp_rate = samp_rate;

      auto time_plot = dynamic_cast<TimeDomainDisplayPlot *>(this->plot);
      if (time_plot)
	      time_plot->setSampleRate(samp_rate, 1, "");
    }

    int
    scope_sink_f_impl::nsamps() const
    {
      return d_size;
    }

    std::string scope_sink_f_impl::name() const
    {
	    return d_name;
    }

    void
    scope_sink_f_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    scope_sink_f_impl::set_displayOneBuffer(bool val)
    {
            d_displayOneBuffer = val;
    }

    void
    scope_sink_f_impl::_reset()
    {
      int n;

      for(n = 0; n < d_nconnections; n++) {
        d_tags[n].clear();
      }

      // Reset the start and end indices.
      d_start = 0;
      d_index = 0;
      d_end = d_size;

      // Reset the trigger. If in free running mode, ignore the
      // trigger delay and always set trigger to true.
      if(d_trigger_mode == TRIG_MODE_FREE) {
	d_index = 0;
	d_triggered = true;
      } else {
	      d_triggered = false;
      }
    }

    void
    scope_sink_f_impl::_test_trigger_tags(int nitems)
    {
      int trigger_index;

      uint64_t nr = nitems_read(d_trigger_channel);
      std::vector<gr::tag_t> tags;
      get_tags_in_range(tags, d_trigger_channel,
			nr, nr + nitems + 1,
			d_trigger_tag_key);
      if(tags.size() > 0) {
	d_triggered = true;
	trigger_index = tags[0].offset - nr;
	d_start = d_index + trigger_index;
	d_end = d_start + d_size;
	_adjust_tags(-d_start);
      }
    }

    void
    scope_sink_f_impl::_npoints_resize()
    {
    }

    void
    scope_sink_f_impl::_adjust_tags(int adj)
    {
      for(size_t n = 0; n < d_tags.size(); n++) {
	for(size_t t = 0; t < d_tags[n].size(); t++) {
	  d_tags[n][t].offset += adj;
	}
      }
    }

    void
    scope_sink_f_impl::clean_buffers()
    {
            gr::thread::scoped_lock lock(d_setlock);

            // Set new size and reset buffer index
            // (throws away any currently held data, but who cares?)
            d_buffer_size = 2*d_size;

            // Resize buffers and replace data
            for(int n = 0; n < d_nconnections; n++) {
                    volk_free(d_buffers[n]);
                    d_buffers[n] = (double*)volk_malloc(d_buffer_size*sizeof(double),
                                                        volk_get_alignment());
                    memset(d_buffers[n], 0, d_buffer_size*sizeof(double));

                    volk_free(d_fbuffers[n]);
                    d_fbuffers[n] = (float*)volk_malloc(d_buffer_size*sizeof(float),
                                                        volk_get_alignment());
                    memset(d_fbuffers[n], 0, d_buffer_size*sizeof(float));
            }

            _reset();
            d_cleanBuffers = true;
    }

    int
    scope_sink_f_impl::work(int noutput_items,
			   gr_vector_const_void_star &input_items,
			   gr_vector_void_star &output_items)
    {
      int n=0, idx=0;
      const float *in;

      _npoints_resize();

      gr::thread::scoped_lock lock(d_setlock);

      if (!d_displayOneBuffer && !d_cleanBuffers) {
              return 0;
      }
      int nfill = d_end - d_index;                 // how much room left in buffers
      int nitems = std::min(noutput_items, nfill); // num items we can put in buffers
      int nItemsToSend = 0;

      // If tag trigger, look for the trigger
      if((d_trigger_mode != TRIG_MODE_FREE) && !d_triggered) {
	// trigger off a tag key (first one found)
	if(d_trigger_mode == TRIG_MODE_TAG) {
	  _test_trigger_tags(nitems);
	}
    }

      // Copy data into the buffers.
      for(n = 0; n < d_nconnections; n++) {
        in = (const float*)input_items[idx];
	memcpy(&d_fbuffers[n][d_index], &in[0], nitems*sizeof(float));
        //volk_32f_convert_64f(&d_buffers[n][d_index],
        //                     &in[1], nitems);

        uint64_t nr = nitems_read(idx);
        std::vector<gr::tag_t> tags;
        get_tags_in_range(tags, idx, nr, nr + nitems + 1);
        for(size_t t = 0; t < tags.size(); t++) {
          tags[t].offset = tags[t].offset - nr + (d_index-d_start-1);
	}
        d_tags[idx].insert(d_tags[idx].end(), tags.begin(), tags.end());
        idx++;
      }
      d_index += nitems;

      // If we've have a full d_size of items in the buffers, plot.
      if((d_end != 0 && !d_displayOneBuffer) ||
                      ((d_triggered) && (d_index == d_end) && d_end != 0 && d_displayOneBuffer)) {
              // Copy data to be plotted to start of buffers.
              for(n = 0; n < d_nconnections; n++) {
                      if (!d_displayOneBuffer) {
                              nItemsToSend = d_index;
                              if (nItemsToSend >= d_size) {
                                      nItemsToSend = d_size;
                                      d_cleanBuffers = false;
                              }
                              volk_32f_convert_64f(d_buffers[n], &d_fbuffers[n][d_start], nItemsToSend);
                      } else {
                              //memmove(d_buffers[n], &d_buffers[n][d_start], d_size*sizeof(double));
                              volk_32f_convert_64f(d_buffers[n], &d_fbuffers[n][d_start], d_size);
                              nItemsToSend = d_size;
                      }
              }

              // Plot if we are able to update
              if((gr::high_res_timer_now() - d_last_time > d_update_time)
                              || !d_cleanBuffers) {
                      d_last_time = gr::high_res_timer_now();
                      if (d_qApplication) {
                              d_qApplication->postEvent(this->plot,
                                                        new IdentifiableTimeUpdateEvent(d_buffers,
                                                                                        nItemsToSend,
                                                                                        d_tags,
                                                                                        d_name));
                      }
              }

              // We've plotting, so reset the state
              if (d_displayOneBuffer) {
                      _reset();
              }
      }

      if (d_displayOneBuffer && d_index == d_end) {
              _reset();
      }
      return nitems;
    }
} /* namespace gr */
