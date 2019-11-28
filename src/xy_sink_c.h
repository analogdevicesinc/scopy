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

#ifndef M2K_XY_SINK_C_H
#define M2K_XY_SINK_C_H

#ifdef ENABLE_PYTHON
#include <Python.h>
#endif

#include <gnuradio/sync_block.h>
#include <qapplication.h>
#include <gnuradio/filter/firdes.h>

namespace adiscope {

    class xy_sink_c : virtual public gr::sync_block
    {
    public:
      // gr::qtgui::xy_sink_c::sptr
      typedef boost::shared_ptr<xy_sink_c> sptr;

      static sptr make(int size,
		       const std::string &name,
		       int nconnections=1,
		       QObject *plot=NULL);

      virtual void exec_() = 0;

      virtual void set_update_time(double t) = 0;
      virtual void set_nsamps(const int newsize) = 0;

      virtual int nsamps() const = 0;
      virtual void reset() = 0;

      QApplication *d_qApplication;
    };

} /* namespace adiscope */

#endif /* M2K_XY_SINK_C_H */
