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
#include "osc_scale_engine.h"
#include "spectrumUpdateEvents.h"
#include "qtgui_types.h"

#include <qwt_color_map.h>
#include <qwt_legend.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <QColor>
#include <qwt_plot_dict.h>

#if QWT_VERSION < 0x060100
#include <qwt_legend_item.h>
#else /* QWT_VERSION < 0x060100 */
#include <QStack>
#include <qwt_legend_data.h>
#include <qwt_legend_label.h>
#endif /* QWT_VERSION < 0x060100 */

#include <boost/date_time/posix_time/posix_time.hpp>

#define ERROR_VALUE -10000000

using QwtAxis::XBottom;

namespace pt = boost::posix_time;
using namespace adiscope;
#include <QDebug>

/***********************************************************************
 * Widget to provide mouse pointer coordinate text
 **********************************************************************/
class WaterfallZoomer : public LimitedPlotZoomer,
		public TimeScaleData
{
public:
#if QWT_VERSION < 0x060100
	WaterfallZoomer(QwtPlotCanvas* canvas, const unsigned int freqPrecision)
#else  /* QWT_VERSION < 0x060100 */
	WaterfallZoomer(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
		: LimitedPlotZoomer(canvas)
	{
		setTrackerMode(QwtPicker::AlwaysOn);
	}

	~WaterfallZoomer() override {}

	virtual void updateTrackerText() { updateDisplay(); }

	void setUnitType(const std::string& type) { d_unitType = type; }

	void setCenterTime(double value) { centerTime = value; }

protected:
	using QwtPlotZoomer::trackerText;
	QwtText trackerText(QPoint const& p) const override
	{
		QwtDoublePoint dp = QwtPlotZoomer::invTransform(p);
		double secs = double(centerTime + dp.y() * getSecondsPerLine());
		QwtText t(QString("(%1 %2, %3 s)")
			  .arg(dp.x(), 0, 'f', 4)
			  .arg(d_unitType.c_str())
			  .arg(secs, 0, 'f', 4));
		return t;
	}

private:
	double centerTime = 0;
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
	enabledChannelID = 0;
	resize(parent->width(), parent->height());
	d_numPoints = 0;
	d_half_freq = false;
	d_legend_enabled = true;
	d_nrows = 200;
	d_color_bar_title_font_size = 18;
	d_time_per_fft = 0;
	d_formatter = static_cast<PrefixFormatter *>(new MetricPrefixFormatter);
	resetAvgAcquisitionTime();

	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data.push_back(
					new WaterfallData(d_start_frequency, d_stop_frequency, d_numPoints, d_nrows));

#if QWT_VERSION < 0x060000
		d_spectrogram.push_back(new PlotWaterfall(d_data[i]	waterfall_plot->setIntensityRange(bottom_value, top_value);
					, "Spectrogram"));

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
					INTENSITY_COLOR_MAP_TYPE_DEFAULT_DARK);
		setIntensityColorMapType(
					i, d_intensity_color_map_type[i], QColor("black"), QColor("white"));

		setAlpha(i, 255);
	}

	// LeftButton for the zooming
	// MiddleButton for the panning
	// RightButton: zoom out by 1
	// Ctrl+RighButton: zoom out to full size
	d_zoomer[0] = new WaterfallZoomer(canvas());

#if QWT_VERSION < 0x060000
	d_zoomer[0]->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif
	d_zoomer[0]->setMousePattern(
				QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
	d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

	const QColor c("#999999");
	d_zoomer[0]->setRubberBandPen(c);
	d_zoomer[0]->setTrackerPen(c);

	QFont font;
	font.setPointSize(10);
	font.setWeight(75);
	d_zoomer[0]->setTrackerFont(font);

	_updateIntensityRangeDisplay();

	// offset between fft plot and waterfall plot raw values
	// TODO: the offset shouldn't be hardcoded
	d_intensity_offset = 75;

	d_xaxis_multiplier = 1;

	setVisibleSampleCount(150);
	d_grid->enableX(false);
	d_grid->enableY(false);

	freqFormatter.setTwoDecimalMode(true);

	OscScaleDraw *xScaleDraw = new OscScaleDraw(&freqFormatter, "");
	setAxisScaleDraw(QwtAxis::XBottom, xScaleDraw);
	xScaleDraw->setFloatPrecision(2);

	OscScaleEngine *scaleEngine = new OscScaleEngine();
	this->setAxisScaleEngine(QwtAxis::XBottom, (QwtScaleEngine *)scaleEngine);
	replot();
	auto div = axisScaleDiv(QwtAxis::XBottom);
	setXaxisNumDiv((div.ticks(2)).size() - 1);

	d_leftHandlesArea->setMinimumWidth(50);
	d_leftHandlesArea->setTopPadding(50);
	d_leftHandlesArea->setBottomPadding(55);
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());

	d_topHandlesArea->setMinimumHeight(50);
	d_topHandlesArea->setLargestChildWidth(50);

	d_TimeFormatter.setTwoDecimalMode(false);
	d_TimeFormatter.format(5, "h", 3);
	OscScaleDraw *yScaleDraw = new OscScaleDraw(&d_TimeFormatter, "");
	setAxisScaleDraw(QwtAxis::YLeft, yScaleDraw);
	yScaleDraw->setFloatPrecision(2);

	setLeftVertAxisUnit("s");
	setBtmHorAxisUnit("Hz");

	replot();

	connect(this, SIGNAL(resetWaterfallData), SLOT(resetAvgAcquisitionTime));

	setupReadouts();
	installEventFilter(this);
}

WaterfallDisplayPlot::~WaterfallDisplayPlot() {
	removeEventFilter(this);
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
}

void WaterfallDisplayPlot::setupReadouts()
{
	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	d_cursorReadouts->setTimeCursor1LabelText("F1 = ");
	d_cursorReadouts->setTimeCursor2LabelText("F2 = ");
	d_cursorReadouts->setTimeDeltaLabelText("ΔF = ");
	d_cursorReadouts->setVoltageCursor1LabelText("Time1 = ");
	d_cursorReadouts->setVoltageCursor2LabelText("Time2 = ");
	d_cursorReadouts->setDeltaVoltageLabelText("ΔTime = ");

	d_cursorReadouts->setFrequencyDeltaVisible(false);
	d_cursorReadouts->setTransparency(0);
}

void WaterfallDisplayPlot::onHCursor1Moved(double value)
{
	QString text;
	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	value = getFlowDirection() == WaterfallFlowDirection::UP ? value * d_avg_acquisition_time:
								value * d_avg_acquisition_time * -1 + d_center_plot_time;
	text = d_formatter->format(value, d_yAxisUnit, 3);
	d_cursorReadouts->setVoltageCursor1Text(error ? "-" : text);
	d_cursorReadoutsText.t1 = error ? "-" : text;

	double valueCursor2;
	if (d_trackMode) {
		valueCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());
	} else {
		valueCursor2 = d_hBar2->plotCoord().y();
	}
	valueCursor2 = getFlowDirection() == WaterfallFlowDirection::UP ? valueCursor2 * d_avg_acquisition_time:
									valueCursor2 * d_avg_acquisition_time * -1 + d_center_plot_time;

	double diff = value - (valueCursor2 * d_displayScale);
	text = d_formatter->format(diff, d_yAxisUnit, 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.tDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void WaterfallDisplayPlot::onHCursor2Moved(double value)
{
	QString text;
	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	value = getFlowDirection() == WaterfallFlowDirection::UP ? value * d_avg_acquisition_time:
								value * d_avg_acquisition_time * -1 + d_center_plot_time;

	text = d_formatter->format(value, d_yAxisUnit, 3);
	d_cursorReadouts->setVoltageCursor2Text(error ? "-" : text);
	d_cursorReadoutsText.t2 = error ? "-" : text;

	double valueCursor1;
	if (d_trackMode) {
		valueCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	} else {
		valueCursor1 = d_hBar1->plotCoord().y();
	}
	valueCursor1 = getFlowDirection() == WaterfallFlowDirection::UP ? valueCursor1 * d_avg_acquisition_time:
									valueCursor1 * d_avg_acquisition_time * -1 + d_center_plot_time;

	double diff = (valueCursor1 * d_displayScale) - value;
	text = d_formatter->format(diff, d_yAxisUnit, 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.tDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void WaterfallDisplayPlot::onVCursor1Moved(double value)
{
	QString text;
	text = d_formatter->format(value, d_xAxisUnit, 3);
	d_cursorReadouts->setTimeCursor1Text(text);
	d_cursorReadoutsText.v1 = text;

	double diff = value - d_vBar2->plotCoord().x();
	text = d_formatter->format(diff, d_xAxisUnit, 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	if (d_trackMode) {
		onHCursor1Moved(getHorizontalCursorIntersection(d_vBar1->plotCoord().x()));
	}

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void WaterfallDisplayPlot::onVCursor2Moved(double value)
{
	QString text;
	text = d_formatter->format(value, d_xAxisUnit, 3);
	d_cursorReadouts->setTimeCursor2Text(text);
	d_cursorReadoutsText.v2 = text;

	double diff = d_vBar1->plotCoord().x() - value;
	text = d_formatter->format(diff, d_xAxisUnit, 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	if (d_trackMode) {
		onHCursor2Moved(getHorizontalCursorIntersection(d_vBar2->plotCoord().x()));
	}

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void WaterfallDisplayPlot::updateCursorsData()
{
	onHCursor1Moved(d_hBar1->plotCoord().y());
	onHCursor2Moved(d_hBar2->plotCoord().y());
}

bool WaterfallDisplayPlot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() && event->type() == QEvent::Resize) {
		updateHandleAreaPadding();

		//force cursor handles to emit position changed
		//when the plot canvas is being resized
		d_hCursorHandle1->triggerMove();
		d_hCursorHandle2->triggerMove();
		d_vCursorHandle1->triggerMove();
		d_vCursorHandle2->triggerMove();
	}
	return QObject::eventFilter(object, event);
}

void WaterfallDisplayPlot::updateHandleAreaPadding()
{
	d_leftHandlesArea->update();
	d_bottomHandlesArea->setLeftPadding(d_leftHandlesArea->width());
	d_bottomHandlesArea->setRightPadding(50);

	d_rightHandlesArea->setTopPadding(50);
	d_rightHandlesArea->setBottomPadding(50);

	//update handle position to avoid cursors getting out of the plot bounds when changing the padding;
	d_hCursorHandle1->updatePosition();
	d_hCursorHandle2->updatePosition();
	d_vCursorHandle1->updatePosition();
	d_vCursorHandle2->updatePosition();
}

void WaterfallDisplayPlot::setVisibleSampleCount(int count)
{
	d_visible_samples = count;

	setYaxis(0, d_visible_samples);
	setNumRows(d_visible_samples);

	resetAxis(false);
}

void WaterfallDisplayPlot::enableYaxisLabels()
{
	d_leftHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new YLeftRuller(this)));
}

void WaterfallDisplayPlot::enableXaxisLabels()
{
	d_bottomHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new XBottomRuller(this)));
}

void WaterfallDisplayPlot::resetAxis(bool resetData)
{
	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data[i]->resizeData(d_start_frequency, d_stop_frequency, d_numPoints, d_nrows);
		if (resetData) {
			d_data[i]->reset();
		}
	}

	setAxisScale(QwtAxis::XBottom, d_start_frequency, d_stop_frequency);

	// Load up the new base zoom settings
	QwtDoubleRect zbase = d_zoomer[0]->zoomBase();
	d_zoomer[0]->zoom(zbase);
	d_zoomer[0]->setZoomBase(zbase);
	d_zoomer[0]->setZoomBase(true);
	d_zoomer[0]->zoom(0);
}

void WaterfallDisplayPlot::setLeftVertAxisUnit(const QString &unit)
{
	if (d_yAxisUnit != unit) {
		d_yAxisUnit = unit;

		auto scale_draw = dynamic_cast<OscScaleDraw*>(axisScaleDraw(QwtAxis::YLeft));
		if (scale_draw)
			scale_draw->setUnitType(unit);
	}
}

void WaterfallDisplayPlot::setBtmHorAxisUnit(const QString &unit)
{
	if (d_xAxisUnit != unit) {
		d_xAxisUnit = unit;

		auto scale_draw = dynamic_cast<OscScaleDraw*>(axisScaleDraw(QwtAxis::XBottom));
		if (scale_draw)
			scale_draw->setUnitType(unit);
	}
}

QString WaterfallDisplayPlot::formatXValue(double value, int precision) const
{
	return d_formatter->format(value, d_xAxisUnit, precision);
}

QString WaterfallDisplayPlot::formatYValue(double value, int precision) const
{
	value = getFlowDirection() == WaterfallFlowDirection::UP ? value * d_avg_acquisition_time:
								value * d_avg_acquisition_time * -1 + d_center_plot_time;

	return d_formatter->format(value, d_yAxisUnit, precision);
}

void WaterfallDisplayPlot::autoScale()
{	
	qDebug() << d_min_val << d_max_val;

	setIntensityRange(d_min_val, d_max_val);
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
			if (reset) {
				resetAxis();
			}

			((WaterfallZoomer*)d_zoomer[0])->setUnitType(strunits);
		}
	}
}

WaterfallFlowDirection WaterfallDisplayPlot::getFlowDirection() const
{
	auto direction = WaterfallFlowDirection::UP;
	for (unsigned int i = 0; i < d_nplots; ++i) {
		if (i == 0) {
			direction = d_data[i]->getFlowDirection();
		} else if (direction != d_data[i]->getFlowDirection()) {
			d_data[i]->setFlowDirection(direction);
		}
	}

	return direction;
}

int WaterfallDisplayPlot::getEnabledChannelID()
{
	return enabledChannelID;
}

void WaterfallDisplayPlot::setFlowDirection(WaterfallFlowDirection direction)
{
	for (unsigned int i = 0; i < d_nplots; ++i) {
		d_data[i]->setFlowDirection(direction);
		d_data[i]->reset();
	}
}

void WaterfallDisplayPlot::setResolutionBW(double value)
{
	d_resolution_bw = value;
}

double WaterfallDisplayPlot::getStartFrequency() const { return d_start_frequency; }

double WaterfallDisplayPlot::getResolutionBW() const { return d_resolution_bw; }

double WaterfallDisplayPlot::getStopFrequency() const { return d_stop_frequency; }

void WaterfallDisplayPlot::plotNewData(const std::vector<double*> dataPoints,
				       const int64_t numDataPoints,
				       gr::high_res_timer_type acquisitionTime,
				       const int droppedFrames)
{
	// Display first half of the plot if d_half_freq is true
	int64_t _npoints_in = d_half_freq ? (getStopFrequency() - getStartFrequency()) / getResolutionBW() : numDataPoints;
	int64_t _in_index = d_half_freq ? getStartFrequency() / getResolutionBW() : 0;
	double current_time = gr::high_res_timer_now();

	// convert to seconds
	double seconds_per_line = (current_time - d_last_draw_time) / 1000000000.;

	// compute avg acquisition time
	if (d_avg_acquisition_time == -1) {
		// avg time was reset
		d_avg_acquisition_time = seconds_per_line;

	} else {
		d_avg_acquisition_time = (d_avg_acquisition_time * (d_visible_line_count - 1) + seconds_per_line) / d_visible_line_count;
	}

	d_last_draw_time = current_time;
	double time_per_line = d_avg_acquisition_time;
	double centerTime = 0;

	if (getFlowDirection() == WaterfallFlowDirection::DOWN) {
		time_per_line *= -1;
		centerTime = -getNumRows() * time_per_line;
	}

	d_center_plot_time = centerTime;
	updateCursorsData();

	if (!d_stop && _npoints_in > 0) {
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

			d_leftHandlesArea->update();
			d_bottomHandlesArea->update();

			((WaterfallZoomer*)d_zoomer[0])->setSecondsPerLine(time_per_line);
			((WaterfallZoomer*)d_zoomer[0])->setCenterTime(centerTime);

			for (int i = 0; i<d_nplots; i++) {
				if (enabledChannelID != i) {
					setAlpha(i, 0);
					continue;
				} else {
					setAlpha(i, 255);
				}

				d_data[i]->addFFTData(&(dataPoints[i][_in_index]), _npoints_in, droppedFrames);
				d_data[i]->incrementNumLinesToUpdate();
				d_spectrogram[i]->invalidateCache();
				d_spectrogram[i]->itemChanged();
			}

			replot();
			d_visible_line_count = std::min(d_visible_line_count + 1, d_nrows);
		}

	Q_EMIT newWaterfallData();
}

void WaterfallDisplayPlot::setIntensityRange(double minIntensity,
					     double maxIntensity)
{
	minIntensity += d_intensity_offset;
	maxIntensity += d_intensity_offset;

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
	d_leftHandlesArea->update();
	d_bottomHandlesArea->update();

	FreqDisplayScaleDraw* freqScale =
			(FreqDisplayScaleDraw*)axisScaleDraw(QwtAxis::XBottom);
//	freqScale->initiateUpdate();

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
		case INTENSITY_COLOR_MAP_TYPE_DEFAULT_DARK: {
			d_intensity_color_map_type[which] = newType;
#if QWT_VERSION < 0x060000
			ColorMap_DefaultDark colorMap;
			d_spectrogram[which]->setColorMap(colorMap);
#else
			d_spectrogram[which]->setColorMap(new ColorMap_DefaultDark());
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

void WaterfallDisplayPlot::enableChannel(int id)
{
	enabledChannelID = id;
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
		case INTENSITY_COLOR_MAP_TYPE_DEFAULT_DARK:
			rightAxis->setColorMap(intv, new ColorMap_DefaultDark());
			break;
		default:
			rightAxis->setColorMap(intv, new ColorMap_DefaultDark());
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
	setAxisAutoScale(QwtAxis::YRight, false);
}

void WaterfallDisplayPlot::enableLegend()
{
	d_legend_enabled = true;
	setAxisAutoScale(QwtAxis::YRight, true);
}

void WaterfallDisplayPlot::enableLegend(bool en)
{
	d_legend_enabled = en;
	setAxisAutoScale(QwtAxis::YRight, en);
}

void WaterfallDisplayPlot::setNumRows(int nrows)
{
	d_nrows = nrows;
}

void WaterfallDisplayPlot::resetAvgAcquisitionTime()
{
	d_avg_acquisition_time = -1;
	d_visible_line_count = 0;
}

void WaterfallDisplayPlot::setUpdateTime(double t)
{
	resetAvgAcquisitionTime();

	d_time_per_fft = t;
}

void WaterfallDisplayPlot::customEvent(QEvent *e)
{
	if (e->type() == WaterfallUpdateEvent::Type()) {
		WaterfallUpdateEvent* event = (WaterfallUpdateEvent*)e;
		const std::vector<double*> dataPoints = event->getPoints();
		const uint64_t numDataPoints = event->getNumDataPoints();
		const gr::high_res_timer_type dataTimestamp = event->getDataTimestamp();

		for (unsigned int i = 0; i < d_nplots; ++i) {
			const double* min_val =
					std::min_element(&dataPoints[i][0], &dataPoints[i][numDataPoints - 1]);
			const double* max_val =
					std::max_element(&dataPoints[i][0], &dataPoints[i][numDataPoints - 1]);
			if (*min_val < d_min_val || i == 0)
				d_min_val = *min_val;
			if (*max_val > d_max_val || i == 0)
				d_max_val = *max_val;
		}
		//		autoScale();
		//		qDebug() << d_min_val << d_max_val;

		plotNewData(dataPoints, numDataPoints, dataTimestamp, 0);

		// reset zoomer base if plot axis changed
		if (getZoomer()->zoomBase().left() != d_start_frequency || getZoomer()->zoomBase().width() != d_stop_frequency - d_start_frequency) {
			getZoomer()->blockSignals(true);

			auto vert_interval = axisInterval(QwtAxis::YLeft);
			auto rect = QRectF(d_start_frequency, vert_interval.minValue(), d_stop_frequency - d_start_frequency, vert_interval.maxValue() - vert_interval.minValue());
			getZoomer()->zoom(rect);
			getZoomer()->setZoomBase(rect);
			getZoomer()->zoom(0);

			auto stack = QStack<QRectF>();
			stack.push(getZoomer()->zoomStack().first());
			getZoomer()->setZoomStack(stack, 0);

			getZoomer()->blockSignals(false);
		}
	}
}

void WaterfallDisplayPlot::setPlotPosHalf(bool half)
{
	d_half_freq = half;
	if (half)
		d_start_frequency = d_center_frequency;
}


#endif /* WATERFALL_DISPLAY_PLOT_C */
