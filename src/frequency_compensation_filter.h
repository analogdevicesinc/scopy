/* -*- c++ -*- */
/*
 * Copyright 2019 Analog Devices Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_BLOCKS_OVERSHOOT_FILTER_H
#define INCLUDED_BLOCKS_OVERSHOOT_FILTER_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/sync_block.h>

namespace adiscope {

    class frequency_compensation_filter : virtual public gr::sync_block
    {
    public:
      // gr::blocks::streams_to_vector::sptr
      typedef boost::shared_ptr<frequency_compensation_filter> sptr;

      /*!
       * Make a stream-to-vector block.
       *
       * \param itemsize the item size of the stream
       * \param nstreams number of streams to combine into a vector (vector size)
       */
      static sptr make(bool enable = true, float TC = 1, float gain = -1,
		       float sample_rate = 100000000);
      virtual void set_enable(bool en) = 0;
      virtual bool get_enable() = 0;
      virtual void set_TC(float TC) = 0;
      virtual float get_TC() = 0;
      virtual void set_gain(float gain) = 0;
      virtual float get_gain() = 0;
      virtual void set_sample_rate(float sample_rate) = 0;
      virtual bool get_high_gain() = 0;
      virtual void set_high_gain(bool en) = 0;
    };

} /* namespace adiscope */
#endif /* INCLUDED_BLOCKS_STREAMS_TO_SHORT_H */
