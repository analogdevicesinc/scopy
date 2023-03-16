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
#include <qwt_math.h>

#include <QStack>
#include <QPainter>
#include <QColor>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <QDebug>
#include <QIcon>
#include <QGridLayout>
#include <qwt_plot_opengl_canvas.h>

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

	for (const auto &tick : qAsConst(ticks)) {
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
		case QwtAxis::YLeft:
			shape = Qt::CursorShape::SizeVerCursor;
		break;

		case QwtAxis::XBottom:
			shape = Qt::CursorShape::SizeHorCursor;
		break;

		default:
			return;
	}

	// Set cursor shape when hovering over the axis
	//setCursorShapeOnHover(shape);

	// Avoid jumping when labels with more/less digits
	// appear/disappear when scrolling vertically
	if (axisPos == QwtAxis::YLeft) {
		const QFontMetrics fm(d_plot->axisWidget(d_axis)->font());
		QwtScaleDraw *scaleDraw = d_plot->axisScaleDraw(d_axis);
		scaleDraw->setMinimumExtent( fm.horizontalAdvance("100.00") );
	}

	// TO DO: Move this to a stylesheet file.
	QwtScaleWidget *scaleWidget = d_plot->axisWidget(d_axis);
	scaleWidget->setStyleSheet("background-color: none;");

	// This helps creating a fixed 5 X 5 grid
	d_plot->setAxisScale(d_axis, -5.0, 5.0, 1);

	d_ptsPerDiv = 1.0;
	d_offset = 0.0;

	if (axisPos == QwtAxis::YLeft) {
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
	} else if (axisPos == QwtAxis::XBottom) {
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
DisplayPlot::DisplayPlot(int nplots, QWidget* parent,  bool isdBgraph,
			 unsigned int xNumDivs, unsigned int yNumDivs)
	: PrintablePlot(parent), d_nplots(nplots), d_stop(false),
	  d_coloredLabels(false), d_mouseGesturesEnabled(false),
	  d_displayScale(1), d_xAxisNumDiv(1), d_trackMode(false),
	  d_cursorsEnabled(false), d_isLogaritmicPlot(false),
	  d_isLogaritmicYPlot(false),
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
	d_panner->setAxisEnabled(QwtAxis::YRight, false);
	d_panner->setMouseButton(Qt::MiddleButton, Qt::ControlModifier);

	// emit the position of clicks on widget
	d_picker = new QwtDblClickPlotPicker(canvas());

#if QWT_VERSION < 0x060000
	connect(d_picker, SIGNAL(selected(const QPointF &)),
		this, SLOT(onPickerPointSelected(const QPointF &)));
#else
	d_picker->setStateMachine(new QwtPickerDblClickPointMachine());
	connect(d_picker, SIGNAL(selected(const QPointF &)),
		this, SLOT(onPickerPointSelected6(const QPointF &)));
#endif

	// Configure horizontal axis
	bottomHorizAxisInit();

  QColor plotColor;
  if (QIcon::themeName() == "scopy-default") {
	  plotColor = QColor("#6E6E6F");
  } else {
	  plotColor = QColor("#D3D3D3");
  }

	// One vertical axis by default
	setLeftVertAxesCount(1);

	setActiveVertAxis(0);

	plotLayout()->setAlignCanvasToScales(true);


	this->plotLayout()->setCanvasMargin(0, QwtAxis::YLeft);
	this->plotLayout()->setCanvasMargin(0, QwtAxis::YRight);
	this->plotLayout()->setCanvasMargin(0, QwtAxis::XTop);
	this->plotLayout()->setCanvasMargin(0, QwtAxis::XBottom);

	((QFrame*) canvas())->setLineWidth(0);

	//Set up the grid and the legend for all displayplots, but dBgraph
	setupDisplayPlotDiv(isdBgraph);

	d_symbolCtrl = new SymbolController(this);

	/* Adjacent areas */
	d_bottomHandlesArea = new HorizHandlesArea(this->canvas());
	d_rightHandlesArea = new VertHandlesArea(this->canvas());
	d_topHandlesArea = new HorizHandlesArea(this->canvas());
	d_leftHandlesArea = new VertHandlesArea(this->canvas());

	d_bottomHandlesArea->setMinimumHeight(50);
	d_rightHandlesArea->setMinimumWidth(50);
	d_bottomHandlesArea->setLargestChildWidth(60);
	d_rightHandlesArea->setLargestChildHeight(60);
	d_rightHandlesArea->setMinimumHeight(this->minimumHeight());

	d_formatter = static_cast<PrefixFormatter *>(new MetricPrefixFormatter);

	markerIntersection1 = new QwtPlotMarker();
	markerIntersection2 = new QwtPlotMarker();
	markerIntersection1->setSymbol(new QwtSymbol(
					       QwtSymbol::Ellipse, QColor(237, 28, 36),
					       QPen(QColor(255, 255 ,255, 140), 2, Qt::SolidLine),
					       QSize(5, 5)));
	markerIntersection2->setSymbol(new QwtSymbol(
					       QwtSymbol::Ellipse, QColor(237, 28, 36),
					       QPen(QColor(255, 255 ,255, 140), 2, Qt::SolidLine),
					       QSize(5, 5)));

	d_selected_channel = -1;

	setupCursors();
	setupReadouts();
}


void DisplayPlot::setupDisplayPlotDiv(bool isdBgraph) {
    if(!isdBgraph)
    {

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

          for (unsigned int i = 0; i < 4; i++) {
          QwtScaleDraw::Alignment scale =
              static_cast<QwtScaleDraw::Alignment>(i);
          auto scaleItem = new EdgelessPlotScaleItem(scale);

          scaleItem->scaleDraw()->setAlignment(scale);
          scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
          scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
		  scaleItem->setFont(this->axisWidget(QwtAxis::YLeft)->font());

          QPalette palette = scaleItem->palette();
	  palette.setBrush(QPalette::WindowText, QColor(0x6E6E6F));
	  palette.setBrush(QPalette::Text, QColor(0x6E6E6F));
          scaleItem->setPalette(palette);
          scaleItem->setBorderDistance(0);
          scaleItem->attach(this);
          scaleItems.push_back(scaleItem);
          scaleItem->setZ(200);
        }

        // Plot needs a grid
        d_grid = new EdgelessPlotGrid();
        QColor majorPenColor("#353537");
        d_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);
        d_grid->attach(this);
    }
}


DisplayPlot::~DisplayPlot()
{
	markerIntersection1->detach();
	markerIntersection2->detach();
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

	delete markerIntersection1;
	delete markerIntersection2;
	delete horizAxis;
}

QWidget* DisplayPlot::getPlotwithElements()
{
	QWidget* widget = new QWidget();
	QGridLayout *gridplot = new QGridLayout();

	gridplot->addWidget(topHandlesArea(), 0, 0, 1, 3);
	gridplot->addWidget(leftHandlesArea(), 0, 0, 3, 1);
	gridplot->addWidget(this, 1, 1, 1, 1);
	gridplot->addWidget(rightHandlesArea(), 0, 2, 3, 1);
	gridplot->addWidget(bottomHandlesArea(), 2, 0, 1, 3);

	gridplot->setVerticalSpacing(0);
	gridplot->setHorizontalSpacing(0);
	gridplot->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(gridplot);

	return widget;
}

void DisplayPlot::setCursorAxes(QwtAxisId fixed_axis, QwtAxisId mobile_axis)
{
	d_vBar1->setMobileAxis(fixed_axis);
	d_vBar1->setFixedAxis(mobile_axis);

	d_vBar2->setMobileAxis(fixed_axis);
	d_vBar2->setFixedAxis(mobile_axis);

	d_hBar1->setMobileAxis(mobile_axis);
	d_hBar1->setFixedAxis(fixed_axis);

	d_hBar2->setMobileAxis(mobile_axis);
	d_hBar2->setFixedAxis(fixed_axis);
}

void DisplayPlot::setupCursors() {

	d_vBar1 = new VertBar(this, true);
	d_vBar2 = new VertBar(this, true);
	d_hBar1 = new HorizBar(this, true);
	d_hBar2 = new HorizBar(this, true);

	d_vCursorHandle1 = new PlotLineHandleV(
				QPixmap(":/icons/v_cursor_handle.svg"),
				d_rightHandlesArea);
	d_vCursorHandle2 = new PlotLineHandleV(
				QPixmap(":/icons/v_cursor_handle.svg"),
				d_rightHandlesArea);
	d_hCursorHandle1 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);
	d_hCursorHandle2 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);

	d_vertCursorsHandleEnabled = true;

	d_symbolCtrl->attachSymbol(d_vBar1);
	d_symbolCtrl->attachSymbol(d_vBar2);
	d_symbolCtrl->attachSymbol(d_hBar1);
	d_symbolCtrl->attachSymbol(d_hBar2);

	QPen cursorsLinePen = QPen(QColor(155, 155, 155), 1, Qt::DashLine);
	d_hBar1->setPen(cursorsLinePen);
	d_hBar2->setPen(cursorsLinePen);
	d_vBar1->setPen(cursorsLinePen);
	d_vBar2->setPen(cursorsLinePen);

	d_vCursorHandle1->setPen(cursorsLinePen);
	d_vCursorHandle2->setPen(cursorsLinePen);
	d_hCursorHandle1->setPen(cursorsLinePen);
	d_hCursorHandle2->setPen(cursorsLinePen);

	d_vBar1->setVisible(false);
	d_vBar2->setVisible(false);
	d_hBar1->setVisible(false);
	d_hBar2->setVisible(false);

	d_vCursorHandle1->hide();
	d_vCursorHandle2->hide();
	d_hCursorHandle1->hide();
	d_hCursorHandle2->hide();

	vertCursorsLocked = false;
	horizCursorsLocked = false;

	/* When a handle position changes the bar follows */
	connect(d_vCursorHandle1, SIGNAL(positionChanged(int)),
		SLOT(onVertCursorHandle1Changed(int)));
	connect(d_vCursorHandle2, SIGNAL(positionChanged(int)),
		SLOT(onVertCursorHandle2Changed(int)));
	connect(d_hCursorHandle1, SIGNAL(positionChanged(int)),
		SLOT(onHorizCursorHandle1Changed(int)));
	connect(d_hCursorHandle2, SIGNAL(positionChanged(int)),
		SLOT(onHorizCursorHandle2Changed(int)));

	/* When bar position changes due to plot resizes update the handle */
	connect(d_hBar1, SIGNAL(pixelPositionChanged(int)),
		SLOT(onHbar1PixelPosChanged(int)));
	connect(d_hBar2, SIGNAL(pixelPositionChanged(int)),
		SLOT(onHbar2PixelPosChanged(int)));
	connect(d_vBar1, SIGNAL(pixelPositionChanged(int)),
		SLOT(onVbar1PixelPosChanged(int)));
	connect(d_vBar2, SIGNAL(pixelPositionChanged(int)),
		SLOT(onVbar2PixelPosChanged(int)));
}

void DisplayPlot::setupReadouts() {
	d_cursorReadoutsVisible = false;

	d_cursorReadouts = new CursorReadouts(this);
	d_cursorReadouts->setAxis(QwtAxis::XTop,QwtAxis::YLeft);
	d_cursorReadouts->setTopLeftStartingPoint(QPoint(8, 8));
	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	/* Update Cursor Readouts */
	connect(d_hBar1, SIGNAL(positionChanged(double)),
		SLOT(onHCursor1Moved(double)));
	connect(d_hBar2, SIGNAL(positionChanged(double)),
		SLOT(onHCursor2Moved(double)));
	connect(d_vBar1, SIGNAL(positionChanged(double)),
		SLOT(onVCursor1Moved(double)));
	connect(d_vBar2, SIGNAL(positionChanged(double)),
		SLOT(onVCursor2Moved(double)));
}

void DisplayPlot::onVertCursorHandle1Changed(int value)
{
	if (vertCursorsLocked) {
		int position2 = value - (pixelPosHandleVert1 - pixelPosHandleVert2);
		pixelPosHandleVert2 = position2;
		d_hBar2->setPixelPosition(position2);
	}
	pixelPosHandleVert1 = value;

	d_hBar1->setPixelPosition(value);
}

void DisplayPlot::onVertCursorHandle2Changed(int value)
{
	if (vertCursorsLocked) {
		int position1 = value + (pixelPosHandleVert1 - pixelPosHandleVert2);
		pixelPosHandleVert1 = position1;
		d_hBar1->setPixelPosition(position1);
	}
	pixelPosHandleVert2 = value;
	d_hBar2->setPixelPosition(value);
}


void DisplayPlot::onHorizCursorHandle1Changed(int value)
{
	if (horizCursorsLocked) {
		int position2 = value - (pixelPosHandleHoriz1 - pixelPosHandleHoriz2);
		pixelPosHandleHoriz2 = position2;
		d_vBar2->setPixelPosition(position2);
	}
	pixelPosHandleHoriz1 = value;
	d_vBar1->setPixelPosition(value);
}

void DisplayPlot::onHorizCursorHandle2Changed(int value)
{
	if (horizCursorsLocked) {
		int position1 = value + (pixelPosHandleHoriz1 - pixelPosHandleHoriz2);
		pixelPosHandleHoriz1 = position1;
		d_vBar1->setPixelPosition(position1);
	}
	pixelPosHandleHoriz2 = value;
	d_vBar2->setPixelPosition(value);
}

VertBar* DisplayPlot::vBar1()
{
	return d_vBar1;
}

VertBar* DisplayPlot::vBar2()
{
	return d_vBar2;
}

HorizHandlesArea* DisplayPlot::bottomHandlesArea()
{
	return d_bottomHandlesArea;
}

QWidget * DisplayPlot::rightHandlesArea()
{
	return d_rightHandlesArea;
}

QWidget * DisplayPlot::leftHandlesArea()
{
	return d_leftHandlesArea;
}

QWidget * DisplayPlot::topHandlesArea()
{
	return d_topHandlesArea;
}

QString DisplayPlot::formatXValue(double value, int precision) const
{
	return d_formatter->format(value, d_xAxisUnit, precision);
}

QString DisplayPlot::formatYValue(double value, int precision) const
{
	return d_formatter->format(value, d_yAxisUnit, precision);
}

void DisplayPlot::onHbar1PixelPosChanged(int pos)
{
	d_vCursorHandle1->setPositionSilenty(pos);
}

void DisplayPlot::onHbar2PixelPosChanged(int pos)
{
	d_vCursorHandle2->setPositionSilenty(pos);
}

void DisplayPlot::onVbar1PixelPosChanged(int pos)
{
	d_hCursorHandle1->setPositionSilenty(pos);
	displayIntersection();
}

void DisplayPlot::onVbar2PixelPosChanged(int pos)
{
	d_hCursorHandle2->setPositionSilenty(pos);
	displayIntersection();
}

struct cursorReadoutsText DisplayPlot::allCursorReadouts() const
{
	return d_cursorReadoutsText;
}

void DisplayPlot::onVCursor1Moved(double value)
{
	QString text;

	text = d_formatter->format(value, "", 3);
	d_cursorReadouts->setTimeCursor1Text(text);
	d_cursorReadoutsText.t1 = text;

	double diff = value - d_vBar2->plotCoord().y();
	text = d_formatter->format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void DisplayPlot::onVCursor2Moved(double value)
{
	QString text;

	text = d_formatter->format(value, "", 3);
	d_cursorReadouts->setTimeCursor2Text(text);
	d_cursorReadoutsText.t2 = text;

	double diff = d_vBar1->plotCoord().y() - value;
	text = d_formatter->format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}


void DisplayPlot::onHCursor1Moved(double value)
{
	QString text;
	bool error = false;

	value *= d_displayScale;
	text = d_formatter->format(value, "", 3);
	d_cursorReadouts->setVoltageCursor1Text(error ? "-" : text);
	d_cursorReadoutsText.v1 = error ? "-" : text;

	double valueCursor2 = d_hBar2->plotCoord().x();

	double diff = value - (valueCursor2 * d_displayScale) ;
	text = d_formatter->format(diff, "", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void DisplayPlot::onHCursor2Moved(double value)
{
	QString text;
	bool error = false;

	value *= d_displayScale;
	text = d_formatter->format(value, "", 3);
	d_cursorReadouts->setVoltageCursor2Text(error ? "-" : text);
	d_cursorReadoutsText.v2 = error ? "-" : text;

	double valueCursor1 = d_hBar1->plotCoord().x();

	double diff = (valueCursor1 * d_displayScale) - value;
	text = d_formatter->format(diff, "", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void DisplayPlot::setVertCursorsEnabled(bool en)
{
	if (d_vertCursorsEnabled != en) {
		d_vertCursorsEnabled = en;
		d_vBar1->setVisible(en);
		d_vBar2->setVisible(en);
		d_hCursorHandle1->setVisible(en);
		d_hCursorHandle2->setVisible(en);
		d_cursorReadouts->setTimeReadoutVisible(en &&
							d_cursorReadoutsVisible);
	}
}

void DisplayPlot::toggleCursors(bool en)
{
	if (d_cursorsEnabled != en) {
		d_cursorsEnabled = en;
		d_vBar1->setVisible(en);
		d_vBar2->setVisible(en);

		if(d_vertCursorsHandleEnabled)
		{
			d_hCursorHandle1->setVisible(en);
			d_hCursorHandle2->setVisible(en);
		}

		d_cursorReadouts->setTimeReadoutVisible(en);
		d_cursorReadouts->setVoltageReadoutVisible(en);

		if (en) {
			onVCursor1Moved(d_vBar1->plotCoord().x());
			onVCursor2Moved(d_vBar2->plotCoord().x());
		} else {
			markerIntersection1->detach();
			markerIntersection2->detach();
			replot();
		}
	}
}

bool DisplayPlot::isLogaritmicPlot() const
{
	return d_isLogaritmicPlot;
}

void DisplayPlot::setPlotLogaritmic(bool value)
{
	d_isLogaritmicPlot = value;
}

bool DisplayPlot::isLogaritmicYPlot() const
{
	return d_isLogaritmicYPlot;
}

void DisplayPlot::setPlotYLogaritmic(bool value)
{
	d_isLogaritmicYPlot = value;
}

void DisplayPlot::setVertCursorsHandleEnabled(bool en)
{
	d_vertCursorsHandleEnabled = en;
}

bool DisplayPlot::vertCursorsEnabled()
{
	return d_vertCursorsEnabled;
}

void DisplayPlot::setHorizCursorsEnabled(bool en)
{
	if (d_horizCursorsEnabled != en) {
		d_horizCursorsEnabled = en;
		d_hBar1->setVisible(en);
		d_hBar2->setVisible(en);
		d_vCursorHandle1->setVisible(en);
		d_vCursorHandle2->setVisible(en);
		d_cursorReadouts->setVoltageReadoutVisible(en &&
							   d_cursorReadoutsVisible);
	}
}

bool DisplayPlot::horizCursorsEnabled()
{
	return d_horizCursorsEnabled;
}

void DisplayPlot::setCursorReadoutsVisible(bool en)
{
	if (d_cursorReadoutsVisible != en) {
		d_cursorReadoutsVisible = en;
		d_cursorReadouts->setVoltageReadoutVisible(en &&
							   d_vertCursorsEnabled );
		d_cursorReadouts->setTimeReadoutVisible(en &&
							d_horizCursorsEnabled );
	}
}

void DisplayPlot::setHorizCursorsLocked(bool value)
{
	horizCursorsLocked = value;
}

void DisplayPlot::setVertCursorsLocked(bool value)
{
	vertCursorsLocked = value;
}

void DisplayPlot::setCursorReadoutsTransparency(int value)
{
	d_cursorReadouts->setTransparency(value);
}

void DisplayPlot::moveCursorReadouts(CustomPlotPositionButton::ReadoutsPosition position)
{
	d_cursorReadouts->moveToPosition(position);
}

void DisplayPlot::trackModeEnabled(bool enabled)
{
	d_trackMode = !enabled;
	if (d_horizCursorsEnabled) {
		d_hBar1->setVisible(enabled);
		d_hBar2->setVisible(enabled);
		d_vCursorHandle1->setVisible(enabled);
		d_vCursorHandle2->setVisible(enabled);
	}
	if (d_trackMode) {
		onVCursor1Moved(d_vBar1->plotCoord().x());
		onVCursor2Moved(d_vBar2->plotCoord().x());
		displayIntersection();
	} else {
		onHCursor1Moved(d_hBar1->plotCoord().y());
		onHCursor2Moved(d_hBar2->plotCoord().y());
		markerIntersection1->detach();
		markerIntersection2->detach();
		replot();
	}
}

void DisplayPlot::displayIntersection()
{
	if (!d_trackMode) {
		return;
	}

	double intersectionCursor1, intersectionCursor2;
	bool attachmk1 = true;
	bool attachmk2 = true;


	intersectionCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	intersectionCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());

	if (intersectionCursor1 == -1000000){
		attachmk1 = false;
	}
	if (intersectionCursor2 == -1000000) {
		attachmk2 = false;
	}

	bool value = isAxisValid(QwtAxisId(QwtAxis::YLeft, d_selected_channel));

	if(value)
	{
		markerIntersection1->setAxes(QwtAxis::XBottom, QwtAxisId(QwtAxis::YLeft, d_selected_channel));
		markerIntersection2->setAxes(QwtAxis::XBottom, QwtAxisId(QwtAxis::YLeft, d_selected_channel));
	}
	else
	{
		markerIntersection1->setAxes(QwtAxis::XBottom, QwtAxisId(QwtAxis::YLeft, 0));
		markerIntersection2->setAxes(QwtAxis::XBottom, QwtAxisId(QwtAxis::YLeft, 0));
	}

	markerIntersection1->setValue(d_vBar1->plotCoord().x(), intersectionCursor1);
	markerIntersection2->setValue(d_vBar2->plotCoord().x(), intersectionCursor2);

	if (attachmk1) {
		markerIntersection1->attach(this);
	} else {
		markerIntersection1->detach();
	}
	if (attachmk2) {
		markerIntersection2->attach(this);
	} else {
		markerIntersection2->detach();
	}

	replot();
}

double DisplayPlot::getHorizontalCursorIntersection(double time)
{
	int n = Curve(d_selected_channel)->data()->size();

	if (n == 0) {
		return -1;
	} else {
		double leftTime, rightTime, leftCustom, rightCustom;
		int rightIndex = -1;
		int leftIndex = -1;

		int left = 0;
		int right = n - 1;

		if (Curve(d_selected_channel)->data()->sample(right).x() < time ||
				Curve(d_selected_channel)->data()->sample(left).x() > time) {
			return -1;
		}

		while (left <= right) {
			int mid = (left + right) / 2;
			double xData = Curve(d_selected_channel)->data()->sample(mid).x();
			if (xData == time) {
				if (mid > 0) {
					leftIndex = mid - 1;
					rightIndex = mid;
				}
				break;
			} else if (xData < time) {
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if ((leftIndex == -1 || rightIndex == -1) && left > 0) {
			leftIndex = left - 1;
			rightIndex = left;
		}

		if (leftIndex == -1 || rightIndex == -1) {
			return -1;
		}

		leftTime = Curve(d_selected_channel)->data()->sample(leftIndex).x();
		rightTime = Curve(d_selected_channel)->data()->sample(rightIndex).x();

		leftCustom = Curve(d_selected_channel)->data()->sample(leftIndex).y();
		rightCustom = Curve(d_selected_channel)->data()->sample(rightIndex).y();

		double value = (rightCustom - leftCustom) / (rightTime - leftTime) *
				(time - leftTime) + leftCustom;

		return value;
	}
}

void DisplayPlot::repositionCursors()
{
	onVCursor1Moved(d_vBar1->plotCoord().x());
	onVCursor2Moved(d_vBar2->plotCoord().x());
	displayIntersection();
}

void
DisplayPlot::disableLegend()
{
  // Haven't found a good way to toggle it on/off
  insertLegend(NULL);
}

void
DisplayPlot::setAllYAxis(double min, double max)
{
	for (unsigned int i = 0; i < vertAxes.size(); ++i) {
		setAxisScale(QwtAxisId(QwtAxis::YLeft, i), min, max);
	}

	if (!d_autoscale_state) {
		for (int i = 0; i < d_zoomer.size(); ++i)
			d_zoomer[i]->setZoomBase();
	}
}

void
DisplayPlot::setYaxis(double min, double max)
{
  setAxisScale(QwtAxis::YLeft, min, max);
  if(!d_autoscale_state) {
    for (unsigned int i = 0; i < d_zoomer.size(); ++i)
	    d_zoomer[i]->setZoomBase();
  }
}

void
DisplayPlot::setXaxis(double min, double max)
{
  setAxisScale(QwtAxis::XBottom, min, max);
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
  setAxisLabelFontSize(QwtAxis::YLeft, fs);
}

void
DisplayPlot::printWithNoBackground(const QString& toolName, bool editScaleDraw)
{
		OscScaleDraw *scaleDraw = static_cast<OscScaleDraw *>(this->axisScaleDraw(QwtAxisId(QwtAxis::YLeft, d_activeVertAxis)));
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
  int fs = getAxisLabelFontSize(QwtAxis::YLeft);
  return fs;
}

void
DisplayPlot::setXaxisLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtAxis::XBottom, fs);
}

int
DisplayPlot::getXaxisLabelFontSize() const {
  int fs = getAxisLabelFontSize(QwtAxis::XBottom);
  return fs;
}

void
DisplayPlot::setAxesLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtAxis::YLeft, fs);
  setAxisLabelFontSize(QwtAxis::XBottom, fs);
}

int
DisplayPlot::getAxesLabelFontSize() const {
  // Returns 0 if all axes do not have the same font size.
  int fs = getAxisLabelFontSize(QwtAxis::YLeft);
  if (getAxisLabelFontSize(QwtAxis::XBottom) != fs)
    return 0;
  return fs;
}

void
DisplayPlot::setLineWidth(int which, qreal width)
{
  if(which < d_nplots) {
    // Set the new line width
    QPen pen(d_plot_curve[which]->pen());
    pen.setWidthF(width);
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
DisplayPlot::onPickerPointSelected(const QPointF & p)
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

void DisplayPlot::zoomBaseUpdate(bool force)
{
	for (unsigned int i = 0; i < d_zoomer.size(); ++i)
		d_zoomer[i]->setZoomBase(force);
}

void DisplayPlot::AddAxisOffset(int axisPos, int axisIdx, double offset)
{
	double min = 0;
	double max = 1;
	double ptsPerDiv = 1;

	switch (axisPos) {
	case QwtAxis::YLeft:
		ptsPerDiv = vertAxes[axisIdx]->ptsPerDiv();
		min = d_yAxisMin * ptsPerDiv;
		max = d_yAxisMax * ptsPerDiv;
		break;
	case QwtAxis::XBottom:
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
	AddAxisOffset(QwtAxis::YLeft, axisIdx, offset);
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
	AddAxisOffset(QwtAxis::XBottom, 0, offset);
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
		setAxisScale(QwtAxisId(QwtAxis::YLeft, axisIdx), min, max, upd);
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
		setAxisScale(QwtAxis::XBottom, min, max, upd);
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
	osd = static_cast<OscScaleDraw*>(axisWidget(QwtAxisId(QwtAxis::YLeft, d_activeVertAxis))->scaleDraw());
	osd->setDisplayScale(d_displayScale);
	osd->invalidateCache();
	axisWidget(QwtAxisId(QwtAxis::YLeft, d_activeVertAxis))->update();
}

void DisplayPlot::setActiveVertAxis(unsigned int axisIdx, bool selected)
{
	int numAxes = this->axesCount(QwtAxis::YLeft);

	if (axisIdx >= numAxes)
		return;

	d_activeVertAxis = axisIdx;

	if (d_usingLeftAxisScales && selected) {
		for (int i = 0; i < numAxes; i++) {
			this->setAxisVisible(QwtAxisId(QwtAxis::YLeft, i),
					(i == axisIdx));
		}
	}

	if (d_coloredLabels && selected) {
		OscScaleDraw *scaleDraw = static_cast<OscScaleDraw *>(this->axisScaleDraw(QwtAxisId(QwtAxis::YLeft, axisIdx)));
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
	displayIntersection();
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

#ifdef __ANDROID__
void DisplayPlot::mousePressEvent(QMouseEvent *event)
{
	if (event->type() == QEvent::MouseButtonDblClick) {
		for (unsigned int i = 0; i < d_zoomer.size(); ++i) {
			OscPlotZoomer *zoomer = static_cast<OscPlotZoomer *>(d_zoomer[i]);
			zoomer->popZoom();
		}
	}
}
#endif

QwtPlotZoomer *DisplayPlot::getZoomer() const
{
	if (d_zoomer.isEmpty())
		return nullptr;

	return d_zoomer[0];
}

void DisplayPlot::setZoomerParams(bool bounded, int maxStackDepth)
{
	if (d_zoomer.isEmpty()) {
		return;
	}

	auto zoomer = dynamic_cast<LimitedPlotZoomer*>(d_zoomer[0]);
	zoomer->setMaxStackDepth(maxStackDepth);
	zoomer->setBoundVertical(bounded);
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

	setAxesCount(QwtAxis::YLeft, numAxis - 1);

	for (unsigned int i = axis + 1; i < numAxis; i++)
		vertAxes[i]->axis().id = i - 1;

	delete vertAxes[axis];
	vertAxes.erase(vertAxes.begin() + axis, vertAxes.begin() + axis + 1);

	for (unsigned int i = axis; i < numAxis - 1; i++) {
		double ptsPerDiv = vertAxes[i]->ptsPerDiv();
		double offset = vertAxes[i]->offset();

		setAxisScale(QwtAxisId(QwtAxis::YLeft, i),
				(d_yAxisMin * ptsPerDiv) + offset,
				(d_yAxisMax * ptsPerDiv) + offset,
				ptsPerDiv);
	}
}

void DisplayPlot::setLeftVertAxesCount(int count)
{
	setAxesCount(QwtAxis::YLeft, count);

	const int numAxis = vertAxes.size();

	for (int i = count; i < numAxis; i++) {
		delete vertAxes[i];
	}

	vertAxes.resize(count);
	for (int i = numAxis; i < count; i++) {
		vertAxes[i] = new PlotAxisConfiguration(QwtAxis::YLeft, i, this);
		configureAxis(QwtAxis::YLeft, i);
		this->setAxisVisible(QwtAxisId(QwtAxis::YLeft, i),
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
	horizAxis = new PlotAxisConfiguration(QwtAxis::XBottom, 0, this);
	horizAxis->setMouseGesturesEnabled(d_mouseGesturesEnabled);
	configureAxis(QwtAxis::XBottom, 0);
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

void DisplayPlot::setXaxisMajorTicksPos(QList<double> ticks)
{
    d_majorTicks = ticks;
}

QList<double> DisplayPlot::getXaxisMajorTicksPos() const
{
    return d_majorTicks;
}

void DisplayPlot::setYaxisMajorTicksPos(QList<double> ticks)
{
    d_majorTicksY = ticks;
}

QList<double> DisplayPlot::getYaxisMajorTicksPos() const
{
    return d_majorTicksY;
}

unsigned int DisplayPlot::xAxisNumDiv() const
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

unsigned int DisplayPlot::yAxisNumDiv() const
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
