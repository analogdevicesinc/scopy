/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FFT_DISPLAY_PLOT_H
#define FFT_DISPLAY_PLOT_H

#include "DisplayPlot.h"

namespace adiscope {
	class FftDisplayPlot : public DisplayPlot
	{
		Q_OBJECT

	private:
		double* x_data;
		std::vector<double*> y_data;

		double d_start_frequency;
		double d_stop_frequency;

		MetricPrefixFormatter dBFormatter;
		MetricPrefixFormatter freqFormatter;

		void plotData(const std::vector<double *> pts,
				uint64_t num_points);
		void _resetXAxisPoints();

	public:
		explicit FftDisplayPlot(int nplots, QWidget *parent = nullptr);
		~FftDisplayPlot();

		void replot();

	public slots:
		void setSampleRate(double sr, double units,
			const std::string &strunits);
		void customEvent(QEvent *e);
	};
}

#endif /* FFT_DISPLAY_PLOT_H */
