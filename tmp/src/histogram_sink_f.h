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

#ifndef M2K_HISTOGRAM_SINK_F_H
#define M2K_HISTOGRAM_SINK_F_H

#include <gnuradio/sync_block.h>
#include <qapplication.h>

namespace adiscope {

    /*!
     * \brief A graphical sink to display a histogram.
     * \ingroup instrumentation_blk
     * \ingroup qtgui_blk
     *
     * \details
     * This is a QT-based graphical sink the displays a histogram of
     * the data.
     *
     * This histogram allows you to set and change at runtime the
     * number of points to plot at once and the number of bins in the
     * histogram. Both x and y-axis have their own auto-scaling
     * behavior. By default, auto-scaling the y-axis is turned on and
     * continuously updates the y-axis max value based on the
     * currently plotted histogram.
     *
     * The x-axis auto-scaling function only updates once when
     * clicked. This resets the x-axis to the current range of minimum
     * and maximum values represented in the histogram. It resets any
     * values currently displayed because the location and width of
     * the bins may have changed.
     *
     * The histogram also has an accumulate function that simply
     * accumulates the data between calls to work. When accumulate is
     * activated, the y-axis autoscaling is turned on by default as
     * the values will quickly grow in the this direction.
     */
    class histogram_sink_f : virtual public gr::sync_block
    {
    public:
      // adiscope::histogram_sink_f::sptr
      typedef std::shared_ptr<histogram_sink_f> sptr;

      /*!
       * \brief Build floating point histogram sink
       *
       * \param size number of points to plot at once
       * \param bins number of bins to sort the data into
       * \param xmin minimum x-axis value
       * \param xmax maximum x-axis value
       * \param name title for the plot
       * \param nconnections number of signals connected to sink
       * \param parent a QWidget parent object, if any
       */
      static sptr make(int size, int bins,
                       double xmin, double xmax,
		       const std::string &name,
		       int nconnections=1,
		       QObject *plot=NULL);

      virtual void exec_() = 0;

    public:

      virtual int nsamps() const = 0;
      virtual int bins() const = 0;
      virtual void reset() = 0;

      QApplication *d_qApplication;

      virtual void set_update_time(double t) = 0;
      virtual void set_nsamps(const int newsize) = 0;
      virtual void set_bins(const int bins) = 0;
    };

} /* namespace adiscope */

#endif /* M2K_HISTOGRAM_SINK_F_H */
