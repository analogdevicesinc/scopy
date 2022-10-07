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

#ifndef WATERFALL_DISPLAY_PLOT_C
#define WATERFALL_DISPLAY_PLOT_C

#include "WaterfallDisplayPlot.h"
#include "spectrumUpdateEvents.h"

#include <gnuradio/qtgui/qtgui_types.h>
#include <qwt_color_map.h>
#include <qwt_legend.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <QColor>
#include <qwt_plot_dict.h>

#if QWT_VERSION < 0x060100
#include <qwt_legend_item.h>
#else /* QWT_VERSION < 0x060100 */
#include <qwt_legend_data.h>
#include <qwt_legend_label.h>
#endif /* QWT_VERSION < 0x060100 */

#include <boost/date_time/posix_time/posix_time.hpp>
using QwtAxis::XBottom;

namespace pt = boost::posix_time;
using namespace adiscope;
#include <QDebug>

class ColorMap_DefaultDark : public QwtLinearColorMap
{
public:
    ColorMap_DefaultDark() : QwtLinearColorMap(Qt::black, QColor(255, 144, 0))
    {
	addColorStop(0.2, Qt::black);
	addColorStop(0.4, Qt::darkCyan);
	addColorStop(0.6, Qt::cyan);
	addColorStop(0.8, QColor(255, 144, 0));
    }
};

/***********************************************************************
 * Text scale widget to provide Y (time) axis text
 **********************************************************************/
class QwtTimeScaleDraw : public QwtScaleDraw, public TimeScaleData
{
public:
	QwtTimeScaleDraw() : QwtScaleDraw(), TimeScaleData() {}

	~QwtTimeScaleDraw() override {}

	QwtText label(double value) const override
	{
		double secs = double(value * getSecondsPerLine());
		return QwtText(QString::number(secs, 'e', 2));
	}

	virtual void initiateUpdate()
	{
		// Do this in one call rather than when zeroTime and secondsPerLine
		// updates is to prevent the display from being updated too often...
		invalidateCache();
	}

protected:
private:
};typedef QPointF QwtDoublePoint;
typedef QRectF QwtDoubleRect;

typedef QwtInterval QwtDoubleInterval;

/***********************************************************************
 * Widget to provide mouse pointer coordinate text
 **********************************************************************/
class WaterfallZoomer : public QwtPlotZoomer,
		public TimeScaleData,
		public FreqOffsetAndPrecisionClass
{
public:
#if QWT_VERSION < 0x060100
	WaterfallZoomer(QwtPlotCanvas* canvas, const unsigned int freqPrecision)
#else  /* QWT_VERSION < 0x060100 */
	WaterfallZoomer(QWidget* canvas, const unsigned int freqPrecision)
#endif /* QWT_VERSION < 0x060100 */
		: QwtPlotZoomer(canvas),
		  TimeScaleData(),
		  FreqOffsetAndPrecisionClass(freqPrecision)
	{
		setTrackerMode(QwtPicker::AlwaysOn);
	}

	~WaterfallZoomer() override {}

	virtual void updateTrackerText() { updateDisplay(); }

	void setUnitType(const std::string& type) { d_unitType = type; }

protected:
	using QwtPlotZoomer::trackerText;
	QwtText trackerText(QPoint const& p) const override
	{
		QwtDoublePoint dp = QwtPlotZoomer::invTransform(p);
		double secs = double(dp.y() * getSecondsPerLine());
		QwtText t(QString("%1 %2, %3 s")
			  .arg(dp.x(), 0, 'f', getFrequencyPrecision())
			  .arg(d_unitType.c_str())
			  .arg(secs, 0, 'e', 2));
		return t;
	}

private:
	std::string d_unitType;
};

/*********************************************************************
 * Main waterfall plot widget
 *********************************************************************/
WaterfallDisplayPlot::WaterfallDisplayPlot(int nplots, QWidget* parent)
	: DisplayPlot(nplots, parent)
{
	d_zoomer.push_back(nullptr); // need this for proper init
	d_start_frequency = -1;
	d_stop_frequency = 1;

	resize(parent->width(), parent->height());
	d_numPoints = 0;
	d_half_freq = false;
	d_legend_enabled = true;
	d_nrows = 200;
	d_color_bar_title_font_size = 18;
	d_time_per_fft = 0;

//	setAxisTitle(QwtAxis::XBottom, "Frequency (Hz)");
	setAxisScaleDraw(QwtAxis::XBottom, new FreqDisplayScaleDraw(0));

//	setAxisTitle(QwtAxis::YLeft, "Time (s)");
	setAxisScaleDraw(QwtAxis::YLeft, new QwtTimeScaleDraw());

	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data.push_back(
					new WaterfallData(d_start_frequency, d_stop_frequency, d_numPoints, d_nrows));

#if QWT_VERSION < 0x060000
		d_spectrogram.push_back(new PlotWaterfall(d_data[i], "Spectrogram"));

#else
		d_spectrogram.push_back(new QwtPlotSpectrogram("Spectrogram"));
		d_spectrogram[i]->setData(d_data[i]);
		d_spectrogram[i]->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
//		d_spectrogram[i]->setColorMap(new ColorMap_MultiColor());
		d_spectrogram[i]->setColorMap(new ColorMap_DefaultDark());
#endif

		// a hack around the fact that we aren't using plot curves for the
		// spectrogram plots.
		d_plot_curve.push_back(new QwtPlotCurve(QString("Data %1").arg(i)));

		d_spectrogram[i]->attach(this);

		d_intensity_color_map_type.push_back(
					INTENSITY_COLOR_MAP_TYPE_MULTI_COLOR);
		setIntensityColorMapType(
					i, d_intensity_color_map_type[i], QColor("white"), QColor("white"));

		setAlpha(i, 255 / d_nplots);
	}

	// Set bottom plot with no transparency as a base
	setAlpha(0, 255);

	// LeftButton for the zooming
	// MiddleButton for the panning
	// RightButton: zoom out by 1
	// Ctrl+RighButton: zoom out to full size
	d_zoomer[0] = new WaterfallZoomer(canvas(), 0);
#if QWT_VERSION < 0x060000
	d_zoomer[0]->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif
	d_zoomer[0]->setMousePattern(
				QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
	d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

	const QColor c(Qt::white);
	d_zoomer[0]->setRubberBandPen(c);
	d_zoomer[0]->setTrackerPen(c);

	_updateIntensityRangeDisplay();

	d_xaxis_multiplier = 1;

	setVisibleSampleCount(150);
	d_grid->enableX(false);
	d_grid->enableY(false);
}

WaterfallDisplayPlot::~WaterfallDisplayPlot() {}

void WaterfallDisplayPlot::setVisibleSampleCount(int count)
{
	d_visible_samples = count;

	setYaxis(0, d_visible_samples);
	setNumRows(d_visible_samples);
}

void WaterfallDisplayPlot::resetAxis()
{
	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data[i]->resizeData(d_start_frequency, d_stop_frequency, d_numPoints, d_nrows);
		d_data[i]->reset();
	}

	setAxisScale(QwtAxis::XBottom, d_start_frequency, d_stop_frequency);

	// Load up the new base zoom settings
	QwtDoubleRect zbase = d_zoomer[0]->zoomBase();
	d_zoomer[0]->zoom(zbase);
	d_zoomer[0]->setZoomBase(zbase);
	d_zoomer[0]->setZoomBase(true);
	d_zoomer[0]->zoom(0);
}

void WaterfallDisplayPlot::autoScale()
{
    double min_int = d_min_val;
    double max_int = d_max_val/1000000;

    qDebug() << d_min_val << d_max_val;

    setIntensityRange(min_int, max_int);
}

void WaterfallDisplayPlot::setCenterFrequency(const double freq)
{
	d_center_frequency = freq;
}

void WaterfallDisplayPlot::setFrequencyRange(const double centerfreq,
					     const double bandwidth,
					     const double units,
					     const std::string& strunits)
{
	double startFreq;
	double stopFreq = (centerfreq + bandwidth / 2.0f) / units;
	if (d_half_freq)
		startFreq = centerfreq / units;
	else
		startFreq = (centerfreq - bandwidth / 2.0f) / units;


	d_xaxis_multiplier = units;

	bool reset = false;
	if ((startFreq != d_start_frequency) || (stopFreq != d_stop_frequency))
		reset = true;

	if (stopFreq > startFreq) {
		d_start_frequency = startFreq;
		d_stop_frequency = stopFreq;
		d_center_frequency = centerfreq / units;

		if ((axisScaleDraw(QwtAxis::XBottom) != NULL) && (d_zoomer[0] != NULL)) {
			double display_units = ceil(log10(units) / 2.0);
			setAxisScaleDraw(QwtAxis::XBottom, new FreqDisplayScaleDraw(display_units));
			setAxisTitle(QwtAxis::XBottom,
				     QString("Frequency (%1)").arg(strunits.c_str()));

			if (reset) {
				resetAxis();
			}

			((WaterfallZoomer*)d_zoomer[0])->setFrequencyPrecision(display_units);
			((WaterfallZoomer*)d_zoomer[0])->setUnitType(strunits);
		}
	}
}


double WaterfallDisplayPlot::getStartFrequency() const { return d_start_frequency; }

double WaterfallDisplayPlot::getStopFrequency() const { return d_stop_frequency; }

void WaterfallDisplayPlot::plotNewData(const std::vector<double*> dataPoints,
				       const int64_t numDataPoints,
				       const double timePerFFT,
				       const gr::high_res_timer_type timestamp,
				       const int droppedFrames)
{
	// Display first half of the plot if d_half_freq is true
	int64_t _npoints_in = d_half_freq ? numDataPoints / 2 : numDataPoints;
	int64_t _in_index = 0;
		_in_index = (getStartFrequency() / getStopFrequency()) * _npoints_in;
//		_in_index = getStartFrequency() * numDataPoints / getStopFrequency();
		_npoints_in -= _in_index;
//		_npoints_in = d_center_frequency * numDataPoints / getStopFrequency();
		qDebug() << "CENTERFREQ:" << _npoints_in << _in_index << d_center_frequency;

	if (!d_stop) {
		if (_npoints_in > 0 && timestamp == 0) {
			d_numPoints = _npoints_in / d_nrows;
			resetAxis();

				for (unsigned int i = 0; i < d_nplots; ++i) {
					d_data[i]->setSpectrumDataBuffer(dataPoints[i]);
					d_data[i]->setNumLinesToUpdate(0);
					d_spectrogram[i]->invalidateCache();
					d_spectrogram[i]->itemChanged();
				}

			QwtTimeScaleDraw* timeScale =
					(QwtTimeScaleDraw*)axisScaleDraw(QwtAxis::YLeft);
			timeScale->setSecondsPerLine(timePerFFT);
			timeScale->setZeroTime(timestamp);
			timeScale->initiateUpdate();

			((WaterfallZoomer*)d_zoomer[0])->setSecondsPerLine(timePerFFT);
			((WaterfallZoomer*)d_zoomer[0])->setZeroTime(timestamp);
			replot();
		}

		else if (_npoints_in > 0) {
			if (_npoints_in != d_numPoints) {
				d_numPoints = _npoints_in;
				resetAxis();

				for (unsigned int i = 0; i < d_nplots; ++i) {
					d_spectrogram[i]->invalidateCache();
					d_spectrogram[i]->itemChanged();
				}

				if (isVisible()) {
					replot();
				}
			}

			QwtTimeScaleDraw* timeScale =
					(QwtTimeScaleDraw*)axisScaleDraw(QwtAxis::YLeft);
			timeScale->setSecondsPerLine(timePerFFT);
			timeScale->setZeroTime(timestamp);

			((WaterfallZoomer*)d_zoomer[0])->setSecondsPerLine(timePerFFT);
			((WaterfallZoomer*)d_zoomer[0])->setZeroTime(timestamp);


			for (auto ch: channel_status.toStdMap()) {
				int i = ch.first;
				if (!ch.second) {
					d_data[i]->reset();
					continue;
				}

				d_data[i]->addFFTData(&(dataPoints[i][_in_index]), _npoints_in, droppedFrames);
				d_data[i]->incrementNumLinesToUpdate();
				d_spectrogram[i]->invalidateCache();
				d_spectrogram[i]->itemChanged();
			}

			replot();
		}
	}
}

void WaterfallDisplayPlot::plotNewData(const double* dataPoints,
				       const int64_t numDataPoints,
				       const double timePerFFT,
				       const gr::high_res_timer_type timestamp,
				       const int droppedFrames)
{
	std::vector<double*> vecDataPoints;
	vecDataPoints.push_back((double*)dataPoints);
	plotNewData(vecDataPoints, numDataPoints, timePerFFT, timestamp, droppedFrames);
}

void WaterfallDisplayPlot::setIntensityRange(const double minIntensity,
					     const double maxIntensity)
{
	for (unsigned int i = 0; i < d_nplots; ++i) {
#if QWT_VERSION < 0x060000
		d_data[i]->setRange(QwtDoubleInterval(minIntensity, maxIntensity));
#else
		d_data[i]->setInterval(Qt::ZAxis, QwtInterval(minIntensity, maxIntensity));
#endif

		Q_EMIT updatedLowerIntensityLevel(minIntensity);
		Q_EMIT updatedUpperIntensityLevel(maxIntensity);

		_updateIntensityRangeDisplay();
	}
}

double WaterfallDisplayPlot::getMinIntensity(unsigned int which) const
{
#if QWT_VERSION < 0x060000
	QwtDoubleInterval r = d_data[which]->range();
#else
	QwtInterval r = d_data[which]->interval(Qt::ZAxis);
#endif

	return r.minValue();
}

double WaterfallDisplayPlot::getMaxIntensity(unsigned int which) const
{
#if QWT_VERSION < 0x060000
	QwtDoubleInterval r = d_data[which]->range();
#else
	QwtInterval r = d_data[which]->interval(Qt::ZAxis);
#endif

	return r.maxValue();
}

int WaterfallDisplayPlot::getColorMapTitleFontSize() const
{
	return d_color_bar_title_font_size;
}

void WaterfallDisplayPlot::setColorMapTitleFontSize(int tfs)
{
	d_color_bar_title_font_size = tfs;
}

void WaterfallDisplayPlot::replot()
{
	QwtTimeScaleDraw* timeScale = (QwtTimeScaleDraw*)axisScaleDraw(QwtAxis::YLeft);
	timeScale->initiateUpdate();

	FreqDisplayScaleDraw* freqScale =
			(FreqDisplayScaleDraw*)axisScaleDraw(QwtAxis::XBottom);
	freqScale->initiateUpdate();

	// Update the time axis display
	if (axisWidget(QwtAxis::YLeft) != NULL) {
		axisWidget(QwtAxis::YLeft)->update();
	}

	// Update the Frequency Offset Display
	if (axisWidget(QwtAxis::XBottom) != NULL) {
		axisWidget(QwtAxis::XBottom)->update();
	}

	if (d_zoomer[0] != NULL) {
		((WaterfallZoomer*)d_zoomer[0])->updateTrackerText();
	}

	QwtPlot::replot();
}

void WaterfallDisplayPlot::clearData()
{
	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data[i]->reset();
	}
}


int WaterfallDisplayPlot::getIntensityColorMapType(unsigned int which) const
{
	return d_intensity_color_map_type[which];
}

void WaterfallDisplayPlot::setIntensityColorMapType(const unsigned int which,
						    const int newType,
						    const QColor lowColor,
						    const QColor highColor)
{
	if ((d_intensity_color_map_type[which] != newType) ||
			((newType == INTENSITY_COLOR_MAP_TYPE_USER_DEFINED) &&
			 (lowColor.isValid() && highColor.isValid()))) {
		switch (newType) {
		case INTENSITY_COLOR_MAP_TYPE_MULTI_COLOR: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_MultiColor colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_MultiColor());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_WHITE_HOT: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_WhiteHot colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_WhiteHot());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_BLACK_HOT: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_BlackHot colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_BlackHot());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_INCANDESCENT: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_Incandescent colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_Incandescent());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_SUNSET: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_Sunset colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_Sunset());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_COOL: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_Cool colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_Cool());
#endif
			break;
		}
		case INTENSITY_COLOR_MAP_TYPE_USER_DEFINED: {
			d_user_defined_low_intensity_color = lowColor;
			d_user_defined_high_intensity_color = highColor;
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_UserDefined colorMap(lowColor, highColor);
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(
						new ColorMap_UserDefined(lowColor, highColor));
#endif
			break;
		}
		default:
			break;
		}

		_updateIntensityRangeDisplay();
	}
}

void WaterfallDisplayPlot::setIntensityColorMapType1(int newType)
{
	setIntensityColorMapType(0,
				 newType,
				 d_user_defined_low_intensity_color,
				 d_user_defined_high_intensity_color);
}

int WaterfallDisplayPlot::getIntensityColorMapType1() const
{
	return getIntensityColorMapType(0);
}

void WaterfallDisplayPlot::setUserDefinedLowIntensityColor(QColor c)
{
	d_user_defined_low_intensity_color = c;
}

const QColor WaterfallDisplayPlot::getUserDefinedLowIntensityColor() const
{
	return d_user_defined_low_intensity_color;
}

void WaterfallDisplayPlot::setUserDefinedHighIntensityColor(QColor c)
{
	d_user_defined_high_intensity_color = c;
}

const QColor WaterfallDisplayPlot::getUserDefinedHighIntensityColor() const
{
	return d_user_defined_high_intensity_color;
}

int WaterfallDisplayPlot::getAlpha(unsigned int which)
{
	return d_spectrogram[which]->alpha();
}

void WaterfallDisplayPlot::setAlpha(unsigned int which, int alpha)
{
	d_spectrogram[which]->setAlpha(alpha);
}

int WaterfallDisplayPlot::getNumRows() const { return d_nrows; }

void WaterfallDisplayPlot::enableChannel(bool en, int id)
{
	if (channel_status.contains(id)) {
		channel_status[id] = en;
	} else {
		channel_status.insert(id, en);
	}
}

void WaterfallDisplayPlot::_updateIntensityRangeDisplay()
{
	QwtScaleWidget* rightAxis = axisWidget(QwtAxis::YRight);
	QwtText colorBarTitle("Intensity (dB)");
	colorBarTitle.setFont(QFont("Arial", d_color_bar_title_font_size));
	rightAxis->setTitle(colorBarTitle);
	rightAxis->setColorBarEnabled(true);

	for (unsigned int i = 0; i < d_nplots; ++i) {
#if QWT_VERSION < 0x060000
		rightAxis->setColorMap(d_spectrogram[i]->data()->range(),
				       d_spectrogram[i]->colorMap());
		setAxisScale(QwtAxis::YRight,
			     d_spectrogram[i]->data()->range().minValue(),
			     d_spectrogram[i]->data()->range().maxValue());
#else
		QwtInterval intv = d_spectrogram[i]->interval(Qt::ZAxis);
		switch (d_intensity_color_map_type[i]) {
		case INTENSITY_COLOR_MAP_TYPE_MULTI_COLOR:
			rightAxis->setColorMap(intv, new ColorMap_MultiColor());
			break;
		case INTENSITY_COLOR_MAP_TYPE_WHITE_HOT:
			rightAxis->setColorMap(intv, new ColorMap_WhiteHot());
			break;
		case INTENSITY_COLOR_MAP_TYPE_BLACK_HOT:
			rightAxis->setColorMap(intv, new ColorMap_BlackHot());
			break;
		case INTENSITY_COLOR_MAP_TYPE_INCANDESCENT:
			rightAxis->setColorMap(intv, new ColorMap_Incandescent());
			break;
		case INTENSITY_COLOR_MAP_TYPE_SUNSET:
			rightAxis->setColorMap(intv, new ColorMap_Sunset());
			break;
		case INTENSITY_COLOR_MAP_TYPE_COOL:
			rightAxis->setColorMap(intv, new ColorMap_Cool());
			break;
		case INTENSITY_COLOR_MAP_TYPE_USER_DEFINED:
			rightAxis->setColorMap(
						intv,
						new ColorMap_UserDefined(d_user_defined_low_intensity_color,
									 d_user_defined_high_intensity_color));
			break;
		default:
			rightAxis->setColorMap(intv, new ColorMap_MultiColor());
			break;
		}
		setAxisScale(QwtAxis::YRight, intv.minValue(), intv.maxValue());
#endif

		plotLayout()->setAlignCanvasToScales(true);

		// Tell the display to redraw everything
		d_spectrogram[i]->invalidateCache();
		d_spectrogram[i]->itemChanged();
	}

	// Draw again
	replot();
}

void WaterfallDisplayPlot::disableLegend()
{
	d_legend_enabled = false;
	//    enableAxis(QwtAxis::YRight, false);
	setAxisAutoScale(QwtAxis::YRight, false);
}

void WaterfallDisplayPlot::enableLegend()
{
	d_legend_enabled = true;
	//    enableAxis(QwtAxis::YRight, true);
	setAxisAutoScale(QwtAxis::YRight, true);
}

void WaterfallDisplayPlot::enableLegend(bool en)
{
	d_legend_enabled = en;
	setAxisAutoScale(QwtAxis::YRight, en);
	//    enableAxis(QwtAxis::YRight, en);
}

void WaterfallDisplayPlot::setNumRows(int nrows) { d_nrows = nrows; }

void WaterfallDisplayPlot::customEvent(QEvent *e)
{
	if (e->type() == WaterfallUpdateEvent::Type()) {
		WaterfallUpdateEvent* event = (WaterfallUpdateEvent*)e;
		const std::vector<double*> dataPoints = event->getPoints();
		const uint64_t numDataPoints = event->getNumDataPoints();
		const gr::high_res_timer_type dataTimestamp = event->getDataTimestamp();

		for (auto ch: channel_status.toStdMap()) {
			int i = ch.first;
				const double* min_val =
						std::min_element(&dataPoints[i][0], &dataPoints[i][numDataPoints - 1]);
				const double* max_val =
						std::max_element(&dataPoints[i][0], &dataPoints[i][numDataPoints - 1]);
					if (*min_val < d_min_val || i == 0)
						d_min_val = *min_val;
					if (*max_val > d_max_val || i == 0)
						d_max_val = *max_val;
		}
		autoScale();

		plotNewData(dataPoints, numDataPoints, d_time_per_fft, dataTimestamp, 0);
	}
}

void WaterfallDisplayPlot::setPlotPosHalf(bool half)
{
	d_half_freq = half;
	if (half)
		d_start_frequency = d_center_frequency;
}


#endif /* WATERFALL_DISPLAY_PLOT_C */
