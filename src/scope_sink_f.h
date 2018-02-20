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

#ifndef M2K_SCOPE_SINK_F_H
#define M2K_SCOPE_SINK_F_H

#ifdef ENABLE_PYTHON
#include <Python.h>
#endif

#include "trigger_mode.h"
#include <gnuradio/sync_block.h>
#include <qapplication.h>

namespace adiscope {

    class scope_sink_f : virtual public gr::sync_block
    {
    public:
      // adiscope::scope_sink_f::sptr
      typedef boost::shared_ptr<scope_sink_f> sptr;

      static sptr make(int size, double samp_rate,
		       const std::string &name,
		       int nconnections=1,
		       QObject *plot=NULL);

      virtual void exec_() = 0;

      virtual void set_update_time(double t) = 0;
      virtual void set_nsamps(const int newsize) = 0;
      virtual void set_samp_rate(const double samp_rate) = 0;

      virtual void set_trigger_mode(trigger_mode mode, int channel,
				    const std::string &tag_key="") = 0;

      virtual int nsamps() const = 0;
      virtual std::string name() const = 0;
      virtual void reset() = 0;
      virtual void set_displayOneBuffer(bool) = 0;
      virtual void clean_buffers() = 0;

      QApplication *d_qApplication;
    };

} /* namespace adiscope */

#endif /* M2K_SCOPE_SINK_F_H */
