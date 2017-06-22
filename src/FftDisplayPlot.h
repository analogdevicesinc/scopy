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
#include <boost/shared_ptr.hpp>

namespace adiscope {
	class SpectrumAverage;
	class SpectrumMarker;
}

namespace adiscope {

	struct marker_data {
		float x;
		float y;
		int bin;
		bool active;
		QString label;
	};

	class FftDisplayPlot : public DisplayPlot
	{
		Q_OBJECT

	public:
		enum AverageType {
			SAMPLE = 0,
			PEAK_HOLD = 1,
			PEAK_HOLD_CONTINUOUS = 2,
			MIN_HOLD = 3,
			MIN_HOLD_CONTINUOUS = 4,
			LINEAR_RMS = 5,
			LINEAR_DB = 6,
			EXPONENTIAL_RMS = 7,
			EXPONENTIAL_DB = 8,
		};

	typedef boost::shared_ptr<SpectrumAverage> average_sptr;

	private:
		double* x_data;
		std::vector<double*> y_data;

		double d_start_frequency;
		double d_stop_frequency;

		MetricPrefixFormatter dBFormatter;
		MetricPrefixFormatter freqFormatter;

		std::vector<enum AverageType> d_ch_average_type;
		std::vector<average_sptr> d_ch_avg_obj;

		QList<int> d_num_markers;
		QList<QList<struct marker_data>> d_markers;
		QList<QList<SpectrumMarker *>> d_gui_markers;

		void plotData(const std::vector<double *> pts,
				uint64_t num_points);
		void _resetXAxisPoints();

		average_sptr getNewAvgObject(enum AverageType avg_type,
			uint data_width, uint history);

		void add_marker(int chn);
		void remove_marker(int chn, int which);

	public:
		explicit FftDisplayPlot(int nplots, QWidget *parent = nullptr);
		~FftDisplayPlot();

		QString leftVerAxisUnit() const;
		void setLeftVertAxisUnit(const QString& unit);
		enum AverageType averageType(uint chIdx) const;
		uint averageHistory(uint chIdx) const;
		void setAverage(uint chIdx, enum AverageType avg_type,
			uint history);
		void resetAverageHistory();

		uint peakCount(uint chIdx) const;
		void setPeakCount(uint chIdx, uint count);
		bool isPeakVisible(uint chIdx, uint peakIdx) const;
		void setPeakVisible(uint chnIdx, uint peakIdx, bool on);
		void findPeaks(int chn);

		void replot();

	public Q_SLOTS:
		void setSampleRate(double sr, double units,
			const std::string &strunits);
		void customEvent(QEvent *e);
	};
}

#endif /* FFT_DISPLAY_PLOT_H */
