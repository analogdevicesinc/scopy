/* -*- c++ -*- */
/*
 * Copyright 2013 Free Software Foundation, Inc.
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

#ifndef M2K_HISTOGRAM_DISPLAY_PLOT_H
#define M2K_HISTOGRAM_DISPLAY_PLOT_H

#include <stdint.h>
#include <cstdio>
#include <vector>

#include "DisplayPlot.h"
#include "spectrumUpdateEvents.h"

#include <qwt_plot_histogram.h>

namespace adiscope {


class HistogramScaleDraw: public QwtScaleDraw
{
public:
	HistogramScaleDraw();
	QwtText label( double ) const;
	void setColor(const QColor &color);
	void setTotalSamples(unsigned int totalSamples);
private:
	unsigned int m_totalSamples;
	QColor m_color;
};

class Histogram: public QwtPlotHistogram
{
public:
	Histogram(const QString& title, const QColor& color);
	void setColor(const QColor& color);
	void setValues(const double *xData, const double *yData, unsigned int nr_bins);
	void setOrientation(Qt::Orientation orientation);
	int getMaxHeight();

private:
	QVector<QwtIntervalSample> m_samples;

};

/*!
 * \brief QWidget for displaying time domain plots.
 * \ingroup qtgui_blk
 */
class HistogramDisplayPlot: public DisplayPlot
{
  Q_OBJECT

public:
  HistogramDisplayPlot(int nplots, QWidget*);
  virtual ~HistogramDisplayPlot();

  void plotNewData(const std::vector<double*> dataPoints,
		   const int64_t numDataPoints, const double timeInterval);

  void replot();

  void setXaxisSpan(double start, double stop);
  void setDataInterval(int min, int max);
  void setOrientation(Qt::Orientation orientation);
  Qt::Orientation getOrientation();
  bool isZoomed();

public Q_SLOTS:
  void setAutoScale(bool state);
  void setAutoScaleX();
  void setSemilogx(bool en);
  void setSemilogy(bool en);
  void setAccumulate(bool en);

  void setMarkerAlpha(int which, int alpha);
  int getMarkerAlpha(int which) const;
  void setLineColor(int which, QColor color);

  void setNumBins(int bins);
  void setXaxis(double min, double max);

  void customEvent(QEvent * e);

  void enableChannel(unsigned int chIdx, bool enable);
  void setYaxisSpan(unsigned int chIdx, double bot, double top);
    void setSelectedChannel(unsigned int value);

private Q_SLOTS:
  void newData(const QEvent*);

  void _onZoom(const QRectF &rect);
private:
  void _resetXAxisPoints(double left, double right);
  void _autoScaleY(double bottom, double top);
  void _updateXScales(unsigned int totalSamples);
  void _orientationChanged();
  void _resetZoom();

  std::vector<Histogram *> d_histograms;
  std::vector<PlotAxisConfiguration *> rightVertAxes;
  double d_height;
  PrefixFormatter *d_pf;

  double* d_xdata;
  std::vector<double*> d_ydata;

  int d_bins;
  bool d_accum;
  double d_xmin, d_xmax, d_left, d_right;
  double d_width;
  int d_minPos, d_maxPos;

  bool d_semilogx;
  bool d_semilogy;
  bool d_autoscalex_state;
  bool stop;
  bool d_zoomed;

  Qt::Orientation d_orientation;
  std::vector<PlotAxisConfiguration *> horizAxes;
  unsigned int d_selected_channel;
};
} //adiscope

#endif /* M2K_HISTOGRAM_DISPLAY_PLOT_H */
