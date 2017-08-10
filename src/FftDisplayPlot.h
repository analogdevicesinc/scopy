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

	struct marker {
		std::shared_ptr<struct marker_data> data;
		std::shared_ptr<SpectrumMarker> ui;
	};

	struct marker_data {
		int type;
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
		QList<QList<marker>> d_markers;
		QList<QList<std::shared_ptr<struct marker_data>>> d_peaks;
		QList<QList<std::shared_ptr<struct marker_data>>> d_freq_asc_sorted_peaks;
		bool d_emitNewMkrData;

		void plotData(const std::vector<double *> pts,
				uint64_t num_points);
		void _resetXAxisPoints();

		average_sptr getNewAvgObject(enum AverageType avg_type,
			uint data_width, uint history);

		void add_marker(int chn);
		void remove_marker(int chn, int which);
		void marker_set_pos_source(uint chIdx, uint mkIdx,
			std::shared_ptr<struct marker_data> source_sptr);
		void findPeaks(int chn);
		void calculate_fixed_markers(int chn);

	public:
		explicit FftDisplayPlot(int nplots, QWidget *parent = nullptr);
		~FftDisplayPlot();

		int64_t posAtFrequency(double freq) const;
		QString leftVerAxisUnit() const;
		void setLeftVertAxisUnit(const QString& unit);
		enum AverageType averageType(uint chIdx) const;
		uint averageHistory(uint chIdx) const;
		void setAverage(uint chIdx, enum AverageType avg_type,
			uint history);
		void resetAverageHistory();

		// Markers
		uint peakCount(uint chIdx) const;
		void setPeakCount(uint chIdx, uint count);

		uint markerCount(uint chIdx) const;
		void setMarkerCount(uint chIdx, uint count);

		bool markerEnabled(uint chIdx, uint mkIdx) const;
		void setMarkerEnabled(uint chIdx, uint mkIdx, bool en);

		double markerFrequency(uint chIdx, uint mkIdx) const;
		double markerMagnutide(uint chIdx, uint mkIdx) const;

		void setMarkerAtFreq(uint chIdx, uint mkIdx, double pos);

		void marker_to_max_peak(uint chIdx, uint mkIdx);
		void marker_to_next_higher_freq_peak(uint chIdx, uint mkIdx);
		void marker_to_next_lower_freq_peak(uint chIdx, uint mkIdx);
		void marker_to_next_higher_mag_peak(uint chIdx, uint mkIdx);
		void marker_to_next_lower_mag_peak(uint chIdx, uint mkIdx);

		void updateMarkerUi(uint chIdx, uint mkIdx);
		void updateMarkersUi();

		void replot();

	Q_SIGNALS:
		void newMarkerData();

	public Q_SLOTS:
		void setSampleRate(double sr, double units,
			const std::string &strunits);
		void customEvent(QEvent *e);
	};
}

#endif /* FFT_DISPLAY_PLOT_H */
