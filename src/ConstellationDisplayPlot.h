/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
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

#ifndef M2K_CONSTELLATION_DISPLAY_PLOT_H
#define M2K_CONSTELLATION_DISPLAY_PLOT_H

#include "DisplayPlot.h"
#include "spectrumUpdateEvents.h"

#include <cstdio>
#include <stdint.h>
#include <vector>

namespace adiscope {
/*!
 * \brief QWidget for displaying constellaton (I&Q) plots.
 * \ingroup qtgui_blk
 */
class ConstellationDisplayPlot : public DisplayPlot {
	Q_OBJECT

public:
	ConstellationDisplayPlot(int nplots, QWidget *);
	virtual ~ConstellationDisplayPlot();

	void plotNewData(const std::vector<double *> &realDataPoints,
			 const std::vector<double *> &imagDataPoints,
			 const int64_t numDataPoints,
			 const double timeInterval);

	// Old method to be removed
	void plotNewData(const double *realDataPoints,
			 const double *imagDataPoints,
			 const int64_t numDataPoints,
			 const double timeInterval);

	void replot();

	void set_xaxis(double min, double max);
	void set_yaxis(double min, double max);
	void set_axis(double xmin, double xmax, double ymin, double ymax);
	void set_pen_size(int size);

public Q_SLOTS:
	void setAutoScale(bool state);

	void customEvent(QEvent *e);

private Q_SLOTS:
	void newData(const QEvent *);

private:
	void _autoScale(double bottom, double top);

	std::vector<double *> d_real_data;
	std::vector<double *> d_imag_data;

	int64_t d_pen_size;
};
} // namespace adiscope

#endif /* M2K_CONSTELLATION_DISPLAY_PLOT_H */
