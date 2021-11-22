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


#ifndef CONSTELLATION_DISPLAY_PLOT_C
#define CONSTELLATION_DISPLAY_PLOT_C

#include <qwt_scale_draw.h>
#include <qwt_legend.h>
#include <QColor>
#include <iostream>

#include "ConstellationDisplayPlot.h"

using namespace adiscope;

class ConstellationDisplayZoomer: public QwtPlotZoomer
{
public:
#if QWT_VERSION < 0x060100
  ConstellationDisplayZoomer(QwtPlotCanvas* canvas)
#else /* QWT_VERSION < 0x060100 */
  ConstellationDisplayZoomer(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
    : QwtPlotZoomer(canvas)
  {
    setTrackerMode(QwtPicker::AlwaysOn);
  }

  virtual ~ConstellationDisplayZoomer(){

  }

  virtual void updateTrackerText(){
    updateDisplay();
  }

protected:
  using QwtPlotZoomer::trackerText;
  virtual QwtText trackerText( const QPoint& p ) const
  {
    QPointF dp = QwtPlotZoomer::invTransform(p);
    QwtText t(QString("(%1, %2)").arg(dp.x(), 0, 'f', 4).
	      arg(dp.y(), 0, 'f', 4));
    return t;
  }
};

ConstellationDisplayPlot::ConstellationDisplayPlot(int nplots, QWidget* parent)
  : DisplayPlot(nplots, parent)
{
  resize(parent->width(), parent->height());

  d_numPoints = 1024;
  d_pen_size = 5;

  d_zoomer.push_back(new ConstellationDisplayZoomer(canvas()));

#if QWT_VERSION < 0x060000
  d_zoomer[0]->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

  d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect2,
                            Qt::RightButton, Qt::ControlModifier);
  d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect3,
                            Qt::RightButton);

  const QColor c(Qt::darkRed);
  d_zoomer[0]->setRubberBandPen(c);
  d_zoomer[0]->setTrackerPen(c);

//  setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
//  set_xaxis(-2.0, 2.0);
//  setAxisTitle(QwtPlot::xBottom, "In-phase");

//  setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
//  set_yaxis(-2.0, 2.0);
//  setAxisTitle(QwtPlot::yLeft, "Quadrature");
  updateAxes();

  // Setup dataPoints and plot vectors
  // Automatically deleted when parent is deleted
  for(int i = 0; i < d_nplots; i++) {
    d_real_data.push_back(new double[d_numPoints]);
    d_imag_data.push_back(new double[d_numPoints]);
    memset(d_real_data[i], 0x0, d_numPoints*sizeof(double));
    memset(d_imag_data[i], 0x0, d_numPoints*sizeof(double));

    d_plot_curve.push_back(new QwtPlotCurve(QString("Data %1").arg(i)));
    d_plot_curve[i]->attach(this);
    d_plot_curve[i]->setPen(QPen(d_CurveColors[i]));

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol,
				      QBrush(d_CurveColors[i]),
                                      QPen(d_CurveColors[i]), QSize(7,7));

#if QWT_VERSION < 0x060000
    d_plot_curve[i]->setRawData(d_real_data[i], d_imag_data[i], d_numPoints);
    d_plot_curve[i]->setSymbol(*symbol);
#else
    d_plot_curve[i]->setRawSamples(d_real_data[i], d_imag_data[i], d_numPoints);
    d_plot_curve[i]->setSymbol(symbol);
#endif

    setLineStyle(i, Qt::NoPen);
    setLineMarker(i, QwtSymbol::Ellipse);
  }
}

ConstellationDisplayPlot::~ConstellationDisplayPlot()
{
  for(int i = 0; i < d_nplots; i++) {
    delete [] d_real_data[i];
    delete [] d_imag_data[i];
  }

  // d_plot_curves deleted when parent deleted
  // d_zoomer and d_panner deleted when parent deleted
}

void
ConstellationDisplayPlot::set_xaxis(double min, double max)
{
  setXaxis(min, max);
}

void
ConstellationDisplayPlot::set_yaxis(double min, double max)
{
  setYaxis(min, max);
}

void
ConstellationDisplayPlot::set_axis(double xmin, double xmax,
				   double ymin, double ymax)
{
  set_xaxis(xmin, xmax);
  set_yaxis(ymin, ymax);
}

void
ConstellationDisplayPlot::set_pen_size(int size)
{
  if(size > 0 && size < 30){
    d_pen_size = size;
    for(int i = 0; i < d_nplots; i++) {
      d_plot_curve[i]->setPen(QPen(Qt::blue, d_pen_size, Qt::SolidLine,
                                   Qt::RoundCap, Qt::RoundJoin));
    }
  }
}

void
ConstellationDisplayPlot::replot()
{
  BasicPlot::replot();
}


void
ConstellationDisplayPlot::plotNewData(const std::vector<double*> &realDataPoints,
				      const std::vector<double*> &imagDataPoints,
				      const int64_t numDataPoints,
				      const double timeInterval)
{
  if(!d_stop) {
    if((numDataPoints > 0)) {
      if(numDataPoints != d_numPoints) {
	d_numPoints = numDataPoints;

	for(int i = 0; i < d_nplots; i++) {
	  delete [] d_real_data[i];
	  delete [] d_imag_data[i];
	  d_real_data[i] = new double[d_numPoints];
	  d_imag_data[i] = new double[d_numPoints];

#if QWT_VERSION < 0x060000
	  d_plot_curve[i]->setRawData(d_real_data[i], d_imag_data[i], d_numPoints);
#else
	  d_plot_curve[i]->setRawSamples(d_real_data[i], d_imag_data[i], d_numPoints);
#endif
	}
      }

      for(int i = 0; i < d_nplots; i++) {
	memcpy(d_real_data[i], realDataPoints[i], numDataPoints*sizeof(double));
	memcpy(d_imag_data[i], imagDataPoints[i], numDataPoints*sizeof(double));
      }

      if(d_autoscale_state) {
	double bottom=1e20, top=-1e20;
	for(int n = 0; n < d_nplots; n++) {
	  for(int64_t point = 0; point < numDataPoints; point++) {
            double b = std::min(realDataPoints[n][point], imagDataPoints[n][point]);
            double t = std::max(realDataPoints[n][point], imagDataPoints[n][point]);
	    if(b < bottom) {
	      bottom = b;
	    }
	    if(t > top) {
	      top = t;
	    }
	  }
	}
	_autoScale(bottom, top);
      }

      replot();

    }
  }
}

void
ConstellationDisplayPlot::plotNewData(const double* realDataPoints,
				      const double* imagDataPoints,
				      const int64_t numDataPoints,
				      const double timeInterval)
{
  std::vector<double*> vecRealDataPoints;
  std::vector<double*> vecImagDataPoints;
  vecRealDataPoints.push_back((double*)realDataPoints);
  vecImagDataPoints.push_back((double*)imagDataPoints);
  plotNewData(vecRealDataPoints, vecImagDataPoints,
	      numDataPoints, timeInterval);
}

void
ConstellationDisplayPlot::newData(const QEvent* updateEvent)
{
  ConstUpdateEvent *tevent = (ConstUpdateEvent*)updateEvent;
  const std::vector<double*> realDataPoints = tevent->getRealPoints();
  const std::vector<double*> imagDataPoints = tevent->getImagPoints();
  const uint64_t numDataPoints = tevent->getNumDataPoints();

  this->plotNewData(realDataPoints,
			 imagDataPoints,
			 numDataPoints,
			 0);
}

void
ConstellationDisplayPlot::customEvent(QEvent * e)
{
  if(e->type() == ConstUpdateEvent::Type()) {
    newData(e);
  }
}

void
ConstellationDisplayPlot::_autoScale(double bottom, double top)
{
  // Auto scale the x- and y-axis with a margin of 20%
  double b = bottom - fabs(bottom)*0.20;
  double t = top + fabs(top)*0.20;
  set_axis(b, t, b, t);
}

void
ConstellationDisplayPlot::setAutoScale(bool state)
{
  d_autoscale_state = state;
}

#endif /* CONSTELLATION_DISPLAY_PLOT_C */
