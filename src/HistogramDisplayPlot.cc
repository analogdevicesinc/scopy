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

#ifndef HISTOGRAM_DISPLAY_PLOT_C
#define HISTOGRAM_DISPLAY_PLOT_C

#include <qwt_scale_draw.h>
#include <qwt_legend.h>
#include <QColor>
#include <cmath>
#include <iostream>
#include <volk/volk.h>
#include <gnuradio/math.h>
#include <boost/math/special_functions/round.hpp>
#include <QLocale>
#include <QDebug>
#include "HistogramDisplayPlot.h"

#ifdef _MSC_VER
#define copysign _copysign
#endif

using namespace adiscope;

class TimePrecisionClass
{
public:
  TimePrecisionClass(const int timeprecision)
  {
    d_time_precision = timeprecision;
  }

  virtual ~TimePrecisionClass()
  {
  }

  virtual unsigned int getTimePrecision() const
  {
    return d_time_precision;
  }

  virtual void setTimePrecision(const unsigned int newprecision)
  {
    d_time_precision = newprecision;
  }
protected:
  unsigned int d_time_precision;
};


class HistogramDisplayZoomer: public LimitedPlotZoomer, public TimePrecisionClass
{
public:
#if QWT_VERSION < 0x060100
  HistogramDisplayZoomer(QwtPlotCanvas* canvas, const unsigned int timeprecision)
#else /* QWT_VERSION < 0x060100 */
  HistogramDisplayZoomer(QWidget* canvas, const unsigned int timeprecision,
			QColor c)
#endif /* QWT_VERSION < 0x060100 */
    : LimitedPlotZoomer(canvas),TimePrecisionClass(timeprecision)
  {
    setTrackerMode(QwtPicker::AlwaysOff);
#if QWT_VERSION < 0x060000
    setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

    setMousePattern(QwtEventPattern::MouseSelect2,
			    Qt::RightButton, Qt::ControlModifier);
    setMousePattern(QwtEventPattern::MouseSelect3,
			    Qt::RightButton);

    setRubberBandPen(QColor("#999999"));
    setTrackerPen(c);

    setEnabled(false);

  }

  virtual ~HistogramDisplayZoomer()
  {
  }

  virtual void updateTrackerText()
  {
    updateDisplay();
  }

  void setUnitType(const std::string &type)
  {
    d_unit_type = type;
  }

protected:
  using QwtPlotZoomer::trackerText;
  virtual QwtText trackerText(const QPoint& p) const
  {
    QwtText t;
    QPointF dp = QwtPlotZoomer::invTransform(p);
    if((dp.y() > 0.0001) && (dp.y() < 10000)) {
      t.setText(QString("%1 V, %2 pts").
		arg(dp.x(), 0, 'f', 2).
		arg(dp.y(), 0, 'f', 0));
    }
    else {
      t.setText(QString("%1 V, %2 pts").
		arg(dp.x(), 0, 'f', 4).
		arg(0));
    }

    t.setBackgroundBrush(QBrush(QColor(0, 0, 0 ,155)));
    return t;
  }

private:
  std::string d_unit_type;
};



Histogram::Histogram(const QString& title, const QColor& color)
	: QwtPlotHistogram(title)
{
	setStyle(QwtPlotHistogram::Columns);
	setPen(color, 0.1);
	setColor(color);
}

void
Histogram::setColor(const QColor& color)
{
	QColor c = color;
	c.setAlpha(255 / 2);
	setBrush(QBrush(c));
}

void
Histogram::setValues(const double *xData, const double *yData, unsigned int nr_bins)
{
	QVector<QwtIntervalSample> samples(nr_bins - 1);
	if (m_samples.size() != samples.size()) {
		m_samples.clear();
		m_samples.resize(samples.size());
	}
	for (unsigned int i = 0; i < nr_bins - 1; ++i) {
		QwtInterval interval(xData[i], xData[i + 1]);
		interval.setBorderFlags(QwtInterval::IncludeBorders);

		int sign = orientation() == Qt::Horizontal ? -1 : 1;
		samples[i] = QwtIntervalSample(sign * yData[i], interval);
	}

	m_samples = samples;
	setData(new QwtIntervalSeriesData(samples));
}

void Histogram::setOrientation(Qt::Orientation orientation)
{
	if (this->orientation() == orientation) {
		return;
	}

	QwtPlotSeriesItem::setOrientation(orientation);
	for (int i = 0; i < m_samples.size(); ++i) {
		m_samples[i].value = (-1) * m_samples[i].value;
	}
	setData(new QwtIntervalSeriesData(m_samples));
}

int Histogram::getMaxHeight()
{
	double max_h = 0;
	int sign = orientation() == Qt::Horizontal ? -1 : 1;
	for (int i = 0; i < m_samples.size(); ++i) {
		max_h = std::max(max_h, sign * m_samples[i].value);
	}
	return max_h;
}

/***********************************************************************
 * Main Time domain plotter widget
 **********************************************************************/
HistogramDisplayPlot::HistogramDisplayPlot(int nplots, QWidget* parent)
  : DisplayPlot(nplots, parent)
{
  d_bins = 100;
  d_accum = false;
  d_height = 0;
  stop = false;
  d_orientation = Qt::Horizontal;
  d_minPos = 0;
  d_maxPos = 0;
  d_zoomed = false;
  d_xmin = 1e20;
  d_xmax = -1e20;

  setLeftVertAxesCount(2);

  // Initialize x-axis data array
  d_xdata = new double[d_bins];
  memset(d_xdata, 0x0, d_bins*sizeof(double));

  d_semilogx = false;
  d_semilogy = false;
  d_autoscale_state = false;
  d_autoscalex_state = false;

  setAxesCount(QwtPlot::xBottom, 2);
  horizAxes.resize(2);
  d_pf = new MetricPrefixFormatter();
  d_pf->setTwoDecimalMode(true);
  for (int i = 0; i < 2; ++i) {
	  horizAxes[i] = new PlotAxisConfiguration(QwtPlot::xBottom, i, this);
	  configureAxis(QwtPlot::xBottom, i);
	  setAxisVisible(QwtAxisId(QwtPlot::xBottom, i),
			 d_usingLeftAxisScales);
	  connect(axisWidget(horizAxes[i]->axis()), SIGNAL(scaleDivChanged()),
			this, SLOT(_onXbottomAxisWidgetScaleDivChanged()));
	  QwtScaleWidget *scaleWidget = axisWidget(QwtAxisId(QwtPlot::xBottom, i));
	  OscScaleDraw* osd = dynamic_cast<OscScaleDraw*>(scaleWidget->scaleDraw());
	  osd->setFormatter(d_pf);
	  osd->setUnitType("V");
	  osd->setFloatPrecision(2);
	  osd->setColor(d_CurveColors[i]);
	  const int fmw = QFontMetrics(scaleWidget->font()).width("-XX.XX XX");
	  scaleWidget->setMinBorderDist(fmw / 2, fmw / 2);
  }
  for (int i = 0; i < 2; ++i) {
	  HistogramScaleDraw *hsd = new HistogramScaleDraw();
	  hsd->setColor(d_CurveColors[i]);
	  setAxisScaleDraw(QwtAxisId(QwtPlot::yLeft, i), hsd);
  }

  this->_updateXScales(100);

  // Setup dataPoints and plot vectors
  // Automatically deleted when parent is deleted
  for(int i = 0; i < d_nplots; i++) {
    d_ydata.push_back(new double[d_bins]);
    memset(d_ydata[i], 0, d_bins*sizeof(double));

    d_histograms.push_back(new Histogram(QString("Data %1").arg(i), d_CurveColors[i]));
    d_histograms[i]->setXAxis(QwtAxisId(QwtPlot::xBottom, i));
    d_histograms[i]->setYAxis(QwtAxisId(QwtPlot::yLeft, i));
	d_histograms[i]->setOrientation(Qt::Horizontal);
    d_histograms[i]->attach(this);


    d_plot_curve.push_back(new QwtPlotCurve(QString("Data %1").arg(i)));
   // d_plot_curve[i]->attach(this);
    d_plot_curve[i]->setPen(QPen(d_CurveColors[i]));
    d_plot_curve[i]->setRenderHint(QwtPlotItem::RenderAntialiased);

    // Adjust color's transparency for the brush
    d_CurveColors[i].setAlpha(127 / d_nplots);
    d_plot_curve[i]->setBrush(QBrush(d_CurveColors[i]));

    d_CurveColors[i].setAlpha(255 / d_nplots);
    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol,
				      QBrush(d_CurveColors[i]),
				      QPen(d_CurveColors[i]), QSize(7,7));

#if QWT_VERSION < 0x060000
    d_plot_curve[i]->setRawData(d_xdata, d_ydata[i], d_bins);
    d_plot_curve[i]->setSymbol(*symbol);
#else
    d_plot_curve[i]->setRawSamples(d_xdata, d_ydata[i], d_bins);
    d_plot_curve[i]->setSymbol(symbol);
#endif
  }

  for (unsigned int i = 0; i < d_histograms.size(); ++i) {
	  d_zoomer.push_back(new HistogramDisplayZoomer(canvas(), 0,
							getLineColor(i)));
	  d_zoomer[i]->setAxes(QwtAxisId(QwtPlot::xBottom, i),
			       QwtAxisId(QwtPlot::yLeft, i));

	  connect(d_zoomer[i], SIGNAL(zoomed(QRectF)),
		  this, SLOT(_onZoom(QRectF)));
  }


  _resetXAxisPoints(-1, 1);

  for (unsigned int i = 0; i < d_histograms.size(); ++i) {
	  setAxisScale(QwtAxisId(QwtPlot::xBottom, i), 0, 100);
	  setAxisScale(QwtAxisId(QwtPlot::yLeft, i), -10, 10);
  }

  setAxisVisible(QwtAxisId(QwtPlot::yLeft, 0), false);
  setAxisVisible(QwtAxisId(QwtPlot::yLeft, 1), false);
  setAxisVisible(QwtAxisId(QwtPlot::xBottom, 0), false);
  setAxisVisible(QwtAxisId(QwtPlot::xBottom, 1), false);

  insertLegend(nullptr);

  d_grid->detach();
  for(QwtPlotScaleItem* scale : qAsConst(scaleItems)){
	  scale->detach();
  }
}

void HistogramDisplayPlot::_onZoom(const QRectF &rect)
{
	QwtPlotZoomer *zoomer = static_cast<QwtPlotZoomer*>(sender());
	d_zoomed = (rect != zoomer->zoomBase());
}

void HistogramDisplayPlot::setOrientation(Qt::Orientation orientation)
{
	d_orientation = orientation;

	for (int i = 0; i < d_histograms.size(); ++i) {
		d_histograms[i]->setOrientation(orientation);
	}

	if (orientation == Qt::Vertical) {
		d_grid->attach(this);
		for(QwtPlotScaleItem* scale : qAsConst(scaleItems)){
			scale->attach(this);
		}
	} else {
		d_grid->detach();
		for(QwtPlotScaleItem* scale : qAsConst(scaleItems)){
			scale->detach();
		}
	}

	QwtInterval xInt = axisInterval(QwtPlot::xBottom);
	QwtInterval yInt = axisInterval(QwtPlot::yLeft);
	QwtInterval xInt2 = axisInterval(QwtAxisId(QwtPlot::xBottom, 1));
	QwtInterval yInt2 = axisInterval(QwtAxisId(QwtPlot::yLeft, 1));


	if (orientation == Qt::Vertical) {
		setAxisScale(QwtAxisId(QwtPlot::xBottom, 0), yInt.minValue(), yInt.maxValue());
		setAxisScale(QwtAxisId(QwtPlot::yLeft, 0), xInt.maxValue(), -xInt.minValue());
		setAxisScale(QwtAxisId(QwtPlot::xBottom, 1), yInt2.minValue(), yInt2.maxValue());
		setAxisScale(QwtAxisId(QwtPlot::yLeft, 1), xInt2.maxValue(), -xInt2.minValue());

		setAxisVisible(QwtAxisId(QwtPlot::yLeft, d_selected_channel), true);
		setAxisVisible(QwtAxisId(QwtPlot::xBottom, d_selected_channel), true);
	} else {
		setAxisScale(QwtPlot::xBottom, 0, yInt.maxValue());
		setAxisScale(QwtAxisId(QwtPlot::yLeft, 0), xInt.minValue(), xInt.maxValue());
		setAxisScale(QwtAxisId(QwtPlot::yLeft, 1), xInt.minValue(), xInt.maxValue());

		setAxisVisible(QwtAxisId(QwtPlot::yLeft, 0), false);
		setAxisVisible(QwtAxisId(QwtPlot::yLeft, 1), false);
		setAxisVisible(QwtAxisId(QwtPlot::xBottom, 0), false);
		setAxisVisible(QwtAxisId(QwtPlot::xBottom, 1), false);
	}

	_orientationChanged();
	replot();
}

Qt::Orientation HistogramDisplayPlot::getOrientation()
{
	return d_orientation;
}

bool HistogramDisplayPlot::isZoomed()
{
	return d_zoomed;
}

HistogramDisplayPlot::~HistogramDisplayPlot()
{
	for(int i = 0; i < d_nplots; i++) {
		delete[] d_ydata[i];
		delete horizAxes[i];
		delete d_histograms[i];
	}

	delete d_pf;
	delete[] d_xdata;

  // d_zoomer and _panner deleted when parent deleted
}

void
HistogramDisplayPlot::enableChannel(unsigned int chIdx, bool enable)
{
	if (chIdx < 0 || chIdx > 1) {
		return;
	}

	if (enable) {
		d_histograms[chIdx]->attach(this);
	} else {
		d_histograms[chIdx]->detach();
	}
	replot();
}

void HistogramDisplayPlot::setYaxisSpan(unsigned int chIdx, double bot, double top)
{
	if (chIdx < 0 || chIdx > 1) {
		return;
	}

	if (d_orientation == Qt::Vertical) {
		setAxisScale(QwtAxisId(QwtPlot::xBottom, chIdx), bot, top);
		return;
	}

	setAxisScale(QwtAxisId(QwtPlot::yLeft, chIdx), bot, top);

}

void HistogramDisplayPlot::setXaxisSpan(double start, double stop)
{
	if (d_orientation == Qt::Vertical) {
		return;
	}

	for (int i = 0; i < d_histograms.size(); ++i) {
		setAxisScale(QwtAxisId(QwtPlot::xBottom, i), start, stop);
	}
}

void HistogramDisplayPlot::setDataInterval(int min, int max)
{
	d_minPos = min;
	d_maxPos = max;
}

void
HistogramDisplayPlot::replot()
{
  BasicPlot::replot();
}

void
HistogramDisplayPlot::plotNewData(const std::vector<double*> dataPoints,
				   const int64_t numDataPoints,
				   const double timeInterval)
{
  if(!d_stop) {
    if((numDataPoints > 0)) {

	    _updateXScales(numDataPoints);
	    if (d_minPos < 0) {
		    d_minPos = 0;
	    }
	    if (d_maxPos > numDataPoints || d_maxPos == 0) {
		    d_maxPos = numDataPoints;
	    }
	    if (d_minPos > d_maxPos) {
		    d_minPos = 0;
		    d_maxPos = numDataPoints;
	    }

      // keep track of the min/max values for when autoscaleX is called.
      double xminTemp = 1e20;
      double xmaxTemp = -1e20;
      for(int n = 0; n < d_nplots; n++) {
	xminTemp = std::min(xminTemp, *std::min_element(dataPoints[n] + d_minPos, dataPoints[n]+d_maxPos));
	xmaxTemp = std::max(xmaxTemp, *std::max_element(dataPoints[n] + d_minPos, dataPoints[n]+d_maxPos));
      }

      const double EPS = 0.1;
      if (std::abs(xminTemp - d_xmin) > EPS ||
	  std::abs(xmaxTemp - d_xmax) > EPS) {
	  if (d_minPos == 0 && d_maxPos == numDataPoints) {
		d_autoscalex_state = true;
	  }
      }
      d_xmin = xminTemp;
      d_xmax = xmaxTemp;

      // If autoscalex has been clicked, clear the data for the new
      // bin widths and reset the x-axis.
      if(d_autoscalex_state) {
        for(int n = 0; n < d_nplots; n++)
	  memset(d_ydata[n], 0, d_bins*sizeof(double));
        _resetXAxisPoints(d_xmin, d_xmax);
        d_autoscalex_state = false;
      }

      int index;
      for(int n = 0; n < d_nplots; n++) {
        if(!d_accum)
	  memset(d_ydata[n], 0, d_bins*sizeof(double));
	for(int64_t point = d_minPos; point < d_maxPos; point++) {
          index = boost::math::iround(1e-20 + (dataPoints[n][point] - d_left)/d_width);
          if((index >= 0) && (index < d_bins))
	    d_ydata[n][index] += 1;
        }
	d_histograms[n]->setValues(d_xdata, d_ydata[n], d_bins);
      }

      double height = 0;
      double histogramHeights[d_nplots];
      for(int n = 0; n < d_nplots; n++) {
		histogramHeights[n] = *std::max_element(d_ydata[n], d_ydata[n]+d_bins);
      }
      for (int n = 0; n < d_nplots - 1; n++) {
	      if (histogramHeights[n] != 0) {
		  height = histogramHeights[n];
		  break;
	      }
      }
      for (int n = 0; n < d_nplots; n++) {
	      if (d_histograms[n]->plot()) {
		  height = std::min(height, histogramHeights[n]);
	      }
      }

      d_height = height;

      if (d_orientation == Qt::Vertical) {
	      for (int i = 0; i < d_histograms.size(); ++i) {
		      double h = histogramHeights[i] + (0.2 * histogramHeights[i]);
		      if (h > numDataPoints) {
			      h = numDataPoints;
		      }
		      double pr = h * 0.15;

		      if (abs(h - axisInterval(QwtAxisId(QwtPlot::yLeft, i)).maxValue()) > pr) {
			setAxisScale(QwtAxisId(QwtPlot::yLeft, i), 0, h);
		      }
	      }
      }

      if(d_autoscale_state) {
	_autoScaleY(0, height);
      }

	setXaxisSpan(-d_height, 0);

	replot();

    }
  }
}

void
HistogramDisplayPlot::newData(const QEvent* updateEvent)
{
  HistogramUpdateEvent *hevent = (HistogramUpdateEvent*)updateEvent;
  const std::vector<double*> dataPoints = hevent->getDataPoints();
  const uint64_t numDataPoints = hevent->getNumDataPoints();

  plotNewData(dataPoints,
		 numDataPoints,
		 0);
}

void
HistogramDisplayPlot::customEvent(QEvent * e)
{
  if(e->type() == HistogramUpdateEvent::Type()) {
    newData(e);
  }
}

void
HistogramDisplayPlot::setXaxis(double min, double max)
{
  _resetXAxisPoints(min, max);
}

void
HistogramDisplayPlot::_resetXAxisPoints(double left, double right)
{
	// Something's wrong with the data (NaN, Inf, or something else)
	if((left == 0 && right == 0) || (left > right))
	{
		// assume some default values
		d_left = -0.01;
		d_right = 0.01;
		qDebug() << "Using default values for histogram";
		// throw std::runtime_error("HistogramDisplayPlot::_resetXAxisPoints left and/or right values are invalid");
	}
	else
	{
		d_left  = left *(1 - copysign(0.1, left));
		d_right = right*(1 + copysign(0.1, right));
	}

	// when both left & right are 0

  d_width = (d_right - d_left)/(d_bins);
  for(long loc = 0; loc < d_bins; loc++){
    d_xdata[loc] = d_left + loc*d_width;
  }
#if QWT_VERSION < 0x060100
  axisScaleDiv(QwtPlot::xBottom)->setInterval(d_left, d_right);
#else /* QWT_VERSION < 0x060100 */
  QwtScaleDiv scalediv(d_left, d_right);
  setAxisScaleDiv(QwtPlot::xBottom, scalediv);
#endif /* QWT_VERSION < 0x060100 */

  // Set up zoomer base for maximum unzoom x-axis
  // and reset to maximum unzoom level
  QRectF zbase = d_zoomer[0]->zoomBase();

  if(d_semilogx) {
    setAxisScale(QwtPlot::xBottom, 1e-1, d_right);
    zbase.setLeft(1e-1);
  }
  else {
    setAxisScale(QwtPlot::xBottom, d_left, d_right);
    zbase.setLeft(d_left);
  }

  zbase.setRight(d_right);
//  d_zoomer[0]->zoom(zbase);
//  d_zoomer[0]->setZoomBase(zbase);
//  d_zoomer[0]->zoom(0);
}

void
HistogramDisplayPlot::_autoScaleY(double bottom, double top)
{
  // Auto scale the y-axis with a margin of 20% (10 dB for log scale)
  double b = bottom - fabs(bottom)*0.20;
  double t = top + fabs(top)*0.20;
  if(d_semilogy) {
    if(bottom > 0) {
      setYaxis(b-10, t+10);
    }
    else {
      setYaxis(1e-3, t+10);
    }
  }
  else {
    setYaxis(b, t);
  }
}

void HistogramDisplayPlot::_updateXScales(unsigned int totalSamples)
{
	for (int i = 0; i < d_histograms.size(); ++i) {
		HistogramScaleDraw *hsd = dynamic_cast<HistogramScaleDraw*>(
					axisWidget(QwtAxisId(QwtPlot::yLeft, i))->scaleDraw());
		if (hsd) {
			hsd->setTotalSamples(totalSamples);
			hsd->invalidateCache();
		}
	}
}

void HistogramDisplayPlot::_orientationChanged()
{
	if (d_orientation == Qt::Horizontal) {
		int min_h = d_histograms[0]->getMaxHeight();
		for (int i = 1; i < d_histograms.size(); ++i) {
			min_h = std::min(min_h, d_histograms[i]->getMaxHeight());
		}
		for (int i = 0; i < d_histograms.size(); ++i) {
			setAxisScale(QwtAxisId(QwtPlot::xBottom, i), -min_h, 0);
		}

	} else {
		//big histogram
		for (int i = 0; i < d_histograms.size(); ++i) {
			int h = d_histograms[i]->getMaxHeight();
			h += h * 0.2;
			setAxisScale(QwtAxisId(QwtPlot::yLeft, i), 0, h);
		}
	}

	for (int i = 0; i < d_zoomer.size(); ++i) {
		d_zoomer[i]->setEnabled(d_orientation == Qt::Horizontal
					? false : true);
	}
}

void HistogramDisplayPlot::_resetZoom()
{
	for (int i = 0; i < d_zoomer.size(); ++i) {
		static_cast<LimitedPlotZoomer*>(d_zoomer[i])->resetZoom();
	}
}

void HistogramDisplayPlot::setSelectedChannel(unsigned int value)
{
	if (value >= d_histograms.size()) {
		return;
	}

	for (int i = 0; i < d_zoomer.size(); ++i) {
		d_zoomer[i]->setTrackerMode(
				(i == value) ? QwtPicker::AlwaysOn : QwtPicker::AlwaysOff);
	}

	d_selected_channel = value;

	d_histograms[value]->detach();
	d_histograms[value]->attach(this);

	if (d_orientation == Qt::Horizontal) {
		return;
	}

	for (int i = 0; i < d_histograms.size(); ++i) {
		setAxisVisible(QwtAxisId(QwtPlot::yLeft, i), value == i);
		setAxisVisible(QwtAxisId(QwtPlot::xBottom, i), value == i);
	}
}

void
HistogramDisplayPlot::setAutoScaleX()
{
	if(d_orientation == Qt::Horizontal) {
		d_autoscalex_state = true;
	}
}

void
HistogramDisplayPlot::setAutoScale(bool state)
{
	d_autoscale_state = state;
}

void
HistogramDisplayPlot::setSemilogx(bool en)
{
  d_semilogx = en;
  if(!d_semilogx) {
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
  }
  else {
#if QWT_VERSION < 0x060100
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#else /* QWT_VERSION < 0x060100 */
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
#endif /* QWT_VERSION < 0x060100 */
  }
}

void
HistogramDisplayPlot::setSemilogy(bool en)
{
  if(d_semilogy != en) {
    d_semilogy = en;

#if QWT_VERSION < 0x060100
    double max = axisScaleDiv(QwtPlot::yLeft)->upperBound();
#else /* QWT_VERSION < 0x060100 */
    double max = axisScaleDiv(QwtPlot::yLeft).upperBound();
#endif /* QWT_VERSION < 0x060100 */

    if(!d_semilogy) {
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
      setYaxis(-pow(10.0, max/10.0), pow(10.0, max/10.0));
    }
    else {
#if QWT_VERSION < 0x060100
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#else /* QWT_VERSION < 0x060100 */
    setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
#endif /* QWT_VERSION < 0x060100 */
      setYaxis(1e-10, 10.0*log10(100*max));
    }
  }
}

void
HistogramDisplayPlot::setAccumulate(bool state)
{
  d_accum = state;
}

void
HistogramDisplayPlot::setMarkerAlpha(int which, int alpha)
{
  if(which < d_nplots) {
    // Get the pen color
    QPen pen(d_plot_curve[which]->pen());
    QBrush brush(d_plot_curve[which]->brush());
    QColor color = brush.color();

    // Set new alpha and update pen
    color.setAlpha(alpha);
    brush.setColor(color);
    color.setAlpha(std::min(255, static_cast<int>(alpha*1.5)));
    pen.setColor(color);
    d_plot_curve[which]->setBrush(brush);
    d_plot_curve[which]->setPen(pen);

    // And set the new color for the markers
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    setLineMarker(which, sym.style());
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setColor(color);
      sym->setPen(pen);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

int
HistogramDisplayPlot::getMarkerAlpha(int which) const
{
  if(which < d_nplots) {
    return d_plot_curve[which]->brush().color().alpha();
  }
  else {
    return 0;
  }
}

void
HistogramDisplayPlot::setLineColor(int which, QColor color)
{
  if(which < d_nplots) {
    // Adjust color's transparency for the brush
    color.setAlpha(127 / d_nplots);

    QBrush brush(d_plot_curve[which]->brush());
    brush.setColor(color);
    d_plot_curve[which]->setBrush(brush);

    // Adjust color's transparency darker for the pen and markers
    color.setAlpha(255 / d_nplots);

    QPen pen(d_plot_curve[which]->pen());
    pen.setColor(color);
    d_plot_curve[which]->setPen(pen);

#if QWT_VERSION < 0x060000
    d_plot_curve[which]->setPen(pen);
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    setLineMarker(which, sym.style());
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setColor(color);
      sym->setPen(pen);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

void
HistogramDisplayPlot::setNumBins(int bins)
{
  d_bins = bins;

  delete [] d_xdata;
  d_xdata = new double[d_bins];
  _resetXAxisPoints(d_left, d_right);

  for(int i = 0; i < d_nplots; i++) {
    delete [] d_ydata[i];
    d_ydata[i] = new double[d_bins];
    memset(d_ydata[i], 0, d_bins*sizeof(double));

#if QWT_VERSION < 0x060000
    d_plot_curve[i]->setRawData(d_xdata, d_ydata[i], d_bins);
#else
    d_plot_curve[i]->setRawSamples(d_ydata[i], d_xdata, d_bins);
#endif
  }
}

#endif /* HISTOGRAM_DISPLAY_PLOT_C */

HistogramScaleDraw::HistogramScaleDraw() :
	QwtScaleDraw(),
	m_color(Qt::gray),
	m_totalSamples(0)
{
	enableComponent(QwtAbstractScaleDraw::Backbone, false);
	enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

QwtText HistogramScaleDraw::label(double value) const
{
	QString prefix = "%";

	double displayValue = 0;

	if (!m_totalSamples) {
		QwtText text("--");
		text.setColor(m_color);
		return text;
	}

	displayValue = value * 100 / m_totalSamples;

	QwtText text(QLocale().toString(displayValue, 'f', 2) + prefix);
	if (m_color != Qt::gray) {
		text.setColor(m_color);
	}

	return text;
}

void HistogramScaleDraw::setColor(const QColor &color)
{
	m_color = color;
}

void HistogramScaleDraw::setTotalSamples(unsigned int totalSamples)
{
	m_totalSamples = totalSamples;
}
