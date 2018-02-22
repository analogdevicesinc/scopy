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

#ifndef M2K_TIME_DOMAIN_DISPLAY_PLOT_H
#define M2K_TIME_DOMAIN_DISPLAY_PLOT_H

#include <stdint.h>
#include <cstdio>
#include <vector>
#include <gnuradio/tags.h>

#include "DisplayPlot.h"
#include "spectrumUpdateEvents.h"

namespace adiscope {

class Sink{
public:
	Sink(std::string name, unsigned int numChannels, unsigned long long channelsDataLength):
		d_name(name), d_numChannels(numChannels), d_channelsDataLength(channelsDataLength) { }

	std::string name() { return d_name; }
	unsigned int numChannels() { return d_numChannels; }
	unsigned int channelsDataLength() { return d_channelsDataLength; }

	void setChannelsDataLength(unsigned long long num) {  d_channelsDataLength = num; }

private:
	std::string d_name;
	unsigned int d_numChannels;
	unsigned long long d_channelsDataLength;
};

class SinkManager
{
public:
	SinkManager();
	~SinkManager();

	bool addSink(const std::string& name, unsigned int numChannels, unsigned long long channelsDataLength);
	bool removeSink(const std::string& name);
	unsigned int sinkListLength();
	Sink* sink(unsigned int index);
	int indexOfSink(const std::string& name);
	int sinkFirstChannelPos(const std::string& name);

private:

	std::vector<Sink> d_sinkList;
};

/*!
 * \brief QWidget for displaying time domain plots.
 * \ingroup qtgui_blk
 */
class TimeDomainDisplayPlot: public DisplayPlot
{
  Q_OBJECT

  Q_PROPERTY ( QColor tag_text_color READ getTagTextColor WRITE setTagTextColor )
  Q_PROPERTY ( QColor tag_background_color READ getTagBackgroundColor WRITE setTagBackgroundColor )
  Q_PROPERTY ( Qt::BrushStyle tag_background_style READ getTagBackgroundStyle WRITE setTagBackgroundStyle )

public:
  TimeDomainDisplayPlot(QWidget*, unsigned int xNumDivs = 10, unsigned int yNumDivs = 10);
  virtual ~TimeDomainDisplayPlot();

  void plotNewData(const std::string sender,
		   const std::vector<double*> dataPoints,
		   const int64_t numDataPoints, const double timeInterval,
                   const std::vector< std::vector<gr::tag_t> > &tags \
		   = std::vector< std::vector<gr::tag_t> >());
  void replot();

  void stemPlot(bool en);

  double sampleRate() const;

  void setYaxisUnit(QString unitType);
  QString yAxisUnit(void);

  const QColor getTagTextColor();
  const QColor getTagBackgroundColor();
  const Qt::BrushStyle getTagBackgroundStyle();

  QString timeScaleValueFormat(double value, int precision) const;
  QString timeScaleValueFormat(double value);
  QString yAxisScaleValueFormat(double value, int precision) const;
  QString yAxisScaleValueFormat(double value);

  bool registerSink(std::string sinkUniqueNme, unsigned int numChannels,
	unsigned long long channelsDataLength, bool curvesAttached = true);
  bool unregisterSink(std::string sinkName);

  long dataStartingPoint() const;

  void addZoomer(unsigned int zoomerIdx);
  void removeZoomer(unsigned int zoomerIdx);
  void setXAxisNumPoints(unsigned int);

Q_SIGNALS:
  void channelAdded(int);
  void newData();
  void filledScreen(bool, unsigned int);

public Q_SLOTS:
  void setSampleRate(double sr, double units,
		     const std::string &strunits);

  void setAutoScale(bool state);
  void setAutoScaleShot();
  void setSemilogx(bool en);
  void setSemilogy(bool en);

  void setLineWidthF(int which, qreal widthF);
  qreal getLineWidthF(int which) const;

  void legendEntryChecked(QwtPlotItem *plotItem, bool on);
  void legendEntryChecked(const QVariant &plotItem, bool on, int index);

  void enableTagMarker(int which, bool en);

  void setYLabel(const std::string &label,
                 const std::string &unit="");

  void attachTriggerLines(bool en);
  void setTriggerLines(double x, double y);

  void setTagTextColor(QColor c);
  void setTagBackgroundColor(QColor c);
  void setTagBackgroundStyle(Qt::BrushStyle b);

  void setZoomerEnabled(bool en);
  bool isZoomerEnabled();
  void setZoomerVertAxis(int index);

  void customEvent(QEvent * e);

  void cancelZoom();

  void setDataStartingPoint(long pos);
  void resetXaxisOnNextReceivedData();
  void hideCurvesUntilNewData();

protected:
  virtual void configureAxis(int axisPos, int axisIdx);
  virtual void cleanUpJustBeforeChannelRemoval(int chnIdx);

private Q_SLOTS:
  void newData(const QEvent*);

protected:
  std::vector<double*> d_ydata;
  std::vector<double*> d_xdata;

private:
  void _resetXAxisPoints(double*& xAxis, unsigned long long numPoints, double sampleRate);
  void _autoScale(double bottom, double top);

  double d_sample_rate;
  double d_delay;
  long d_data_starting_point;
  std::vector<bool> d_sink_reset_x_axis_pts;

  bool d_semilogx;
  bool d_semilogy;
  bool d_autoscale_shot;

  std::vector< std::vector<QwtPlotMarker*> > d_tag_markers;
  std::vector<bool> d_tag_markers_en;

  MetricPrefixFormatter d_metricFormatter;
  TimePrefixFormatter d_timeFormatter;

  QColor d_tag_text_color;
  QColor d_tag_background_color;
  Qt::BrushStyle d_tag_background_style;

  QwtPlotMarker *d_trigger_lines[2];

  SinkManager d_sinkManager;

  unsigned int d_nbPtsXAxis;
  bool d_curves_hidden;

  QColor getChannelColor();
};
} //adiscope

#endif /* M2K_TIME_DOMAIN_DISPLAY_PLOT_H */
