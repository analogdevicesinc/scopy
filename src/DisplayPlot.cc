/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
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

#include "DisplayPlot.h"
#include "osc_scale_engine.h"

#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_legend.h>
#include <qwt_plot_layout.h>

#include <QColor>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <QDebug>

using namespace adiscope;

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv& from_scaleDiv);

/*
 * OscScaleDraw class implementation
 */

OscScaleDraw::OscScaleDraw(const QString &unit) : QwtScaleDraw(),
	m_floatPrecision(3),
	m_unit(unit),
	m_formatter(NULL),
	m_color(Qt::gray),
	m_displayScale(1),
	m_shouldDrawMiddleDelta(false),
	m_nrTicks(0),
	m_delta(false)
{
	enableComponent(QwtAbstractScaleDraw::Backbone, false);
	enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

OscScaleDraw::OscScaleDraw(PrefixFormatter *formatter, const QString& unit) :
	OscScaleDraw(unit)
{
	m_formatter = formatter;
}

void OscScaleDraw::setFloatPrecision(unsigned int numDigits)
{
	m_floatPrecision = numDigits;
}

unsigned int OscScaleDraw::getFloatPrecison() const
{
	return m_floatPrecision;
}

void OscScaleDraw::setUnitType(const QString& unit)
{
	if (m_unit != unit) {
		m_unit = unit;

		// Trigger a new redraw of scale labels since there's a new unit that needs to be redrawn
		invalidateCache();
	}
}

QString OscScaleDraw::getUnitType() const
{
	return m_unit;
}

void OscScaleDraw::setColor(QColor color)
{
	m_color = color;
}

void OscScaleDraw::setDisplayScale(double value)
{
	m_displayScale = value;
}


void OscScaleDraw::setFormatter(PrefixFormatter *formatter)
{
	m_formatter = formatter;
}

void OscScaleDraw::enableDeltaLabel(bool enable)
{
	if (enable != m_delta) {
		m_delta = enable;

		// Trigger a new redraw of the scale
		invalidateCache();
	}
}

void OscScaleDraw::draw(QPainter *painter, const QPalette &) const
{
	int nrMajorTicks = scaleDiv().ticks(QwtScaleDiv::MajorTick).size();

	m_nrTicks = nrMajorTicks;

	QList<double> ticks = scaleDiv().ticks(QwtScaleDiv::MajorTick);
	QList<QRect> labels;

	for (int i = 0; i < ticks.size(); ++i){
		QRect bounds = boundingLabelRect(painter->font(), ticks[i]);
		int half = painter->font().pointSize() / 4;

		if (orientation() == Qt::Horizontal)
			bounds.adjust(-half, 0, half, 0);
		else
			bounds.adjust(0, -half / 2, 0, half / 2);

		labels.append(bounds);
	}

	bool overlap = false;

	int midLabelPos = nrMajorTicks / 2;

	do {
		overlap = false;
		for(int i = 1; i < labels.size(); ++i){
			QRect last_rectangle = labels.at(i - 1);
			QRect current_rectangle = labels.at(i);

			if (current_rectangle.intersects(last_rectangle)){
				overlap = true;
				break;
			}
		}

		if (overlap) {
			if (m_delta) {
				// If the middle delta label is to be drawn we are sure that
				// ticks.size() is an odd number
				int center = midLabelPos;
				for (int i = center - 1; i >= 0; i -= 2) {
					// Remove the tick and make sure to update the center
					// label position
					ticks.removeAt(i);
					labels.removeAt(i);
					--center;
				}
				for (int j = center + 1; j < ticks.size(); j += 1) {
					ticks.removeAt(j);
					labels.removeAt(j);
				}
			} else {
				for (int i = 1; i < ticks.size(); ++i) {
					ticks.removeAt(i);
					labels.removeAt(i);
				}
			}
		}

	} while (overlap);

	double delta = -INFINITY;

	if (m_delta && m_nrTicks > midLabelPos) {
		delta = scaleDiv().ticks(QwtScaleDiv::MajorTick)[midLabelPos];
		drawLabel(painter, delta);
	}

	for (const auto &tick : ticks) {
		if (tick != delta) {
			drawLabel(painter, tick);
		}
	}
}


QwtText OscScaleDraw::label( double value ) const
{
	QString prefix;
	double scale = 1.0;
	QString sign = "";
	int bonusPrecision = 0;
	bool center = false;

	double lower = scaleDiv().interval().minValue();
	double upper = scaleDiv().interval().maxValue();
	double diff = upper - lower;
	double step = diff / (m_nrTicks ? (m_nrTicks - 1) : 1);

	int mid = (m_nrTicks / 2 + 1);

	if (m_delta) {
		int current = 0;
		while (value > (lower + current * step)) current++;
		int position = current + 1;

		if (position == mid){
			// center label with extra precision
			center = true;
			bonusPrecision = 1;
		} else if (position < mid){
			sign = "-";
			// negative delta label
			value = step * (mid - position);
		} else if (position > mid) {
			sign = "+";
			// positive delta label
			value = step * (position - mid);
		}
	}

	value *= m_displayScale;

	if (m_formatter) {
		m_formatter->getFormatAttributes(value, prefix, scale);
	}

	if (orientation() == Qt::Vertical) {
		double absVal = value > 0 ? value : -value;
		if (absVal > 1e-2 && prefix == "m") {
			scale = 1.0;
			prefix = "";
		} else if (absVal > 1e-5 && prefix == "μ") {
			scale = 1e-3;
			prefix = "m";
		} else if (absVal > 1e-8 && prefix == "n") {
			scale = 1e-6;
			prefix = "μ";
		} else if (absVal > 1e-11 && prefix == "p") {
			scale = 1e-9;
			prefix = "n";
		}
	}

	QwtText text(sign + QLocale().toString(value / scale, 'f', m_floatPrecision + bonusPrecision)
		+ ' ' + prefix + m_unit);

	if (m_color != Qt::gray)
		text.setColor(m_color);
	if (center) {
		text.setColor(QColor(255, 255,255));
	}

	return text;
}

/*
 * EdgelessPlotScaleItem class implementation
 */
EdgelessPlotScaleItem::EdgelessPlotScaleItem(
	QwtScaleDraw::Alignment alignment, const double pos ):
    QwtPlotScaleItem(alignment, pos)
{
}

void EdgelessPlotScaleItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
	QwtPlotScaleItem::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv),
					getEdgelessScaleDiv(yScaleDiv));
}

/*
 * EdgelessPlotGrid class implementation
 */
EdgelessPlotGrid::EdgelessPlotGrid():
    QwtPlotGrid()
{
}

void EdgelessPlotGrid::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
	QwtPlotGrid::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv),
					getEdgelessScaleDiv(yScaleDiv));
}

OscPlotZoomer::OscPlotZoomer(QWidget *parent, bool doReplot) :
	ExtendingPlotZoomer(parent, doReplot),
	lastIndex(-1)
{
}

void OscPlotZoomer::rescale()
{
	DisplayPlot *plt = static_cast<DisplayPlot *>(plot());

	if ( !plt )
	    return;

	if (!plt->canvas()->isVisible())
		return;

	const QStack<QRectF> &stack = zoomStack();
	int index = zoomRectIndex();
	const QRectF &rect = stack[index];
	bool isZoomOut = false;
	if ( rect != scaleRect() )
	{

	    double x1 = rect.left();
	    double x2 = rect.right();
	    double y1 = rect.top();
	    double y2 = rect.bottom();

	    if ( !plt->axisScaleDiv( xAxis() ).isIncreasing() ){
		qSwap( x1, x2 );
	    }

	    if ( !plt->axisScaleDiv( yAxis() ).isIncreasing() )
		qSwap( y1, y2 );

	    double width = fabs(x1 - x2);
	    double height = fabs(y1 - y2);

	    if (height == 0 || width == 0) {
		    return;
	    }

	    if (lastIndex < index) {
		    Q_EMIT zoomIn();
	    }
	    else {
		    Q_EMIT zoomOut();
		    isZoomOut = true;
	    }
	    lastIndex = index;

	    const bool doReplot = plt->autoReplot();
	    plt->setAutoReplot( false );

	    if (yAxis().id == 0) {
		plt->setHorizUnitsPerDiv(width / plt->xAxisNumDiv());
		plt->setHorizOffset(x1 + (width / 2));
	    }

	    plt->setVertUnitsPerDiv(height / plt->yAxisNumDiv(), yAxis().id);
	    plt->setVertOffset(y1 + (height / 2), yAxis().id);

	    plt->setAutoReplot( doReplot );

	    plt->replot();

	    Q_EMIT zoomFinished(isZoomOut);
	}
}

/*
 * PlotAxisConfiguration
 */
PlotAxisConfiguration::PlotAxisConfiguration(int axisPos, int axisIdx, DisplayPlot *plot):
	d_axis(QwtAxisId(axisPos, axisIdx)), d_plot(plot), d_hoverCursorShape(Qt::ArrowCursor)
{
	Qt::CursorShape shape;

	switch (axisPos) {
		case QwtPlot::yLeft:
			shape = Qt::CursorShape::SizeVerCursor;
		break;

		case QwtPlot::xBottom:
			shape = Qt::CursorShape::SizeHorCursor;
		break;

		default:
			return;
	}

	// Set cursor shape when hovering over the axis
	//setCursorShapeOnHover(shape);

	// Avoid jumping when labels with more/less digits
	// appear/disappear when scrolling vertically
	if (axisPos == QwtPlot::yLeft) {
		const QFontMetrics fm(d_plot->axisWidget(d_axis)->font());
		QwtScaleDraw *scaleDraw = d_plot->axisScaleDraw(d_axis);
		scaleDraw->setMinimumExtent( fm.width("100.00") );
	}

	// TO DO: Move this to a stylesheet file.
	QwtScaleWidget *scaleWidget = d_plot->axisWidget(d_axis);
	scaleWidget->setStyleSheet("background-color: none;");

	// This helps creating a fixed 5 X 5 grid
	d_plot->setAxisScale(d_axis, -5.0, 5.0, 1);

	d_ptsPerDiv = 1.0;
	d_offset = 0.0;

	if (axisPos == QwtPlot::yLeft) {
		d_mouseGestures = new VertMouseGestures(d_plot->axisWidget(d_axis), d_axis);
		d_mouseGestures->setEnabled(false);

		QObject::connect(this->d_mouseGestures, SIGNAL(wheelDown(int)),
			d_plot, SLOT(vertAxisScaleIncrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(wheelUp(int)),
			d_plot, SLOT(vertAxisScaleDecrease()));

		QObject::connect(this->d_mouseGestures, SIGNAL(upMovement(double)),
			d_plot, SLOT(onVertAxisOffsetDecrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(downMovement(double)),
			d_plot, SLOT(onVertAxisOffsetIncrease()));
	} else if (axisPos == QwtPlot::xBottom) {
		d_mouseGestures = new HorizMouseGestures(d_plot->axisWidget(d_axis), d_axis);
		d_mouseGestures->setEnabled(false);

		QObject::connect(this->d_mouseGestures, SIGNAL(wheelDown(int)),
			d_plot, SLOT(horizAxisScaleIncrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(wheelUp(int)),
			d_plot, SLOT(horizAxisScaleDecrease()));

		QObject::connect(this->d_mouseGestures, SIGNAL(rightMovement(double)),
			d_plot, SLOT(onHorizAxisOffsetDecrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(leftMovement(double)),
			d_plot, SLOT(onHorizAxisOffsetIncrease()));
	}
}

PlotAxisConfiguration::~PlotAxisConfiguration()
{
}

QwtAxisId& PlotAxisConfiguration::axis()
{
	return d_axis;
}

void PlotAxisConfiguration::setPtsPerDiv(double value)
{
	d_ptsPerDiv = value;
}

double PlotAxisConfiguration::ptsPerDiv()
{
	return d_ptsPerDiv;
}

void PlotAxisConfiguration::setOffset(double value)
{
	d_offset = value;
}

double PlotAxisConfiguration::offset()
{
	return d_offset;
}

void PlotAxisConfiguration::setCursorShapeOnHover(Qt::CursorShape shape)
{
	d_hoverCursorShape = shape;
	QwtScaleWidget *scaleWidget = d_plot->axisWidget(d_axis);
	scaleWidget->setCursor(shape);
}

void PlotAxisConfiguration::setMouseGesturesEnabled(bool en)
{
	d_mouseGestures->setEnabled(en);
	QwtScaleWidget *scaleWidget = d_plot->axisWidget(d_axis);
	if (en) {
		scaleWidget->setCursor(d_hoverCursorShape);
	} else {
		scaleWidget->setCursor(QCursor());
	}
}

/*
 * DisplayPlot class
 */

DisplayPlot::DisplayPlot(int nplots, QWidget* parent,
			 unsigned int xNumDivs, unsigned int yNumDivs)
  : PrintablePlot(parent), d_nplots(nplots), d_stop(false),
    d_coloredLabels(false), d_mouseGesturesEnabled(false),
    d_displayScale(1), d_xAxisNumDiv(1),
    d_yAxisNumDiv(1)
{
  d_CurveColors << QColor("#ff7200") << QColor("#9013fe") << QColor(Qt::green)
       << QColor(Qt::cyan) << QColor(Qt::magenta)
       << QColor(Qt::yellow) << QColor(Qt::gray) << QColor(Qt::darkRed)
       << QColor(Qt::darkGreen) << QColor(Qt::darkBlue) << QColor(Qt::darkGray)
       << QColor(Qt::black);

  d_printColors << QColor("#ff7200") << QColor("#9013fe") << QColor(Qt::darkGreen)
       << QColor(Qt::blue) << QColor(Qt::magenta)
       << QColor(Qt::darkRed);

  qRegisterMetaType<QColorList>("QColorList");
  resize(parent->width(), parent->height());

  d_autoscale_state = false;

  d_yAxisUnit = "";
  d_xAxisUnit = "";

  setXaxisNumDiv(xNumDivs);
  setYaxisNumDiv(yNumDivs);

  d_usingLeftAxisScales = true;

  // Disable polygon clipping
#if QWT_VERSION < 0x060000
  QwtPainter::setDeviceClipping(false);
#else
  QwtPainter::setPolylineSplitting(false);
#endif

#if QWT_VERSION < 0x060000
  // We don't need the cache here
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);
#endif

  QColor default_palette_color = QColor("white");
  setPaletteColor(default_palette_color);

  d_panner = new QwtPlotPanner(canvas());
  d_panner->setAxisEnabled(QwtPlot::yRight, false);
  d_panner->setMouseButton(Qt::MidButton, Qt::ControlModifier);

  // emit the position of clicks on widget
  d_picker = new QwtDblClickPlotPicker(canvas());

#if QWT_VERSION < 0x060000
  connect(d_picker, SIGNAL(selected(const QwtDoublePoint &)),
      this, SLOT(onPickerPointSelected(const QwtDoublePoint &)));
#else
  d_picker->setStateMachine(new QwtPickerDblClickPointMachine());
  connect(d_picker, SIGNAL(selected(const QPointF &)),
	  this, SLOT(onPickerPointSelected6(const QPointF &)));
#endif

  // Configure horizontal axis
  bottomHorizAxisInit();

  // One vertical axis by default
  setLeftVertAxesCount(1);

  setActiveVertAxis(0);

  plotLayout()->setAlignCanvasToScales(true);


  for (unsigned int i = 0; i < 4; i++) {
	QwtScaleDraw::Alignment scale =
		static_cast<QwtScaleDraw::Alignment>(i);
	auto scaleItem = new EdgelessPlotScaleItem(scale);

	scaleItem->scaleDraw()->setAlignment(scale);
	scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	scaleItem->setFont(this->axisWidget(QwtPlot::yLeft)->font());

	QPalette palette = scaleItem->palette();
	palette.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
	palette.setBrush(QPalette::Text, QColor("#6E6E6F"));
	scaleItem->setPalette(palette);
	scaleItem->setBorderDistance(0);
	scaleItem->attach(this);
	scaleItems.push_back(scaleItem);
  }

  this->plotLayout()->setCanvasMargin(0, QwtPlot::yLeft);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::yRight);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::xTop);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::xBottom);

  ((QFrame*) canvas())->setLineWidth(0);

  // Avoid jumping when labels with more/less digits
  // appear/disappear when scrolling vertically

  QwtLegend* legendDisplay = new QwtLegend(this);

#if QWT_VERSION < 0x060100
  legendDisplay->setItemMode(QwtLegend::CheckableItem);
  insertLegend(legendDisplay);
  connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),
	  this, SLOT(legendEntryChecked(QwtPlotItem *, bool)));
#else /* QWT_VERSION < 0x060100 */
  legendDisplay->setDefaultItemMode(QwtLegendData::Checkable);
  insertLegend(legendDisplay);
  connect(legendDisplay, SIGNAL(checked(const QVariant&, bool, int)),
	  this, SLOT(legendEntryChecked(const QVariant&, bool, int)));
#endif /* QWT_VERSION < 0x060100 */

  // Plot needs a grid
  d_grid = new EdgelessPlotGrid();
  QColor majorPenColor("#353537");
  d_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);
  d_grid->attach(this);
}

DisplayPlot::~DisplayPlot()
{
	// d_zoomer and d_panner deleted when parent deleted

	// Since some curves may not be attached to the plot they won't get deleted
	for (auto it = d_plot_curve.begin(); it != d_plot_curve.end() ; ++it) {
		QwtPlotCurve * qpc = (*it);
		qpc->detach();
		delete qpc;
	}

	// delete vertAxes
	for (auto it = vertAxes.begin(); it != vertAxes.end(); ++it) {
		delete *it;
	}

	for (auto it = scaleItems.begin(); it != scaleItems.end(); ++it) {
		delete *it;
	}

	delete d_grid;

	//delete horizAxis
	delete horizAxis;
}

void
DisplayPlot::disableLegend()
{
  // Haven't found a good way to toggle it on/off
  insertLegend(NULL);
}

void
DisplayPlot::setYaxis(double min, double max)
{
  setAxisScale(QwtPlot::yLeft, min, max);
  if(!d_autoscale_state) {
    for (unsigned int i = 0; i < d_zoomer.size(); ++i)
	    d_zoomer[i]->setZoomBase();
  }
}

void
DisplayPlot::setXaxis(double min, double max)
{
  setAxisScale(QwtPlot::xBottom, min, max);
  for (unsigned int i = 0; i < d_zoomer.size(); ++i)
	  d_zoomer[i]->setZoomBase();
}

void
DisplayPlot::setLineLabel(int which, QString label)
{
  d_plot_curve[which]->setTitle(label);
}

QString
DisplayPlot::getLineLabel(int which)
{
  return d_plot_curve[which]->title().text();
}

void DisplayPlot::setLineColor(int chnIdx, int colorIdx)
{
        if (colorIdx >= d_CurveColors.size()) {
                colorIdx = d_CurveColors.size() - 1;
        } else if (colorIdx < 0) {
                colorIdx = 0;
        }
        setLineColor(chnIdx, d_CurveColors.at(colorIdx));
}

void
DisplayPlot::setLineColor(int which, QColor color)
{
    if (which < d_plot_curve.size()) {
    // Set the color of the pen
    QPen pen(d_plot_curve[which]->pen());
    pen.setColor(color);
    d_plot_curve[which]->setPen(pen);
    // And set the color of the markers
#if QWT_VERSION < 0x060000
    //d_plot_curve[which]->setBrush(QBrush(QColor(color)));
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

QColor
DisplayPlot::getLineColor(int which) const
{
  // If that plot doesn't exist then return black.
  if (which < d_plot_curve.size())
    return d_plot_curve[which]->pen().color();
  return QColor("black");
}

// Use a preprocessor macro to create a bunch of hooks for Q_PROPERTY and hence the stylesheet.
#define SETUPLINE(i, im1) \
    void DisplayPlot::setLineColor ## i (QColor c) {setLineColor(im1, c);} \
    const QColor DisplayPlot::getLineColor ## i () const {return getLineColor(im1);} \
    void DisplayPlot::setLineWidth ## i (int width) {setLineWidth(im1, width);} \
    int DisplayPlot::getLineWidth ## i () const {return getLineWidth(im1);} \
    void DisplayPlot::setLineStyle ## i (Qt::PenStyle ps) {setLineStyle(im1, ps);} \
    const Qt::PenStyle DisplayPlot::getLineStyle ## i () const {return getLineStyle(im1);} \
    void DisplayPlot::setLineMarker ## i (QwtSymbol::Style ms) {setLineMarker(im1, ms);} \
    const QwtSymbol::Style DisplayPlot::getLineMarker ## i () const {return getLineMarker(im1);} \
    void DisplayPlot::setMarkerAlpha ## i (int alpha) {setMarkerAlpha(im1, alpha);} \
    int DisplayPlot::getMarkerAlpha ## i () const {return getMarkerAlpha(im1);}
SETUPLINE(1, 0)
SETUPLINE(2, 1)
SETUPLINE(3, 2)
SETUPLINE(4, 3)
SETUPLINE(5, 4)
SETUPLINE(6, 5)
SETUPLINE(7, 6)
SETUPLINE(8, 7)
SETUPLINE(9, 8)

void
DisplayPlot::setZoomerColor(QColor c) {
  for (unsigned int i = 0; i < d_zoomer.size(); ++i) {
	d_zoomer[i]->setRubberBandPen(c);
	d_zoomer[i]->setTrackerPen(c);
  }
}

QColor
DisplayPlot::getZoomerColor() const {
  return d_zoomer[0]->rubberBandPen().color();
}

void
DisplayPlot::setPaletteColor(QColor c) {
  QPalette palette;
  palette.setColor(canvas()->backgroundRole(), c);
  canvas()->setPalette(palette);
}

QColor
DisplayPlot::getPaletteColor() const {
  return canvas()->palette().color(canvas()->backgroundRole());
}

void
DisplayPlot::setAxisLabelFontSize(int axisId, int fs) {
  QwtText axis_title = QwtText(axisWidget(axisId)->title());
  QFont font = QFont(axis_title.font());
  font.setPointSize(fs);
  axis_title.setFont(font);
  axisWidget(axisId)->setTitle(axis_title);
}

int
DisplayPlot::getAxisLabelFontSize(int axisId) const {
  return axisWidget(axisId)->title().font().pointSize();
}

void
DisplayPlot::setYaxisLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::yLeft, fs);
}

void
DisplayPlot::printWithNoBackground(const QString& toolName, bool editScaleDraw)
{
        OscScaleDraw *scaleDraw = static_cast<OscScaleDraw *>(this->axisScaleDraw(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis)));
        QStack<QColor> colors;
        for (int i = 0; i < d_plot_curve.size(); ++i) {
                colors.push_back(getLineColor(i));
                setLineColor(i, d_printColors[i]);
        }

        if (editScaleDraw) {
                scaleDraw->setColor(getLineColor(d_activeVertAxis));
                scaleDraw->invalidateCache();
        }

        PrintablePlot::printPlot(toolName);

        for (int i = d_plot_curve.size() - 1; i >= 0; --i) {
                setLineColor(i, colors.pop());
        }
        if (editScaleDraw) {
                scaleDraw->setColor(getLineColor(d_activeVertAxis));
                scaleDraw->invalidateCache();
        }
}

int
DisplayPlot::getYaxisLabelFontSize() const {
  int fs = getAxisLabelFontSize(QwtPlot::yLeft);
  return fs;
}

void
DisplayPlot::setXaxisLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::xBottom, fs);
}

int
DisplayPlot::getXaxisLabelFontSize() const {
  int fs = getAxisLabelFontSize(QwtPlot::xBottom);
  return fs;
}

void
DisplayPlot::setAxesLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::yLeft, fs);
  setAxisLabelFontSize(QwtPlot::xBottom, fs);
}

int
DisplayPlot::getAxesLabelFontSize() const {
  // Returns 0 if all axes do not have the same font size.
  int fs = getAxisLabelFontSize(QwtPlot::yLeft);
  if (getAxisLabelFontSize(QwtPlot::xBottom) != fs)
    return 0;
  return fs;
}

void
DisplayPlot::setLineWidth(int which, int width)
{
  if(which < d_nplots) {
    // Set the new line width
    QPen pen(d_plot_curve[which]->pen());
    pen.setWidth(width);
    d_plot_curve[which]->setPen(pen);

    // Scale the marker size proportionally
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    sym.setSize(7+10*log10(1.0*width), 7+10*log10(1.0*width));
    d_plot_curve[which]->setSymbol(sym);
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setSize(7+10*log10(1.0*width), 7+10*log10(1.0*width));
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

int
DisplayPlot::getLineWidth(int which) const {
  if (which < d_nplots) {
    return d_plot_curve[which]->pen().width();
  }
  else {
    return 0;
  }
}

void
DisplayPlot::setLineStyle(int which, Qt::PenStyle style)
{
  if(which < d_nplots) {
    QPen pen(d_plot_curve[which]->pen());
    pen.setStyle(style);
    d_plot_curve[which]->setPen(pen);
  }
}

const Qt::PenStyle
DisplayPlot::getLineStyle(int which) const
{
  if(which < d_nplots) {
    return d_plot_curve[which]->pen().style();
  }
  else {
    return Qt::SolidLine;
  }
}

void
DisplayPlot::setLineMarker(int which, QwtSymbol::Style marker)
{
  if(which < d_nplots) {
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    QPen pen(d_plot_curve[which]->pen());
    QBrush brush(pen.color());
    sym.setStyle(marker);
    sym.setPen(pen);
    sym.setBrush(brush);
    d_plot_curve[which]->setSymbol(sym);
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setStyle(marker);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

const QwtSymbol::Style
DisplayPlot::getLineMarker(int which) const
{
  if(which < d_nplots) {
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    return sym.style();
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    return sym->style();
#endif
  }
  else {
    return QwtSymbol::NoSymbol;
  }
}

void
DisplayPlot::setMarkerAlpha(int which, int alpha)
{
  if (which < d_nplots) {
    // Get the pen color
    QPen pen(d_plot_curve[which]->pen());
    QColor color = pen.color();

    // Set new alpha and update pen
    color.setAlpha(alpha);
    pen.setColor(color);
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
DisplayPlot::getMarkerAlpha(int which) const
{
  if(which < d_nplots) {
    return d_plot_curve[which]->pen().color().alpha();
  }
  else {
    return 0;
  }
}

void
DisplayPlot::setStop(bool on)
{
  d_stop = on;
}

void
DisplayPlot::resizeSlot( QSize *s )
{
  // -10 is to spare some room for the legend and x-axis label
  resize(s->width()-10, s->height()-10);
}

void DisplayPlot::legendEntryChecked(QwtPlotItem* plotItem, bool on)
{
  plotItem->setVisible(!on);
  replot();
}

void DisplayPlot::legendEntryChecked(const QVariant &plotItem, bool on, int index)
{
#if QWT_VERSION < 0x060100
  std::runtime_error("DisplayPlot::legendEntryChecked with QVariant not enabled in this version of QWT.\n");
#else
  QwtPlotItem *p = infoToItem(plotItem);
  legendEntryChecked(p, on);
#endif /* QWT_VERSION < 0x060100 */
}

void
DisplayPlot::onPickerPointSelected(const QwtDoublePoint & p)
{
  QPointF point = p;
  //fprintf(stderr,"onPickerPointSelected %f %f\n", point.x(), point.y());
  Q_EMIT plotPointSelected(point);
}

void
DisplayPlot::onPickerPointSelected6(const QPointF & p)
{
  QPointF point = p;
  //fprintf(stderr,"onPickerPointSelected %f %f\n", point.x(), point.y());
  Q_EMIT plotPointSelected(point);
}

void DisplayPlot::zoomBaseUpdate()
{
	for (unsigned int i = 0; i < d_zoomer.size(); ++i)
		d_zoomer[i]->setZoomBase(true);
}

void DisplayPlot::AddAxisOffset(int axisPos, int axisIdx, double offset)
{
	double min = 0;
	double max = 1;
	double ptsPerDiv = 1;

	switch (axisPos) {
	case QwtPlot::yLeft:
		ptsPerDiv = vertAxes[axisIdx]->ptsPerDiv();
		min = d_yAxisMin * ptsPerDiv;
		max = d_yAxisMax * ptsPerDiv;
		break;
	case QwtPlot::xBottom:
		ptsPerDiv = horizAxis->ptsPerDiv();
		min = d_xAxisMin * ptsPerDiv;
		max = d_xAxisMax * ptsPerDiv;
		break;
	}

	QwtAxisId axisId(axisPos, axisIdx);

	setAxisScale(axisId, min + offset, max + offset,
			axisStepSize(axisId));
}

void DisplayPlot::setVertOffset(double offset, int axisIdx)
{
	AddAxisOffset(QwtPlot::yLeft, axisIdx, offset);
	vertAxes[axisIdx]->setOffset(offset);
}

double DisplayPlot::VertOffset(int axisIdx)
{
	if ((axisIdx >= 0) && (axisIdx < vertAxes.size()))
		return vertAxes[axisIdx]->offset();
	return 0;
}

void DisplayPlot::setHorizOffset(double offset)
{
	AddAxisOffset(QwtPlot::xBottom, 0, offset);
	horizAxis->setOffset(offset);
}

double DisplayPlot::HorizOffset()
{
	return horizAxis->offset();
}

void DisplayPlot::setVertUnitsPerDiv(double upd, int axisIdx)
{
	double min, max;
	double ptsPerDiv = vertAxes[axisIdx]->ptsPerDiv();
	double offset = vertAxes[axisIdx]->offset();

	if (ptsPerDiv != upd) {
		vertAxes[axisIdx]->setPtsPerDiv(upd);
		min = (d_yAxisMin * upd) + offset;
		max = (d_yAxisMax * upd) + offset;
		setAxisScale(QwtAxisId(QwtPlot::yLeft, axisIdx), min, max, upd);
		Q_EMIT vertScaleDivisionChanged(upd);
	}
}

double DisplayPlot::VertUnitsPerDiv(int axisIdx)
{
	return vertAxes[axisIdx]->ptsPerDiv();
}

void DisplayPlot::setHorizUnitsPerDiv(double upd)
{
	double min, max;
	double ptsPerDiv = horizAxis->ptsPerDiv();
	double offset = horizAxis->offset();

	if (ptsPerDiv != upd) {
		horizAxis->setPtsPerDiv(upd);
		min = (d_xAxisMin * upd) + offset;
		max = (d_xAxisMax * upd) + offset;
		setAxisScale(QwtPlot::xBottom, min, max, upd);
		Q_EMIT horizScaleDivisionChanged(upd);
	}
}

void DisplayPlot::enableColoredLabels(bool colored)
{
	d_coloredLabels = colored;
}

void DisplayPlot::enableMouseGesturesOnScales(bool enable)
{
	d_mouseGesturesEnabled = enable;
}

void DisplayPlot::setDisplayScale(double value)
{
	d_displayScale = value;
	OscScaleDraw *osd = nullptr;
	osd = static_cast<OscScaleDraw*>(axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->scaleDraw());
	osd->setDisplayScale(d_displayScale);
	osd->invalidateCache();
	axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->update();
}

void DisplayPlot::setActiveVertAxis(unsigned int axisIdx, bool selected)
{
	int numAxes = this->axesCount(QwtPlot::yLeft);

	if (axisIdx >= numAxes)
		return;

	d_activeVertAxis = axisIdx;

	if (d_usingLeftAxisScales && selected) {
		for (int i = 0; i < numAxes; i++) {
			this->setAxisVisible(QwtAxisId(QwtPlot::yLeft, i),
					(i == axisIdx));
		}
	}

	if (d_coloredLabels && selected) {
		OscScaleDraw *scaleDraw = static_cast<OscScaleDraw *>(this->axisScaleDraw(QwtAxisId(QwtPlot::yLeft, axisIdx)));
		scaleDraw->setColor(getLineColor(axisIdx));
		scaleDraw->invalidateCache();
	}
}

int DisplayPlot::activeVertAxis()
{
	return d_activeVertAxis;
}

double DisplayPlot::HorizUnitsPerDiv()
{
	return horizAxis->ptsPerDiv();
}

void DisplayPlot::DetachCurve(unsigned int curveIdx)
{
	if (curveIdx < this->d_plot_curve.size()) {
		this->d_plot_curve[curveIdx]->detach();
	}
}

void DisplayPlot::AttachCurve(unsigned int curveIdx)
{
	if (curveIdx < this->d_plot_curve.size()) {
		this->d_plot_curve[curveIdx]->attach((QwtPlot *)this);
	}
}

void DisplayPlot::bringCurveToFront(unsigned int curveIdx)
{
	DetachCurve(curveIdx);
	AttachCurve(curveIdx);
	replot();
}

QwtPlotCurve * DisplayPlot::Curve(unsigned int curveIdx)
{
	QwtPlotCurve *curve = NULL;

	if (curveIdx < this->d_plot_curve.size())
		curve = d_plot_curve[curveIdx];

	return curve;
}

void DisplayPlot::setMinXaxisDivision(double minDivison)
{
	this->d_hScaleDivisions.setLower(minDivison);
}

double DisplayPlot::minXaxisDivision()
{
	return this->d_hScaleDivisions.lower();
}

void DisplayPlot::setMaxXaxisDivision(double maxDivison)
{
	this->d_hScaleDivisions.setUpper(maxDivison);
}

double DisplayPlot::maxXaxisDivision()
{
	return this->d_hScaleDivisions.upper();
}

void DisplayPlot::setMinYaxisDivision(double minDivison)
{
	this->d_vScaleDivisions.setLower(minDivison);
}

double DisplayPlot::minYaxisDivision()
{
	return this->d_vScaleDivisions.lower();
}

void DisplayPlot::setMaxYaxisDivision(double maxDivision)
{
	this->d_vScaleDivisions.setUpper(maxDivision);
}

double DisplayPlot::maxYaxisDivision()
{
	return this->d_vScaleDivisions.upper();
}

void DisplayPlot::onHorizAxisOffsetDecrease()
{
	double offset = this->HorizOffset();
	double scale = this->HorizUnitsPerDiv();

	offset -= scale / xAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / xAxisNumDiv() ) == 0 )
		offset = 0;

	this->setHorizOffset(offset);
	this->replot();
	Q_EMIT horizScaleOffsetChanged(offset);
}

void DisplayPlot::onHorizAxisOffsetIncrease()
{
	double offset = this->HorizOffset();
	double scale = this->HorizUnitsPerDiv();

	offset += scale / xAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / xAxisNumDiv() ) == 0 )
		offset = 0;

	this->setHorizOffset(offset);
	this->replot();
	Q_EMIT horizScaleOffsetChanged(offset);
}

void DisplayPlot::onVertAxisOffsetDecrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double offset = this->VertOffset(osc_adj->axisId().id);
	double scale = this->VertUnitsPerDiv(osc_adj->axisId().id);

	offset -= scale / yAxisNumDiv();
	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / yAxisNumDiv() ) == 0 )
		offset = 0;

	this->setVertOffset(offset, osc_adj->axisId().id);
	this->replot();
	Q_EMIT vertScaleOffsetChanged(offset);
}

void DisplayPlot::onVertAxisOffsetIncrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double offset = this->VertOffset(osc_adj->axisId().id);
	double scale = this->VertUnitsPerDiv(osc_adj->axisId().id);

	offset += scale / yAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / yAxisNumDiv() ) == 0 )
		offset = 0;

	this->setVertOffset(offset, osc_adj->axisId().id);
	this->replot();

	Q_EMIT vertScaleOffsetChanged(offset);
}

void DisplayPlot::_onXbottomAxisWidgetScaleDivChanged()
{
	QwtScaleWidget *axis_widget = dynamic_cast<QwtScaleWidget *>(QObject::sender());
	OscScaleDraw *scale_draw = dynamic_cast<OscScaleDraw *>(axis_widget->scaleDraw());

	if (scale_draw) {
		scale_draw->invalidateCache();
		axis_widget->update();
	}
}

void DisplayPlot::_onYleftAxisWidgetScaleDivChanged()
{
	QwtScaleWidget *axis_widget = dynamic_cast<QwtScaleWidget *>(QObject::sender());
	OscScaleDraw *scale_draw = dynamic_cast<OscScaleDraw *>(axis_widget->scaleDraw());

	if (scale_draw) {
		scale_draw->invalidateCache();
		axis_widget->update();
	}
}

QwtPlotZoomer *DisplayPlot::getZoomer() const
{
	if (d_zoomer.isEmpty())
		return nullptr;

	return d_zoomer[0];
}

void DisplayPlot::horizAxisScaleIncrease()
{
	double div = HorizUnitsPerDiv();
	double newDiv = d_hScaleDivisions.getNumberAfter(div);

	if (newDiv != div) {
		setHorizUnitsPerDiv(newDiv);
		replot();
	}
}

void DisplayPlot::horizAxisScaleDecrease()
{
	double div = HorizUnitsPerDiv();
	double newDiv = d_hScaleDivisions.getNumberBefore(div);

	if (newDiv != div) {
		setHorizUnitsPerDiv(newDiv);
		replot();
	}
}

void DisplayPlot::vertAxisScaleIncrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double div = VertUnitsPerDiv(osc_adj->axisId().id);
	double newDiv = d_vScaleDivisions.getNumberAfter(div);

	if (newDiv != div) {
		setVertUnitsPerDiv(newDiv, osc_adj->axisId().id);
		replot();
	}
}

void DisplayPlot::vertAxisScaleDecrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double div = VertUnitsPerDiv(osc_adj->axisId().id);
	double newDiv = d_vScaleDivisions.getNumberBefore(div);

	if (newDiv != div) {
		setVertUnitsPerDiv(newDiv, osc_adj->axisId().id);
		replot();
	}
}

void DisplayPlot::removeLeftVertAxis(unsigned int axis)
{
	const unsigned int numAxis = vertAxes.size();

	if (axis >= numAxis)
		return;

	setAxesCount(QwtPlot::yLeft, numAxis - 1);

	for (unsigned int i = axis + 1; i < numAxis; i++)
		vertAxes[i]->axis().id = i - 1;

	delete vertAxes[axis];
	vertAxes.erase(vertAxes.begin() + axis, vertAxes.begin() + axis + 1);

	for (unsigned int i = axis; i < numAxis - 1; i++) {
		double ptsPerDiv = vertAxes[i]->ptsPerDiv();
		double offset = vertAxes[i]->offset();

		setAxisScale(QwtAxisId(QwtPlot::yLeft, i),
				(d_yAxisMin * ptsPerDiv) + offset,
				(d_yAxisMax * ptsPerDiv) + offset,
				ptsPerDiv);
	}
}

void DisplayPlot::setLeftVertAxesCount(int count)
{
	setAxesCount(QwtPlot::yLeft, count);

	const int numAxis = vertAxes.size();

	for (int i = count; i < numAxis; i++) {
		delete vertAxes[i];
	}

	vertAxes.resize(count);
	for (int i = numAxis; i < count; i++) {
		vertAxes[i] = new PlotAxisConfiguration(QwtPlot::yLeft, i, this);
		configureAxis(QwtPlot::yLeft, i);
		this->setAxisVisible(QwtAxisId(QwtPlot::yLeft, i),
			d_usingLeftAxisScales);
		connect(axisWidget(vertAxes[i]->axis()), SIGNAL(scaleDivChanged()),
			      this, SLOT(_onYleftAxisWidgetScaleDivChanged()));
	}
}

int DisplayPlot::leftVertAxesCount()
{
	return vertAxes.size();
}

void DisplayPlot::setUsingLeftAxisScales(bool on)
{
	d_usingLeftAxisScales = on;
}

bool DisplayPlot::usingLeftAxisScales()
{
	return d_usingLeftAxisScales;
}

void DisplayPlot::configureAxis(int axisPos, int axisIdx)
{
	QwtAxisId axis(axisPos, axisIdx);

	// Use a custom Scale Engine to keep the grid fixed
	OscScaleEngine *scaleEngine = new OscScaleEngine();
	this->setAxisScaleEngine(axis, (QwtScaleEngine *)scaleEngine);

	// Use a custom Scale Draw to control the drawing of axis values
	OscScaleDraw *scaleDraw = new OscScaleDraw();
	this->setAxisScaleDraw(axis, scaleDraw);
}

void DisplayPlot::resizeEvent(QResizeEvent *event)
{
	PrintablePlot::resizeEvent(event);

	Q_EMIT plotSizeChanged();
}

void DisplayPlot::bottomHorizAxisInit()
{
	horizAxis = new PlotAxisConfiguration(QwtPlot::xBottom, 0, this);
	horizAxis->setMouseGesturesEnabled(d_mouseGesturesEnabled);
	configureAxis(QwtPlot::xBottom, 0);
	connect(axisWidget(horizAxis->axis()), SIGNAL(scaleDivChanged()),
		      this, SLOT(_onXbottomAxisWidgetScaleDivChanged()));
}

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv& from_scaleDiv)
{
	double lowerBound;
	double upperBound;
	QList<double> minorTicks;
	QList<double> mediumTicks;
	QList<double> majorTicks;

	lowerBound = from_scaleDiv.lowerBound();
	upperBound = from_scaleDiv.upperBound();
	minorTicks = from_scaleDiv.ticks(QwtScaleDiv::MinorTick);
	mediumTicks = from_scaleDiv.ticks(QwtScaleDiv::MediumTick);
	majorTicks = from_scaleDiv.ticks(QwtScaleDiv::MajorTick);
	if (majorTicks.size() >= 2) {
		majorTicks.erase(majorTicks.begin());
		majorTicks.erase(majorTicks.end() - 1);
	}
	return QwtScaleDiv(lowerBound, upperBound, minorTicks, mediumTicks, majorTicks);
}

unsigned int DisplayPlot::xAxisNumDiv()
{
	return d_xAxisNumDiv;
}

void DisplayPlot::setXaxisNumDiv(unsigned int num)
{
	if (d_xAxisNumDiv != num) {
		d_xAxisNumDiv = num;
		d_xAxisMin = -(double)num / 2;
		d_xAxisMax = (double)num / 2;
	}
}

unsigned int DisplayPlot::yAxisNumDiv()
{
	return d_yAxisNumDiv;
}

void DisplayPlot::setYaxisNumDiv(unsigned int num)
{
	if (d_yAxisNumDiv != num) {
		d_yAxisNumDiv = num;
		d_yAxisMin = -(double)num / 2;
		d_yAxisMax = (double)num / 2;
	}
}

void DisplayPlot::setXaxisMouseGesturesEnabled(bool en)
{
	horizAxis->setMouseGesturesEnabled(en);
}

void DisplayPlot::setYaxisMouseGesturesEnabled(int axisId, bool en)
{
	if (axisId < vertAxes.size()) {
		vertAxes[axisId]->setMouseGesturesEnabled(en);
	}
}
