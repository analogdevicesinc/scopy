/* -*- c++ -*- */
/*
 * Copyright 2012,2014-2015 Free Software Foundation, Inc.
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
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
/*
 * Copyright (c) 2022 Analog Devices Inc.
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

#ifndef INCLUDED_QTGUI_WATERFALL_SINK_H
#define INCLUDED_QTGUI_WATERFALL_SINK_H

#ifdef ENABLE_PYTHON
#include <Python.h>
#endif

#include "WaterfallDisplayPlot.h"
#include "scopy-gr-gui_export.h"

#include <gnuradio/filter/firdes.h>
#include <gnuradio/sync_block.h>

#include <qapplication.h>

#include <trigger_mode.h>

namespace scopy {

class SCOPY_GR_GUI_EXPORT waterfall_sink : virtual public gr::sync_block
{
public:
	typedef std::shared_ptr<waterfall_sink> sptr;

	/*!
	 * \brief Build a complex number waterfall sink.
	 *
	 * \param size size of the FFT to compute and display. If using
	 *        the PDU message port to plot samples, the length of
	 *        each PDU must be a multiple of the FFT size.
	 * \param wintype type of window to apply (see gr::fft::window::win_type)
	 * \param fc center frequency of signal (use for x-axis labels)
	 * \param bw bandwidth of signal (used to set x-axis labels)
	 * \param name title for the plot
	 * \param nconnections number of signals to be connected to the
	 *        sink. The PDU message port is always available for a
	 *        connection, and this value must be set to 0 if only
	 *        the PDU message port is being used.
	 * \param parent a QWidget parent object, if any
	 */
	static sptr make(int size, std::vector<float> win, double fc, double bw, const std::string &name,
			 int nconnections = 1, WaterfallDisplayPlot *plot = NULL, bool fft_shift = false);

	virtual void reset() = 0;
	virtual QWidget *qwidget() = 0;

	virtual void clear_data() = 0;

	virtual void set_fft_size(const int fftsize) = 0;
	virtual int fft_size() const = 0;
	virtual void set_time_per_fft(const double t) = 0;
	virtual float fft_average() const = 0;
	virtual void set_fft_window(const std::vector<float> window) = 0;

	virtual void set_frequency_range(const double centerfreq, const double bandwidth) = 0;
	virtual void set_intensity_range(const double min, const double max) = 0;

	virtual void set_update_time(double t) = 0;
	virtual void set_line_label(unsigned int which, const std::string &line) = 0;
	virtual void set_line_alpha(unsigned int which, double alpha) = 0;

	/*!
	 *  Pass "true" to this function to only show the positive half
	 *  of the spectrum. By default, this plotter shows the full
	 *  spectrum (positive and negative halves).
	 */
	virtual void set_plot_pos_half(bool half) = 0;

	virtual double line_alpha(unsigned int which) = 0;

	virtual void set_size(int width, int height) = 0;

	virtual void auto_scale() = 0;
	virtual double min_intensity(unsigned int which) = 0;
	virtual double max_intensity(unsigned int which) = 0;
	virtual void disable_legend() = 0;
	virtual void set_trigger_mode(trigger_mode mode, int channel, const std::string &tag_key = "") = 0;

	QApplication *d_qApplication;
};

} /* namespace scopy */

#endif /* waterfall_sink_H */
