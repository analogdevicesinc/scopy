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
	class MarkerController;
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
		bool update_ui;
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

		enum MagnitudeType {
			DBFS = 0,
			DBV = 1,
			DBU = 2,
			VPEAK = 3,
			VRMS = 4,
		};

		enum MarkerType {
			MANUAL = 0,
			PEAK = 1,
			DELTA = 2,
			FIXED = 3,
		};

	typedef boost::shared_ptr<SpectrumAverage> average_sptr;

	private:
		double* x_data;
		std::vector<double*> y_data;
		std::vector<double*> y_original_data;

		std::vector<double> y_scale_factor;

		double d_start_frequency;
		double d_stop_frequency;
		double d_sampl_rate;
		double d_preset_sampl_rate;

		MetricPrefixFormatter dBFormatter;
		MetricPrefixFormatter freqFormatter;

		std::vector<enum AverageType> d_ch_average_type;
		std::vector<average_sptr> d_ch_avg_obj;

		enum MagnitudeType d_presetMagType;
		enum MagnitudeType d_magType;

		MarkerController *d_mrkCtrl;
		QList<int> d_num_markers;
		QList<QList<marker>> d_markers;
		QList<QList<std::shared_ptr<struct marker_data>>> d_peaks;
		QList<QList<std::shared_ptr<struct marker_data>>> d_freq_asc_sorted_peaks;
		bool d_emitNewMkrData;

		QList<QColor> d_markerColors;

		void plotData(const std::vector<double *> pts,
				uint64_t num_points);
		void _resetXAxisPoints();

		void resetAverages();
		void averageDataAndComputeMagnitude(std::vector<double *>
			in_data, std::vector<double *> out_data,
			uint64_t nb_points);
		average_sptr getNewAvgObject(enum AverageType avg_type,
			uint data_width, uint history);

		void add_marker(int chn);
		void remove_marker(int chn, int which);
		void marker_set_pos_source(uint chIdx, uint mkIdx,
			std::shared_ptr<struct marker_data> source_sptr);
		void findPeaks(int chn);
		void calculate_fixed_markers(int chn);
		int getMarkerPos(const QList<marker>& marker_list,
			 std::shared_ptr<SpectrumMarker> marker) const;
		void detectMarkers();

	private Q_SLOTS:
		void onMrkCtrlMarkerSelected(std::shared_ptr<SpectrumMarker>);
		void onMrkCtrlMarkerPosChanged(std::shared_ptr<SpectrumMarker>);
		void onMrkCtrlMarkerReleased(std::shared_ptr<SpectrumMarker>);

	public:
		explicit FftDisplayPlot(int nplots, QWidget *parent = nullptr);
		~FftDisplayPlot();

		// Scaling factors for plot samples (one per channel)
		double channelScaleFactor(int chIdx) const;
		void setScaleFactor(int chIdx, double scale);

		int64_t posAtFrequency(double freq) const;
		QString leftVerAxisUnit() const;
		void setLeftVertAxisUnit(const QString& unit);

		enum MagnitudeType magnitudeType() const;
		void setMagnitudeType(enum MagnitudeType);

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

		int markerType(uint chIdx, uint mkIdx) const;

		void marker_to_max_peak(uint chIdx, uint mkIdx);
		void marker_to_next_higher_freq_peak(uint chIdx, uint mkIdx);
		void marker_to_next_lower_freq_peak(uint chIdx, uint mkIdx);
		void marker_to_next_higher_mag_peak(uint chIdx, uint mkIdx);
		void marker_to_next_lower_mag_peak(uint chIdx, uint mkIdx);

		void updateMarkerUi(uint chIdx, uint mkIdx);
		void updateMarkersUi();

		void selectMarker(uint chIdx, uint mkIdx);

		void recalculateMagnitudes();
		void replot();
		void setZoomerEnabled();
		double sampleRate();
		void setNumPoints(uint64_t num_points);

	Q_SIGNALS:
		void newData();
		void sampleRateUpdated(double);
		void sampleCountUpdated(uint);
		void newMarkerData();
		void markerSelected(uint chIdx, uint mkIdx);

	public Q_SLOTS:
		void setSampleRate(double sr, double units,
			const std::string &strunits);
		void presetSampleRate(double sr);
		void customEvent(QEvent *e);
	};
}

#endif /* FFT_DISPLAY_PLOT_H */
