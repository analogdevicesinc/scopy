/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
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

#ifndef WATERFALL_DISPLAY_PLOT_H
#define WATERFALL_DISPLAY_PLOT_H

#include <gnuradio/high_res_timer.h>
#include "DisplayPlot.h"
//#include "spectrum_analyzer.hpp"
#include "waterfallGlobalData.h"
#include <qwt_plot_spectrogram.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <qwt_interval.h>
#include "scopygui_export.h"

#if QWT_VERSION < 0x060000
#include <gnuradio/qtgui/plot_waterfall.h>
#else
#include <QwtLinearColorMap>
#include <qwt_interval.h>

typedef QPointF QwtDoublePoint;
typedef QRectF QwtDoubleRect;
typedef QwtInterval QwtDoubleInterval;

#endif

class SCOPYGUI_EXPORT ColorMap_DefaultDark : public QwtLinearColorMap
{
public:
	ColorMap_DefaultDark() : QwtLinearColorMap(Qt::black, Qt::white)
	{
		addColorStop(0.16, Qt::black);
		addColorStop(0.33, QColor(58, 36, 59)); // deep purple
		addColorStop(0.5, QColor(74, 100, 255)); // scopy blue
		addColorStop(0.66, QColor(255, 144, 0)); // scopy orange
		addColorStop(0.83, Qt::white);
	}
};
enum {
    INTENSITY_COLOR_MAP_TYPE_DEFAULT_DARK = 7
};

/*!
 * \brief QWidget for displaying waterfall (spectrogram) plots.
 * \ingroup qtgui_blk
 */
namespace scopy {
class SCOPYGUI_EXPORT WaterfallDisplayPlot : public DisplayPlot
{
	friend class SpectrumAnalyzer_API;
	Q_OBJECT

	Q_PROPERTY(int intensity_color_map_type1 READ getIntensityColorMapType1 WRITE
		   setIntensityColorMapType1)
	Q_PROPERTY(QColor low_intensity_color READ getUserDefinedLowIntensityColor WRITE
		   setUserDefinedLowIntensityColor)
	Q_PROPERTY(QColor high_intensity_color READ getUserDefinedHighIntensityColor WRITE
		   setUserDefinedHighIntensityColor)
	Q_PROPERTY(int color_map_title_font_size READ getColorMapTitleFontSize WRITE
		   setColorMapTitleFontSize)


public:
	WaterfallDisplayPlot(int nplots, QWidget*);
	~WaterfallDisplayPlot() override;

	void resetAxis(bool resetData = true);

	void setFrequencyRange(const double,
			       const double,
			       const double units = 1000.0,
			       const std::string& strunits = "kHz");
	double getStartFrequency() const;
	double getStopFrequency() const;

	void plotNewData(const std::vector<double*> dataPoints,
			 const int64_t numDataPoints,
			 gr::high_res_timer_type acquisitionTime,
			 const int droppedFrames);

	void setIntensityRange(double minIntensity, double maxIntensity);
	double getMinIntensity(unsigned int which) const;
	double getMaxIntensity(unsigned int which) const;

	void replot(void) override;
	void clearData();

	int getIntensityColorMapType(unsigned int) const;
	int getIntensityColorMapType1() const;
	int getColorMapTitleFontSize() const;
	const QColor getUserDefinedLowIntensityColor() const;
	const QColor getUserDefinedHighIntensityColor() const;

	int getAlpha(unsigned int which);
	void setAlpha(unsigned int which, int alpha);

	int getNumRows() const;

	void setVisibleSampleCount(int count);
	void autoScale();

	void setCenterFrequency(const double freq);
	void setUpdateTime(double t);
	void setFlowDirection(WaterfallFlowDirection direction);
	WaterfallFlowDirection getFlowDirection() const;
	int getEnabledChannelID();

	double getResolutionBW() const;
	void setResolutionBW(double values);
	void enableYaxisLabels();
	void enableXaxisLabels();
	QString formatXValue(double value, int precision) const;

	void setBtmHorAxisUnit(const QString &unit);
	void setLeftVertAxisUnit(const QString &unit);
	QString formatYValue(double value, int precision) const;
	void updateHandleAreaPadding();

	bool eventFilter(QObject *, QEvent *);
	void setupReadouts();
	void updateCursorsData();

public Q_SLOTS:
	void
	setIntensityColorMapType(const unsigned int, const int, const QColor, const QColor);
	void setIntensityColorMapType1(int);
	void setColorMapTitleFontSize(int tfs);
	void setUserDefinedLowIntensityColor(QColor);
	void setUserDefinedHighIntensityColor(QColor);
	void setPlotPosHalf(bool half);
	void disableLegend() override;
	void enableLegend();
	void enableLegend(bool en);
	void setNumRows(int nrows);
	void customEvent(QEvent *e);
	void enableChannel(int id);
	void resetAvgAcquisitionTime();

private Q_SLOTS:
	void onHCursor1Moved(double value);
	void onHCursor2Moved(double value);
	void onVCursor1Moved(double value);
	void onVCursor2Moved(double value);

Q_SIGNALS:
	void updatedLowerIntensityLevel(const double);
	void updatedUpperIntensityLevel(const double);
	void newWaterfallData();

private:
	void _updateIntensityRangeDisplay();

	double d_start_frequency;
	double d_stop_frequency;
	double d_center_frequency;
	int d_xaxis_multiplier;
	bool d_half_freq;
	bool d_legend_enabled;
	int d_nrows;
	int enabledChannelID;
	int d_visible_samples;

	double d_min_val;
	double d_max_val;
	double d_time_per_fft;
	int d_intensity_offset;
	double d_avg_acquisition_time;
	int d_visible_line_count;
	double d_resolution_bw;
	double d_last_draw_time;
	double d_center_plot_time;

	std::vector<WaterfallData*> d_data;
	TimePrefixFormatter d_TimeFormatter;
	MetricPrefixFormatter freqFormatter;
	PrefixFormatter *d_formatter;

#if QWT_VERSION < 0x060000
	std::vector<PlotWaterfall*> d_spectrogram;
#else
	std::vector<QwtPlotSpectrogram*> d_spectrogram;
#endif

	std::vector<int> d_intensity_color_map_type;
	QColor d_user_defined_low_intensity_color;
	QColor d_user_defined_high_intensity_color;
	int d_color_bar_title_font_size;
};
}

#endif /* WATERFALL_DISPLAY_PLOT_H */
