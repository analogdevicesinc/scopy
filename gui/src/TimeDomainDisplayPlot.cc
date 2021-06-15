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

#ifndef TIME_DOMAIN_DISPLAY_PLOT_C
#define TIME_DOMAIN_DISPLAY_PLOT_C

#include <QColor>
#include <QFont>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

#include <cmath>
#include <iostream>
#include <scopy/gui/TimeDomainDisplayPlot.h>
#include <scopy/gui/osc_scale_engine.hpp>
#include <scopy/gui/smooth_curve_fitter.hpp>
#include <volk/volk.h>

using namespace scopy::gui;

class TimeDomainDisplayZoomer : public OscPlotZoomer
{
public:
#if QWT_VERSION < 0x060100
	TimeDomainDisplayZoomer(QwtPlotCanvas* canvas)
#else  /* QWT_VERSION < 0x060100 */
	TimeDomainDisplayZoomer(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
		: OscPlotZoomer(canvas)
	{
		setTrackerMode(QwtPicker::AlwaysOn);
	}

	virtual ~TimeDomainDisplayZoomer() {}

	virtual void updateTrackerText() { updateDisplay(); }

protected:
	using QwtPlotZoomer::trackerText;
	virtual QwtText trackerText(const QPoint& p) const
	{
		QwtText t;
		QPointF dp = QwtPlotZoomer::invTransform(p);
		const TimeDomainDisplayPlot* plt = (const TimeDomainDisplayPlot*)plot();
		t.setText(plt->timeScaleValueFormat(dp.x(), 3) + "," + plt->yAxisScaleValueFormat(dp.y(), 3));
		return t;
	}
};

SinkManager::SinkManager() {}

SinkManager::~SinkManager() {}

bool SinkManager::addSink(const std::string& name, unsigned int numChannels, unsigned long long channelsDataLength)
{
	bool can_add = true;

	for (auto it = d_sinkList.begin(); it != d_sinkList.end(); ++it)
		if ((*it).name() == name) {
			can_add = false;
			break;
		}
	if (can_add)
		d_sinkList.push_back(Sink(name, numChannels, channelsDataLength));

	return can_add;
}

bool SinkManager::removeSink(const std::string& name)
{
	bool found = false;

	for (auto it = d_sinkList.begin(); it != d_sinkList.end(); ++it)
		if ((*it).name() == name) {
			found = true;
			it = d_sinkList.erase(it);
			break;
		}

	return found;
}

unsigned int SinkManager::sinkListLength() { return d_sinkList.size(); }

Sink* SinkManager::sink(unsigned int index)
{
	if (index < d_sinkList.size())
		return &d_sinkList[index];

	return NULL;
}

int SinkManager::indexOfSink(const std::string& name)
{
	int pos = find_if(d_sinkList.begin(), d_sinkList.end(), [&name](Sink& s) { return s.name() == name; }) -
		d_sinkList.begin();

	if (pos >= d_sinkList.size())
		pos = -1;

	return pos;
}

int SinkManager::sinkFirstChannelPos(const std::string& name)
{
	int i, index = 0;

	for (i = 0; i < d_sinkList.size(); ++i) {
		if (d_sinkList[i].name() == name)
			break;
		index += d_sinkList[i].numChannels();
	}
	if (i == d_sinkList.size())
		index = -1;

	return index;
}

/***********************************************************************
 * Main Time domain plotter widget
 **********************************************************************/
TimeDomainDisplayPlot::TimeDomainDisplayPlot(QWidget* parent, bool isdBgraph, unsigned int xNumDivs,
					     unsigned int yNumDivs, PrefixFormatter* pfXaxis, PrefixFormatter* pfYaxis)
	: DisplayPlot(0, parent, isdBgraph, xNumDivs, yNumDivs)
{
	d_tag_text_color = Qt::black;
	d_tag_background_color = Qt::white;
	d_tag_background_style = Qt::NoBrush;

	d_sample_rate = 1;
	d_data_starting_point = 0.0;
	d_curves_hidden = false;
	d_nbPtsXAxis = 0;

	d_nb_ref_curves = 0;

	d_autoscale_state = false;

	// Reconfigure the bottom horizontal axis that was created by the base class
	configureAxis(QwtPlot::xBottom, 0, pfXaxis);
	configureAxis(QwtPlot::yLeft, 0, pfYaxis);

	d_yAxisUnit = "";
	d_xAxisUnit = "";

	// d_zoomer = new TimeDomainDisplayZoomer(canvas());

	QFont font;
	font.setPointSize(10);
	font.setWeight(75);
	// d_zoomer->setTrackerFont(font);

#if QWT_VERSION < 0x060000
	// d_zoomer->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

	// d_zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
	//			   Qt::RightButton, Qt::ControlModifier);
	// d_zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
	//		   Qt::RightButton);

	const QColor c("#999999");
	// d_zoomer->setRubberBandPen(c);
	// d_zoomer->setTrackerPen(c);

	d_semilogx = false;
	d_semilogy = false;
	d_autoscale_shot = false;

	d_xAxisFormatter->setTwoDecimalMode(true);
	d_yAxisFormatter->setTwoDecimalMode(true);

	d_xAxisFormatter->setTrimZeroes(true);
	d_yAxisFormatter->setTrimZeroes(true);

	d_tag_markers.resize(d_nplots);
	d_tag_markers_en = std::vector<bool>(d_nplots, true);

	d_trigger_lines[0] = new QwtPlotMarker();
	d_trigger_lines[0]->setLineStyle(QwtPlotMarker::HLine);
	d_trigger_lines[0]->setLinePen(QPen(Qt::red, 0.6, Qt::DashLine));
	d_trigger_lines[0]->setRenderHint(QwtPlotItem::RenderAntialiased);
	d_trigger_lines[0]->setXValue(0.0);
	d_trigger_lines[0]->setYValue(0.0);

	d_trigger_lines[1] = new QwtPlotMarker();
	d_trigger_lines[1]->setLineStyle(QwtPlotMarker::VLine);
	d_trigger_lines[1]->setLinePen(QPen(Qt::red, 0.6, Qt::DashLine));
	d_trigger_lines[1]->setRenderHint(QwtPlotItem::RenderAntialiased);
	d_trigger_lines[1]->setXValue(0.0);
	d_trigger_lines[1]->setYValue(0.0);

	for (int i = 0; i < 6; i++) {
		d_zoomer.push_back(new TimeDomainDisplayZoomer(this->canvas()));
		d_zoomer[i]->setEnabled(false);
	}
}

TimeDomainDisplayPlot::~TimeDomainDisplayPlot()
{
	while (d_sinkManager.sinkListLength()) {
		Sink* sink = d_sinkManager.sink(0);
		unregisterSink(sink->name());
	}

	// TODO: prob remove d_trigger_lines -> not used.
	delete d_trigger_lines[0];
	delete d_trigger_lines[1];

	for (int i = 0; i < d_ref_ydata.size(); ++i) {
		delete[] d_ref_ydata[i];
	}

	// d_zoomer and _panner deleted when parent deleted
}

void TimeDomainDisplayPlot::replot() { QwtPlot::replot(); }

void TimeDomainDisplayPlot::plotNewData(const std::string& sender, const std::vector<double*>& dataPoints,
					const int64_t numDataPoints, const double timeInterval,
					const std::vector<std::vector<gr::tag_t>>& tags)
{
	int sinkIndex = d_sinkManager.indexOfSink(sender);

	if (!d_stop) {
		if ((numDataPoints > 0) && sinkIndex >= 0) {
			Sink* sink = d_sinkManager.sink((unsigned int)sinkIndex);
			int start = d_sinkManager.sinkFirstChannelPos(sender);
			unsigned int sinkNumChannels = sink->numChannels();
			unsigned long long sinkNumPoints = sink->channelsDataLength();
			bool reset_x_axis_points = d_sink_reset_x_axis_pts[sinkIndex];

			if (numDataPoints != sinkNumPoints) {
				sinkNumPoints = numDataPoints;
				sink->setChannelsDataLength(numDataPoints);

				delete[] d_xdata[sinkIndex];
				d_xdata[sinkIndex] = new double[numDataPoints];

				int ref_offset = countReferenceWaveform(start);
				for (int i = start; i < start + sinkNumChannels; i++) {
					delete[] d_ydata[i];
					d_ydata[i] = new double[numDataPoints];

					d_plot_curve[i + ref_offset]->setRawSamples(d_xdata[sinkIndex], d_ydata[i],
										    numDataPoints);
				}

				_resetXAxisPoints(d_xdata[sinkIndex], numDataPoints, d_sample_rate);
			} else if (reset_x_axis_points) {
				_resetXAxisPoints(d_xdata[sinkIndex], numDataPoints, d_sample_rate);
				reset_x_axis_points = false;
			}

			for (int i = 0; i < sinkNumChannels; i++) {
				if (d_semilogy) {
					for (int n = 0; n < numDataPoints; n++)
						d_ydata[start + i][n] = fabs(dataPoints[i][n]);
				} else {
					memcpy(d_ydata[start + i], dataPoints[i], numDataPoints * sizeof(double));
				}
			}

			for (int i = 0; i < d_plot_curve.size(); i++)
				d_plot_curve.at(i)->show();
			d_curves_hidden = false;

			//      // Detach and delete any tags that were plotted last time
			//      for(int n = 0; n < d_nplots; n++) {
			//        for(size_t i = 0; i < d_tag_markers[n].size(); i++) {
			//          d_tag_markers[n][i]->detach();
			//          delete d_tag_markers[n][i];
			//        }
			//        d_tag_markers[n].clear();
			//      }

			// Plot and attach any new tags found.
			// First test if this was a complex input where real/imag get
			// split here into two stream.
			//      if(tags.size() > 0) {
			//        bool cmplx = false;
			//        int mult = (int)d_nplots / (int)tags.size();
			//        if(mult == 2)
			//          cmplx = true;

			//        std::vector< std::vector<gr::tag_t> >::const_iterator tag = tags.begin();
			//        for(int i = 0; i < d_nplots; i+=mult) {
			//          std::vector<gr::tag_t>::const_iterator t;
			//          for(t = tag->begin(); t != tag->end(); t++) {
			//            uint64_t offset = (*t).offset;

			//            // Ignore tag if its offset is outside our plottable vector.
			//            if(offset >= (uint64_t)d_numPoints) {
			//              continue;
			//            }

			//            double sample_offset = double(offset)/d_sample_rate;

			//            std::stringstream s;
			//            s << (*t).key << ": " << (*t).value;

			//            // Select the right input stream to put the tag on. If real,
			//            // just use i; if it's a complex stream, find the max of the
			//            // real and imaginary parts and put the tag on that one.
			//            int which = i;
			//            if(cmplx) {
			//              bool show0 = d_plot_curve[i]->isVisible();
			//              bool show1 = d_plot_curve[i+1]->isVisible();

			//              // If we are showing both streams, select the inptu stream
			//              // with the larger value
			//              if(show0 && show1) {
			//                if(fabs(d_ydata[i][offset]) < fabs(d_ydata[i+1][offset]))
			//                  which = i+1;
			//              }
			//              else {
			//                // If show0, we keep which = i; otherwise, use i+1.
			//                if(show1)
			//                  which = i+1;
			//              }
			//            }

			//            double yval = d_ydata[which][offset];

			//            // Find if we already have a marker at this location
			//            std::vector<QwtPlotMarker*>::iterator mitr;
			//            for(mitr = d_tag_markers[which].begin(); mitr != d_tag_markers[which].end();
			//            mitr++) {
			//              if((*mitr)->xValue() == sample_offset) {
			//                break;
			//              }
			//            }

			//            // If no matching marker, create a new one
			//            if(mitr == d_tag_markers[which].end()) {
			//              bool show = d_plot_curve[which]->isVisible();

			//              QwtPlotMarker *m = new QwtPlotMarker();
			//              m->setXValue(sample_offset);
			//              m->setYValue(yval);

			//              QBrush brush;
			//              brush.setColor(QColor(0xC8, 0x2F, 0x1F));
			//              brush.setStyle(Qt::SolidPattern);

			//              QPen pen;
			//              pen.setColor(Qt::black);
			//              pen.setWidth(1);

			//              QwtSymbol *sym = new QwtSymbol(QwtSymbol::NoSymbol, brush, pen, QSize(12,12));

			//              if(yval >= 0) {
			//                sym->setStyle(QwtSymbol::DTriangle);
			//                m->setLabelAlignment(Qt::AlignTop);
			//              }
			//              else {
			//                sym->setStyle(QwtSymbol::UTriangle);
			//                m->setLabelAlignment(Qt::AlignBottom);
			//              }

			//#if QWT_VERSION < 0x060000
			//              m->setSymbol(*sym);
			//#else
			//              m->setSymbol(sym);
			//#endif

			//              m->setLabel(QwtText(s.str().c_str()));
			//              m->attach(this);

			//              if(!(show && d_tag_markers_en[which])) {
			//                m->hide();
			//              }

			//              d_tag_markers[which].push_back(m);
			//            }
			//            else {
			//              // Prepend the new tag to the existing marker
			//              // And set it at the max value
			//              if(fabs(yval) < fabs((*mitr)->yValue()))
			//                (*mitr)->setYValue(yval);
			//              QString orig = (*mitr)->label().text();
			//              s << std::endl;
			//              orig.prepend(s.str().c_str());

			//              QwtText newtext(orig);
			//              newtext.setColor(getTagTextColor());

			//              QBrush brush(getTagBackgroundColor(), getTagBackgroundStyle());
			//              newtext.setBackgroundBrush(brush);

			//              (*mitr)->setLabel(newtext);
			//            }
			//          }

			//          tag++;
			//        }
			//      }

			if (d_autoscale_state) {
				double bottom = 1e20, top = -1e20;
				for (int n = 0; n < d_nplots; n++) {
					if (d_plot_curve[n]->plot()) {
						for (int64_t point = 0; point < numDataPoints; point++) {
							if (d_ydata[n][point] < bottom) {
								bottom = d_ydata[n][point];
							}
							if (d_ydata[n][point] > top) {
								top = d_ydata[n][point];
							}
						}
					}
				}
				_autoScale(bottom, top);
				if (d_autoscale_shot) {
					d_autoscale_state = false;
					d_autoscale_shot = false;
				}
			}

			replot();

			Q_EMIT newData();
		}
	}
}

void TimeDomainDisplayPlot::newData(const QEvent* updateEvent)
{
	IdentifiableTimeUpdateEvent* tevent = (IdentifiableTimeUpdateEvent*)updateEvent;
	const std::vector<double*> dataPoints = tevent->getTimeDomainPoints();
	const uint64_t numDataPoints = tevent->getNumTimeDomainDataPoints();
	const std::vector<std::vector<gr::tag_t>> tags = tevent->getTags();
	const std::string sender = tevent->senderName();

	if ((d_nbPtsXAxis != 0) && (d_nbPtsXAxis <= numDataPoints) && sender == "Osc Time") {
		Q_EMIT filledScreen(true, numDataPoints);
	}

	this->plotNewData(sender, dataPoints, numDataPoints, 0, tags);
}

void TimeDomainDisplayPlot::customEvent(QEvent* e)
{
	if (e->type() == TimeUpdateEvent::Type()) {
		newData(e);
	}
}

void TimeDomainDisplayPlot::legendEntryChecked(QwtPlotItem* plotItem, bool on)
{
	// When line is turned on/off, immediately show/hide tag markers
	for (int n = 0; n < d_nplots; n++) {
		if (plotItem == d_plot_curve[n]) {
			for (size_t i = 0; i < d_tag_markers[n].size(); i++) {
				if (!(!on && d_tag_markers_en[n]))
					d_tag_markers[n][i]->hide();
				else
					d_tag_markers[n][i]->show();
			}
		}
	}
	DisplayPlot::legendEntryChecked(plotItem, on);
}

void TimeDomainDisplayPlot::legendEntryChecked(const QVariant& plotItem, bool on, int index)
{
#if QWT_VERSION < 0x060100
	std::runtime_error(
		"TimeDomainDisplayPlot::legendEntryChecked with QVariant not enabled in this version of QWT.\n");
#else
	QwtPlotItem* p = infoToItem(plotItem);
	legendEntryChecked(p, on);
#endif /* QWT_VERSION < 0x060100 */
}

void TimeDomainDisplayPlot::_resetXAxisPoints(double*& xAxis, unsigned long long numPoints, double sampleRate)
{
	double delt = 1.0 / sampleRate;

	for (long loc = 0; loc < numPoints; loc++)
		xAxis[loc] = (d_data_starting_point + loc) * delt;

	// Set up zoomer base for maximum unzoom x-axis
	// and reset to maximum unzoom level
	//  QRectF zbase = d_zoomer->zoomBase();

	//  if(d_semilogx) {
	//    setAxisScale(QwtPlot::xBottom, 1e-1, d_numPoints*delt);
	//    zbase.setLeft(1e-1);
	//  }
	//  else {
	//    setAxisScale(QwtPlot::xBottom, 0, d_numPoints*delt);
	//    zbase.setLeft(0);
	//  }

	//  zbase.setRight(d_numPoints*delt);
	//  d_zoomer->zoom(zbase);
	//  d_zoomer->setZoomBase(zbase);
	//  d_zoomer->zoom(0);
}

void TimeDomainDisplayPlot::_autoScale(double bottom, double top)
{
	// Auto scale the y-axis with a margin of 20% (10 dB for log scale)
	double _bot = bottom - fabs(bottom - top) * 0.125;
	double _top = top + fabs(bottom - top) * 0.125;
	if (d_semilogy) {
		if (bottom > 0) {
			setYaxis(_bot - 10, _top + 10);
		} else {
			setYaxis(1e-3, _top + 10);
		}
	} else {
		if (_bot == 0 && _top == 0) {
			setYaxis(-1e-3, 1e-3);
		} else {
			setYaxis(_bot, _top);
		}
	}

	replot();
}

void TimeDomainDisplayPlot::addZoomer(unsigned int zoomerIdx)
{
	QFont font;
	font.setPointSize(10);
	font.setWeight(75);
	d_zoomer[zoomerIdx]->setTrackerFont(font);

#if QWT_VERSION < 0x060000
	d_zoomer[zoomerIdx]->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

	d_zoomer[zoomerIdx]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
	d_zoomer[zoomerIdx]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
	d_zoomer[zoomerIdx]->setTrackerMode(QwtPicker::AlwaysOff);
	const QColor c("#999999");
	d_zoomer[zoomerIdx]->setRubberBandPen(c);
	d_zoomer[zoomerIdx]->setTrackerPen(getLineColor(zoomerIdx));

	d_zoomer[zoomerIdx]->setEnabled(true);
	d_zoomer[zoomerIdx]->setAxes(QwtAxisId(QwtPlot::xBottom, 0), QwtAxisId(QwtPlot::yLeft, zoomerIdx));
}

void TimeDomainDisplayPlot::removeZoomer(unsigned int zoomerIdx)
{
	if (zoomerIdx == 0 || zoomerIdx == 1) {
		d_zoomer[zoomerIdx]->setEnabled(false);
		return;
	}

	int toDisable = zoomerIdx;
	while (d_zoomer[toDisable]->isEnabled() && toDisable < d_zoomer.size() - 1) {
		toDisable++;
	}

	if (toDisable == d_zoomer.size() - 1 && d_zoomer[toDisable]->isEnabled()) {
		d_zoomer[toDisable]->setEnabled(false);
	} else {
		d_zoomer[toDisable - 1]->setEnabled(false);
	}

	for (int i = 0; i < d_zoomer.size(); ++i) {
		if (d_zoomer[i]->isEnabled()) {
			d_zoomer[i]->setAxes(QwtAxisId(QwtPlot::xBottom, 0), QwtAxisId(QwtPlot::yLeft, i));
			d_zoomer[i]->setTrackerPen(getLineColor(i));
		}
	}
}

void TimeDomainDisplayPlot::setXAxisNumPoints(unsigned int pts) { d_nbPtsXAxis = pts; }

void TimeDomainDisplayPlot::setAutoScale(bool state)
{
	d_autoscale_state = state;

	if (d_autoscale_state) {
		double bottom = 1e20, top = -1e20;
		for (int n = 0; n < d_nplots; n++) {
			if (d_plot_curve[n]->plot()) {
				for (int64_t point = 0; point < Curve(n)->data()->size(); point++) {
					if (d_ydata[n][point] < bottom) {
						bottom = d_ydata[n][point];
					}
					if (d_ydata[n][point] > top) {
						top = d_ydata[n][point];
					}
				}
			}
		}
		_autoScale(bottom, top);
		if (d_autoscale_shot) {
			d_autoscale_state = false;
			d_autoscale_shot = false;
		}
	} else {
		setYaxis(-5, 5);
		replot();
	}
}

void TimeDomainDisplayPlot::setAutoScaleShot()
{
	d_autoscale_state = true;
	d_autoscale_shot = true;
}

void TimeDomainDisplayPlot::setSampleRate(double sr, double units, const std::string& strunits)
{
	double newsr = sr / units;
	if (newsr != d_sample_rate) {
		d_sample_rate = sr / units;

		for (unsigned int i = 0; i < d_sinkManager.sinkListLength(); i++)
			_resetXAxisPoints(d_xdata[i], d_sinkManager.sink(i)->channelsDataLength(), d_sample_rate);
	}
}

double TimeDomainDisplayPlot::sampleRate() const { return d_sample_rate; }

void TimeDomainDisplayPlot::setYaxisUnit(QString unitType)
{
	if (d_yAxisUnit != unitType) {
		d_yAxisUnit = unitType;

		OscScaleDraw* scaleDraw = static_cast<OscScaleDraw*>(this->axisScaleDraw(QwtPlot::yLeft));
		if (scaleDraw)
			scaleDraw->setUnitType(d_yAxisUnit);
	}
}

QString TimeDomainDisplayPlot::yAxisUnit(void) { return d_yAxisUnit; }

void TimeDomainDisplayPlot::setXaxisUnit(QString unitType)
{
	if (d_xAxisUnit != unitType) {
		d_xAxisUnit = unitType;

		OscScaleDraw* scaleDraw = static_cast<OscScaleDraw*>(this->axisScaleDraw(QwtPlot::xBottom));
		if (scaleDraw)
			scaleDraw->setUnitType(d_xAxisUnit);
	}
}

QString TimeDomainDisplayPlot::xAxisUnit() { return d_xAxisUnit; }

void TimeDomainDisplayPlot::stemPlot(bool en)
{
	if (en) {
		for (int i = 0; i < d_nplots; i++) {
			d_plot_curve[i]->setStyle(QwtPlotCurve::Sticks);
			setLineMarker(i, QwtSymbol::Ellipse);
		}
	} else {
		for (int i = 0; i < d_nplots; i++) {
			d_plot_curve[i]->setStyle(QwtPlotCurve::Lines);
			setLineMarker(i, QwtSymbol::NoSymbol);
		}
	}
}

void TimeDomainDisplayPlot::setPlotLineStyle(unsigned int chIdx, unsigned int style)
{
	if (chIdx < 0 || chIdx >= d_plot_curve.size()) {
		return;
	}

	auto curve = d_plot_curve[chIdx];

	curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
	curve->setCurveAttribute(QwtPlotCurve::Fitted, false);

	switch (style) {
	case 0:
		curve->setStyle(QwtPlotCurve::CurveStyle::Lines);
		replot();
		break;
	case 1:
		curve->setStyle(QwtPlotCurve::CurveStyle::Dots);
		replot();
		break;
	case 2:
		curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
		replot();
		break;
	case 3:
		curve->setStyle(QwtPlotCurve::CurveStyle::Sticks);
		replot();
		break;
	case 4:
		curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
		curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
		curve->setStyle(QwtPlotCurve::CurveStyle::Lines);
		replot();
		break;
	}
}

int TimeDomainDisplayPlot::getPlotLineStyle(unsigned int chIdx) const
{
	if (chIdx < 0 || chIdx >= d_plot_curve.size()) {
		return -1;
	}

	auto style = d_plot_curve[chIdx]->style();
	switch (style) {
	case QwtPlotCurve::CurveStyle::Lines:
		if (d_plot_curve[chIdx]->testCurveAttribute(QwtPlotCurve::Fitted)) {
			return 4;
		} else {
			return 0;
		}

	case QwtPlotCurve::CurveStyle::Dots:
		return 1;

	case QwtPlotCurve::CurveStyle::Steps:
		return 2;

	case QwtPlotCurve::CurveStyle::Sticks:
		return 3;
	default:
		return -1;
	}
}

void TimeDomainDisplayPlot::setSemilogx(bool en)
{
	d_semilogx = en;
	if (!d_semilogx) {
		setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
	} else {
#if QWT_VERSION < 0x060100
		setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#else  /* QWT_VERSION < 0x060100 */
		setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
#endif /*QWT_VERSION < 0x060100 */
	}
	for (unsigned int i = 0; i < d_sinkManager.sinkListLength(); i++)
		_resetXAxisPoints(d_xdata[i], d_sinkManager.sink(i)->channelsDataLength(), d_sample_rate);
}

void TimeDomainDisplayPlot::setSemilogy(bool en)
{
	if (d_semilogy != en) {
		d_semilogy = en;

#if QWT_VERSION < 0x060100
		double max = axisScaleDiv(QwtPlot::yLeft)->upperBound();
#else  /* QWT_VERSION < 0x060100 */
		double max = axisScaleDiv(QwtPlot::yLeft).upperBound();
#endif /* QWT_VERSION < 0x060100 */

		if (!d_semilogy) {
			setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
			setYaxis(-pow(10.0, max / 10.0), pow(10.0, max / 10.0));
		} else {
#if QWT_VERSION < 0x060100
			setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#else  /* QWT_VERSION < 0x060100 */
			setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
#endif /*QWT_VERSION < 0x060100 */
			setYaxis(1e-10, 10.0 * log10(max));
		}
	}
}

void TimeDomainDisplayPlot::enableTagMarker(int which, bool en)
{
	if ((size_t)which < d_tag_markers_en.size())
		d_tag_markers_en[which] = en;
	else
		throw std::runtime_error("TimeDomainDisplayPlot: enabled tag marker does not exist.\n");
}

const QColor TimeDomainDisplayPlot::getTagTextColor() { return d_tag_text_color; }

const QColor TimeDomainDisplayPlot::getTagBackgroundColor() { return d_tag_background_color; }

const Qt::BrushStyle TimeDomainDisplayPlot::getTagBackgroundStyle() { return d_tag_background_style; }

void TimeDomainDisplayPlot::setTagTextColor(QColor c) { d_tag_text_color = c; }

void TimeDomainDisplayPlot::setTagBackgroundColor(QColor c) { d_tag_background_color = c; }

void TimeDomainDisplayPlot::setTagBackgroundStyle(Qt::BrushStyle b) { d_tag_background_style = b; }

void TimeDomainDisplayPlot::setZoomerEnabled(bool en)
{
	for (unsigned int i = 0; i < d_zoomer.size(); ++i)
		d_zoomer[i]->setEnabled(en);
}

bool TimeDomainDisplayPlot::isZoomerEnabled()
{
	if (d_zoomer.isEmpty())
		return false;
	return d_zoomer[0]->isEnabled();
}

void TimeDomainDisplayPlot::setZoomerVertAxis(int index)
{
	if (index < -1 || index >= axesCount(QwtPlot::yLeft))
		return;

	for (unsigned int i = 0; i < d_zoomer.size(); ++i)
		if (d_zoomer[i]->isEnabled())
			d_zoomer[i]->setTrackerMode((i == index) ? QwtPicker::AlwaysOn : QwtPicker::AlwaysOff);
}

QString TimeDomainDisplayPlot::timeScaleValueFormat(double value, int precision) const
{
	return d_yAxisFormatter->format(value, "", precision);
}

QString TimeDomainDisplayPlot::timeScaleValueFormat(double value)
{
	OscScaleDraw* scale = static_cast<OscScaleDraw*>(this->axisScaleDraw(QwtPlot::xBottom));

	return d_yAxisFormatter->format(value, "", scale->getFloatPrecison());
}

QString TimeDomainDisplayPlot::yAxisScaleValueFormat(double value, int precision) const
{
	value *= d_displayScale;
	return d_xAxisFormatter->format(value, d_yAxisUnit, precision);
}

QString TimeDomainDisplayPlot::yAxisScaleValueFormat(double value)
{
	OscScaleDraw* scale = static_cast<OscScaleDraw*>(this->axisScaleDraw(QwtPlot::yLeft));

	return d_xAxisFormatter->format(value, d_yAxisUnit, scale->getFloatPrecison());
}

void TimeDomainDisplayPlot::setYLabel(const std::string& label, const std::string& unit)
{
	std::string l = label;
	if (unit.length() > 0)
		l += " (" + unit + ")";
	setAxisTitle(QwtPlot::yLeft, QString(l.c_str()));
}

void TimeDomainDisplayPlot::attachTriggerLines(bool en)
{
	if (en) {
		d_trigger_lines[0]->attach(this);
		d_trigger_lines[1]->attach(this);
	} else {
		d_trigger_lines[0]->detach();
		d_trigger_lines[1]->detach();
	}
}

void TimeDomainDisplayPlot::setTriggerLines(double x, double y)
{
	d_trigger_lines[0]->setXValue(x);
	d_trigger_lines[0]->setYValue(y);
	d_trigger_lines[1]->setXValue(x);
	d_trigger_lines[1]->setYValue(y);
}

QColor TimeDomainDisplayPlot::getChannelColor()
{
	for (QList<QColor>::const_iterator it = d_CurveColors.cbegin(); it != d_CurveColors.cend(); ++it) {
		bool used = false;

		for (std::vector<QwtPlotCurve*>::const_iterator it2 = d_plot_curve.cbegin();
		     !used && it2 != d_plot_curve.cend(); ++it2)
			used = (*it2)->pen().color() == (*it);

		if (!used)
			return *it;
	}

	return Qt::black;
}

bool TimeDomainDisplayPlot::isReferenceWaveform(QwtPlotCurve* curve) { return d_ref_curves.values().contains(curve); }

bool TimeDomainDisplayPlot::isMathWaveform(QwtPlotCurve* curve) const { return d_math_curves.values().contains(curve); }

int TimeDomainDisplayPlot::getCurveNextTo(int pos)
{
	while (isReferenceWaveform(Curve(pos)))
		pos++;
	return pos;
}

int TimeDomainDisplayPlot::countReferenceWaveform(int position)
{
	/* returns the number of curves that are of type reference that were added before "curve" */

	int curveIdx = getCurveNextTo(position);
	QwtPlotCurve* curve = Curve(curveIdx);

	int count = 0;
	for (int i = 0; i < d_plot_curve.size(); ++i)
		if (d_plot_curve[i] == curve) {
			return count;
		} else if (isReferenceWaveform(d_plot_curve[i])) {
			count++;
		}
	return count;
}

void TimeDomainDisplayPlot::registerReferenceWaveform(QString name, QVector<double> xData, QVector<double> yData)
{

	QColor color = getChannelColor();

	QwtPlotCurve* curve = new QwtPlotCurve();
	curve->setSamples(xData, yData);

	curve->setPen(QPen(color));
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);

	QwtSymbol* symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(color), QPen(color), QSize(7, 7));

	curve->setSymbol(symbol);

	curve->attach(this);

	d_ref_ydata.push_back(new double[yData.size()]);
	int n = d_ref_ydata.size() - 1;
	memset(d_ref_ydata[n], 0x0, yData.size() * sizeof(double));
	for (int i = 0; i < yData.size(); ++i)
		d_ref_ydata[d_ref_ydata.size() - 1][i] = yData[i];

	d_plot_curve.push_back(curve);
	d_ref_curves.insert(name, curve);
	d_nplots += 1;

	Q_EMIT channelAdded(d_ydata.size() + d_nb_ref_curves);

	d_nb_ref_curves++;
}

void TimeDomainDisplayPlot::unregisterReferenceWaveform(QString name)
{
	auto it = std::find(d_plot_curve.begin(), d_plot_curve.end(), d_ref_curves[name]);

	if (it == d_plot_curve.end()) {
		return;
	}

	d_plot_curve.erase(std::find(d_plot_curve.begin(), d_plot_curve.end(), d_ref_curves[name]));
	QwtPlotCurve* curve = d_ref_curves[name];
	d_ref_curves.remove(name);
	int pos = 0;
	int i = 0;
	for (i = 0; i < d_plot_curve.size(); i++) {
		if (isReferenceWaveform(Curve(i))) {
			if (Curve(i)->title().text() == name) {
				break;
			} else {
				pos++;
			}
		}
	}
	cleanUpJustBeforeChannelRemoval(i);

	delete[] d_ref_ydata[pos];
	d_ref_ydata.erase(d_ref_ydata.begin() + pos);
	curve->detach();
	delete curve;

	d_nb_ref_curves--;
}

void TimeDomainDisplayPlot::addDigitalPlotCurve(QwtPlotCurve* curve, bool visible)
{
	d_logic_curves.push_back(curve);

	if (visible) {
		curve->attach(this);
	}

	Q_EMIT digitalPlotCurveAdded(d_logic_curves.size() - 1);
}

void TimeDomainDisplayPlot::removeDigitalPlotCurve(QwtPlotCurve* curve)
{
	curve->detach();
	d_logic_curves.erase(std::find(d_logic_curves.begin(), d_logic_curves.end(), curve));
}

void TimeDomainDisplayPlot::enableDigitalPlotCurve(int curveId, bool enable)
{
	if (curveId < 0 || curveId > d_logic_curves.size() - 1) {
		return;
	}

	if (enable) {
		d_logic_curves[curveId]->attach(this);
	} else {
		d_logic_curves[curveId]->detach();
	}
}

int TimeDomainDisplayPlot::getNrDigitalPlotCurves() const { return d_logic_curves.size(); }

QwtPlotCurve* TimeDomainDisplayPlot::getDigitalPlotCurve(int curveId)
{
	if (curveId < 0 || curveId > d_logic_curves.size() - 1) {
		return nullptr;
	}

	return d_logic_curves[curveId];
}

void TimeDomainDisplayPlot::addPreview(QVector<QVector<double>> curvesToBePreviewed, double reftimebase,
				       double timebase, double timeposition)
{
	d_preview_ydata.resize(curvesToBePreviewed[0].size());
	for (int i = 0; i < curvesToBePreviewed.size(); ++i) {
		for (int j = 0; j < curvesToBePreviewed[i].size(); ++j) {
			d_preview_ydata[j].push_back(curvesToBePreviewed[i][j]);
		}
	}

	double mid_point_on_screen = (timebase * 8) - ((timebase * 8) - timeposition);
	double x_axis_step_size = (reftimebase / (curvesToBePreviewed.size() / xAxisNumDiv()));

	QVector<double> xData;
	for (int i = -(curvesToBePreviewed.size() / 2); i < (curvesToBePreviewed.size() / 2); ++i) {
		xData.push_back(mid_point_on_screen + ((double)i * x_axis_step_size));
	}

	for (int i = 0; i < curvesToBePreviewed[0].size(); ++i) {
		QwtPlotCurve* curve = new QwtPlotCurve();
		curve->setSamples(xData, d_preview_ydata[i]);

		curve->setPen(QPen(Qt::gray));
		curve->setRenderHint(QwtPlotItem::RenderAntialiased);

		QwtSymbol* symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(Qt::gray), QPen(Qt::gray), QSize(7, 7));

		curve->setSymbol(symbol);

		curve->attach(this);

		d_preview_curves.push_back(curve);
		replot();
	}
}

void TimeDomainDisplayPlot::clearPreview()
{
	for (auto it = d_preview_curves.begin(); it != d_preview_curves.end(); ++it) {
		(*it)->detach();
		delete *it;
	}
	d_preview_curves.clear();
}

void TimeDomainDisplayPlot::updatePreview(double reftimebase, double timebase, double timeposition)
{
	if (!d_preview_ydata.size()) {
		return;
	}
	double mid_point_on_screen = (timebase * 8) - ((timebase * 8) - timeposition);
	double x_axis_step_size = (reftimebase / (d_preview_ydata[0].size() / xAxisNumDiv()));

	QVector<double> xData;
	for (int i = -(d_preview_ydata[0].size() / 2); i < (d_preview_ydata[0].size() / 2); ++i) {
		xData.push_back(mid_point_on_screen + ((double)i * x_axis_step_size));
	}

	for (int i = 0; i < d_preview_curves.size(); ++i) {

		d_preview_curves[i]->setSamples(xData, d_preview_ydata[i]);

		replot();
	}
}

void TimeDomainDisplayPlot::realignReferenceWaveforms(double timebase, double timeposition)
{

	QList<QwtPlotCurve*> curves = d_ref_curves.values();

	for (auto& curve : curves) {
		double x_axis_step_size = curve->data()->sample(1).x() - curve->data()->sample(0).x();
		double mid_point_on_screen = (timebase * 8) - ((timebase * 8) - timeposition);
		int nr_of_samples_in_file = curve->data()->size();

		QVector<double> xData;
		QVector<double> yData;

		for (int i = -(nr_of_samples_in_file / 2); i < (nr_of_samples_in_file / 2); ++i) {
			xData.push_back(mid_point_on_screen + ((double)i * x_axis_step_size));
		}

		for (int i = 0; i < nr_of_samples_in_file; ++i)
			yData.push_back(curve->data()->sample(i).y());

		curve->setSamples(xData, yData);
	}
}

bool TimeDomainDisplayPlot::registerMathWaveform(const std::string& sinkUniqueNme, unsigned int numChannels,
						 unsigned long long channelsDataLength, bool curvesAttached)
{
	bool ret = registerSink(sinkUniqueNme, numChannels, channelsDataLength, curvesAttached);
	if (ret) {
		d_math_curves.insert(QString::fromStdString(sinkUniqueNme), d_plot_curve.back());
	}
	return ret;
}

bool TimeDomainDisplayPlot::registerSink(std::string sinkUniqueNme, unsigned int numChannels,
					 unsigned long long channelsDataLength, bool curvesAttached)
{
	bool ret;

	ret = d_sinkManager.addSink(sinkUniqueNme, numChannels, channelsDataLength);
	if (ret) {
		int numCurves = d_ydata.size();
		int sinkIndex = d_sinkManager.indexOfSink(sinkUniqueNme);
		d_xdata.push_back(new double[channelsDataLength]);

		for (int i = 0; i < numChannels; i++) {
			int n = i + numCurves;
			d_ydata.push_back(new double[channelsDataLength]);
			memset(d_ydata[n], 0x0, channelsDataLength * sizeof(double));

			QColor color = getChannelColor();

			QwtPlotCurve* curve = new QwtPlotCurve(QString("Data %1").arg(n));
			curve->setPen(QPen(color));
			curve->setRenderHint(QwtPlotItem::RenderAntialiased);
			d_plot_curve.push_back(curve);
			curve->setItemAttribute(QwtPlotItem::Legend, true);

			QwtSymbol* symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(color), QPen(color), QSize(7, 7));

			if (sinkIndex >= 1) {
				d_plot_curve.back()->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
				d_plot_curve.back()->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive, true);
			}

			d_plot_curve.back()->setRawSamples(d_xdata[sinkIndex], d_ydata[n], channelsDataLength);
			d_plot_curve.back()->setSymbol(symbol);

			d_plot_curve.back()->setCurveFitter(new SmoothCurveFitter());

			if (curvesAttached)
				d_plot_curve.back()->attach(this);

			Q_EMIT channelAdded(n + d_nb_ref_curves);
		}
		d_nplots += numChannels;
		_resetXAxisPoints(d_xdata[sinkIndex], channelsDataLength, d_sample_rate);
		d_tag_markers.resize(d_nplots);

		d_sink_reset_x_axis_pts.push_back(false);
	}

	return ret;
}

bool TimeDomainDisplayPlot::unregisterMathWaveform(const std::string& sinkName)
{
	int ret = unregisterSink(sinkName);
	if (ret) {
		d_math_curves.remove(QString::fromStdString(sinkName));
	}
	return ret;
}

bool TimeDomainDisplayPlot::unregisterSink(std::string sinkName)
{
	bool ret = false;

	int sinkIndex = d_sinkManager.indexOfSink(sinkName);
	if (sinkIndex >= 0) {

		// Remove X axis associated with the channels of the sink
		delete[] d_xdata[sinkIndex];
		d_xdata.erase(d_xdata.begin() + sinkIndex);

		// Remove Y axes corresponding to each channel of the sink
		int offset = d_sinkManager.sinkFirstChannelPos(sinkName);
		int numChannels = d_sinkManager.sink(sinkIndex)->numChannels();
		for (int i = offset; i < offset + numChannels; i++) {
			cleanUpJustBeforeChannelRemoval(offset);
			delete[] d_ydata[i];
		}
		d_ydata.erase(d_ydata.begin() + offset, d_ydata.begin() + offset + numChannels);

		/* Remove the QwtPlotCurve */
		int ref_offset = countReferenceWaveform(offset);
		for (int i = offset; i < offset + numChannels; i++) {
			d_plot_curve[i + ref_offset]->detach();
			delete d_plot_curve[i + ref_offset];
		}

		d_plot_curve.erase(d_plot_curve.begin() + offset + ref_offset,
				   d_plot_curve.begin() + ref_offset + offset + numChannels);

		// Finally remove the sink
		ret = d_sinkManager.removeSink(sinkName);

		d_nplots -= numChannels;
		d_tag_markers.resize(d_nplots);

		d_sink_reset_x_axis_pts.erase(d_sink_reset_x_axis_pts.begin() + sinkIndex);
	}

	return ret;
}

void TimeDomainDisplayPlot::configureAxis(int axisPos, int axisIdx, PrefixFormatter* prefixFormatter)
{
	QwtAxisId axis(axisPos, axisIdx);
//	PrefixFormatter* prefixFormatter;
	QString unit;
	unsigned int floatPrecision;
	unsigned int numDivs;

	if (axisPos == QwtPlot::yLeft) {
		d_xAxisFormatter = prefixFormatter;
		unit = d_yAxisUnit;
		floatPrecision = 2;
		numDivs = yAxisNumDiv();
	} else {
		d_yAxisFormatter = prefixFormatter;
		unit = d_xAxisUnit;
		floatPrecision = 2;
		numDivs = xAxisNumDiv();
	}

	// Use a custom Scale Engine to keep the grid fixed
	OscScaleEngine* scaleEngine = new OscScaleEngine();
	scaleEngine->setMajorTicksCount(numDivs + 1);
	this->setAxisScaleEngine(axis, (QwtScaleEngine*)scaleEngine);

	// Use a custom Scale Draw to control the drawing of axis values
	OscScaleDraw* scaleDraw = new OscScaleDraw(prefixFormatter, unit);
	scaleDraw->setFloatPrecision(floatPrecision);
	this->setAxisScaleDraw(axis, scaleDraw);
	if (axisPos == QwtPlot::yLeft) {
		// yLeft 0 has a different position than the rest, so we
		// give it a bigger minimum extent in order to align it with
		// the other yLeft axes.
		if (axisIdx == 0)
			scaleDraw->setMinimumExtent(94);
		else
			scaleDraw->setMinimumExtent(65);
	}
}

void TimeDomainDisplayPlot::cleanUpJustBeforeChannelRemoval(int) {}

void TimeDomainDisplayPlot::cancelZoom()
{
	for (unsigned int i = 0; i < d_zoomer.size(); ++i) {
		OscPlotZoomer* zoomer = static_cast<OscPlotZoomer*>(d_zoomer[i]);
		zoomer->resetZoom();
	}
}

long TimeDomainDisplayPlot::dataStartingPoint() const { return d_data_starting_point; }

void TimeDomainDisplayPlot::resetXaxisOnNextReceivedData()
{
	for (int i = 0; i < d_sink_reset_x_axis_pts.size(); i++)
		d_sink_reset_x_axis_pts[i] = true;
}

void TimeDomainDisplayPlot::setDataStartingPoint(long pos) { d_data_starting_point = pos; }

void TimeDomainDisplayPlot::setLineWidthF(int which, qreal widthF)
{
	if (which < d_plot_curve.size()) {
		QPen pen(d_plot_curve[which]->pen());
		pen.setWidthF(widthF);
		d_plot_curve[which]->setPen(pen);
	}
}

qreal TimeDomainDisplayPlot::getLineWidthF(int which) const
{
	if (which < d_plot_curve.size())
		return d_plot_curve[which]->pen().widthF();
	else
		return 0;
}

void TimeDomainDisplayPlot::hideCurvesUntilNewData()
{
	for (int i = 0; i < d_plot_curve.size(); i++)
		d_plot_curve.at(i)->hide();
	d_curves_hidden = true;
}

#endif /* TIME_DOMAIN_DISPLAY_PLOT_C */
