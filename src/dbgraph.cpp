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

#include "dbgraph.hpp"
#include "DisplayPlot.h"
#include "osc_scale_engine.h"
#include "x_axis_scale_zoomer.h"
#include "plotpickerwrapper.h"

#include <qwt_plot_layout.h>

using namespace adiscope;

void dBgraph::setupCursors()
{
	d_symbolCtrl = new SymbolController(this);

	d_vBar1 = new VertBar(this,true);
	d_vBar2 = new VertBar(this,true);
	d_plotBar = new VertBar(this, true);
	d_frequencyBar = new VertBar(this, true);
	d_symbolCtrl->attachSymbol(d_vBar1);
	d_symbolCtrl->attachSymbol(d_vBar2);
	d_symbolCtrl->attachSymbol(d_plotBar);
	d_symbolCtrl->attachSymbol(d_frequencyBar);

	QPen cursorsLinePen = QPen(QColor(155,155,155),1,Qt::DashLine);
	QPen plotLinePen = QPen(QColor(211, 211, 211, 50), 5, Qt::SolidLine);
	QPen frequencyLinePen = QPen(QColor(74, 100, 255, 150), 2, Qt::DashLine);

	d_vBar1->setPen(cursorsLinePen);
	d_vBar2->setPen(cursorsLinePen);
	d_vBar1->setVisible(false);
	d_vBar2->setVisible(false);
	d_plotBar->setVisible(false);

	d_frequencyBar->setPen(frequencyLinePen);
	d_frequencyBar->setVisible(true);
	d_frequencyBar->setMobileAxis(QwtPlot::xTop);
	d_frequencyBar->setPixelPosition(0);

	d_plotBar->setPen(plotLinePen);
	d_plotBar->setMobileAxis(QwtPlot::xTop);

	connect(d_vBar1, SIGNAL(pixelPositionChanged(int)),
		SLOT(onVbar1PixelPosChanged(int)));
	connect(d_vBar1, SIGNAL(pixelPositionChanged(int)),
		SLOT(onCursor1Moved(int)));

	connect(d_vBar2, SIGNAL(pixelPositionChanged(int)),
		SLOT(onVbar2PixelPosChanged(int)));
	connect(d_vBar2, SIGNAL(pixelPositionChanged(int)),
		SLOT(onCursor2Moved(int)));

	connect(d_frequencyBar, &VertBar::pixelPositionChanged,
		this, &dBgraph::frequencyBarPositionChanged);
}

void dBgraph::parametersOverrange(bool enable)
{
	if (enable) {
		d_plotBar->setPen(QPen(QColor(250, 0, 0, 50), 5, Qt::SolidLine));
	} else {
		d_plotBar->setPen(QPen(QColor(211, 211, 211, 50), 5, Qt::SolidLine));
	}
}

void dBgraph::setupReadouts()
{
	d_cursorReadouts = new CursorReadouts(this);
	d_cursorReadouts->setAxis(QwtPlot::xTop,QwtPlot::yLeft);
	d_cursorReadouts->setTopLeftStartingPoint(QPoint(8, 8));
	d_cursorReadouts->moveToPosition(CustomPlotPositionButton::topLeft);

	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	d_cursorReadouts->setTimeCursor1LabelText("F1= ");
	d_cursorReadouts->setTimeCursor2LabelText("F2= ");
	d_cursorReadouts->setVoltageCursor1LabelText("Mag1= ");
	d_cursorReadouts->setVoltageCursor2LabelText("Mag2= ");
	d_cursorReadouts->setDeltaVoltageLabelText("ΔMag= ");

	d_cursorReadouts->setFrequencyDeltaVisible(false);
	d_cursorReadouts->setTimeDeltaVisible(false);
	d_cursorReadouts->setTransparency(0);
}

dBgraph::dBgraph(QWidget *parent) : DisplayPlot(0, parent),
	curve("data"),
	reference("reference"),
	d_cursorsCentered(false),
	d_cursorsEnabled(false),
	xmin(10),
	xmax(10),
	ymin(10),
	ymax(10),
	d_plotPosition(0),
	numSamples(0),
	delta_label(false),
	d_plotBarEnabled(true)
{
	enableAxis(QwtPlot::xBottom, false);
	enableAxis(QwtPlot::xTop, true);

	setAxisAutoScale(QwtPlot::yLeft, false);
	setAxisAutoScale(QwtPlot::xTop, false);

	EdgelessPlotGrid *grid = new EdgelessPlotGrid;
	grid->setMajorPen(QColor("#353537"), 1.0, Qt::DashLine);
	grid->setXAxis(QwtPlot::xTop);
	grid->attach(this);

	plotLayout()->setAlignCanvasToScales(true);

	curve.attach(this);
	curve.setRenderHint(QwtPlotItem::RenderAntialiased);
	curve.setXAxis(QwtPlot::xTop);
	curve.setYAxis(QwtPlot::yLeft);

	reference.setRenderHint(QwtPlotItem::RenderAntialiased);
	reference.setXAxis(QwtPlot::xTop);
	reference.setYAxis(QwtPlot::yLeft);
	reference.setPen(Qt::red, 1.5);

	thickness = 1;

	OscScaleEngine *scaleLeft = new OscScaleEngine;
	scaleLeft->setMajorTicksCount(7);
	this->setAxisScaleEngine(QwtPlot::yLeft,
				 static_cast<QwtScaleEngine *>(scaleLeft));

	/* draw_x / draw_y: Outmost X / Y scales. Only draw the labels */
	formatter = static_cast<PrefixFormatter *>(new MetricPrefixFormatter);
	draw_x = new OscScaleDraw(formatter, "Hz");
	draw_x->setFloatPrecision(2);
	draw_x->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	draw_x->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	setAxisScaleDraw(QwtPlot::xTop, draw_x);

	draw_y = new OscScaleDraw("dB");
	draw_y->setFloatPrecision(2);
	draw_y->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	draw_y->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	draw_y->setMinimumExtent(50);
	setAxisScaleDraw(QwtPlot::yLeft, draw_y);

	useLogFreq(false);

	/* Create 4 scales within the plot itself. Only draw the ticks */
	for (unsigned int i = 0; i < 4; i++) {
		EdgelessPlotScaleItem *scaleItem = new EdgelessPlotScaleItem(
			static_cast<QwtScaleDraw::Alignment>(i));

		/* Top/bottom scales must be sync'd to xTop; left/right scales
		 * must be sync'd to yLeft */
		if (i < 2) {
			scaleItem->setXAxis(QwtPlot::xTop);
		} else {
			scaleItem->setYAxis(QwtPlot::yLeft);
		}

		scaleItem->scaleDraw()->enableComponent(
			QwtAbstractScaleDraw::Backbone, false);
		scaleItem->scaleDraw()->enableComponent(
			QwtAbstractScaleDraw::Labels, false);

		QPalette palette = scaleItem->palette();
		palette.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
		palette.setBrush(QPalette::Text, QColor("#6E6E6F"));
		scaleItem->setPalette(palette);
		scaleItem->setBorderDistance(0);
		scaleItem->attach(this);
	}

	zoomer = new XAxisScaleZoomer(canvas());
	zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
				Qt::RightButton);
	zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
				Qt::RightButton, Qt::ControlModifier);

	static_cast<QFrame *>(canvas())->setLineWidth(0);
	setContentsMargins(10, 10, 24, 20);

	picker = new PlotPickerWrapper(QwtPlot::xTop,QwtPlot::yLeft,this->canvas());

	QMargins margins = contentsMargins();
	margins.setBottom(0);
	setContentsMargins(margins);

	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xTop);
	const int fmw = QFontMetrics(scaleWidget->font()).width("XXXX.XX XX");
	scaleWidget->setMinBorderDist(fmw / 2, fmw / 2);

	markerIntersection1 = new QwtPlotMarker();
	markerIntersection2 = new QwtPlotMarker();
	markerIntersection1->setSymbol(new QwtSymbol(
					       QwtSymbol::Ellipse, QColor(237, 28, 36),
					       QPen(QColor(255, 255,255, 140), 2, Qt::SolidLine),
					       QSize(5, 5)));
	markerIntersection2->setSymbol(new QwtSymbol(
					       QwtSymbol::Ellipse, QColor(237, 28, 36),
					       QPen(QColor(255, 255,255, 140), 2, Qt::SolidLine),
					       QSize(5, 5)));

	markerIntersection1->setAxes(QwtPlot::xTop, QwtPlot::yLeft);
	markerIntersection2->setAxes(QwtPlot::xTop, QwtPlot::yLeft);

	setupCursors();

	setupReadouts();

}

dBgraph::~dBgraph()
{
	markerIntersection1->detach();
	markerIntersection2->detach();
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
	delete markerIntersection1;
	delete markerIntersection2;
	delete formatter;
	delete picker;
}

void dBgraph::replot()
{
    QwtPlot::replot();
}

void dBgraph::setAxesScales(double xmin, double xmax, double ymin, double ymax)
{
	setAxisScale(QwtPlot::xTop, xmin, xmax);
	setAxisScale(QwtPlot::yLeft, ymin, ymax);
}

void dBgraph::setAxesTitles(const QString& x, const QString& y)
{
	QwtText xTitle(x);
	QwtText yTitle(y);

	QFont font = axisTitle(QwtPlot::xTop).font();
	font.setWeight(QFont::Normal);
	xTitle.setFont(font);
	yTitle.setFont(font);

	setAxisTitle(QwtPlot::xTop, xTitle);
	setAxisTitle(QwtPlot::yLeft, yTitle);
}

void dBgraph::plot(double x, double y)
{
	if (!d_plotBar->isVisible() && !xdata.size()) {
		if (d_plotBarEnabled) {
			d_plotBar->setVisible(true);
		}
	}

	if (xdata.size() == numSamples) {
		xdata[d_plotPosition] = x;
		ydata[d_plotPosition] = y;

		// Advance to next point
		d_plotPosition++;

		// Return to first point when reaching the end
		if (d_plotPosition == numSamples) {
			d_plotPosition = 0;
		}
	} else {
		xdata.push_back(x);
		ydata.push_back(y);
	}

	d_plotBar->setPlotCoord(QPointF(x, d_plotBar->plotCoord().y()));

	curve.setRawSamples(xdata.data(), ydata.data(), xdata.size());

	if (d_cursorsEnabled) {
		onCursor1Moved(d_vBar1->transform(d_vBar1->plotCoord()).x());
		onCursor2Moved(d_vBar2->transform(d_vBar2->plotCoord()).x());
	}

	replot();
}

int dBgraph::getNumSamples() const
{
	return numSamples;
}

QString dBgraph::getScaleValueFormat(double value, QwtAxisId scale) const
{
	auto *scaleDraw = static_cast<const OscScaleDraw *>(
				  axisScaleDraw(scale));

	return formatter->format(value,
				 scaleDraw->getUnitType(),
				 scaleDraw->getFloatPrecison());
}

QString dBgraph::getScaleValueFormat(double value, QwtAxisId scale,
				     int precision) const
{
	auto *scaleDraw = static_cast<const OscScaleDraw *>(
				  axisScaleDraw(scale));

	return formatter->format(value,
				 scaleDraw->getUnitType(),
				 precision);
}

void dBgraph::setShowZero(bool en)
{
	OscScaleEngine *scaleLeft = new OscScaleEngine();
	scaleLeft->setMajorTicksCount(7);
	scaleLeft->showZero(en);

	this->setAxisScaleEngine(QwtPlot::yLeft,
				 static_cast<QwtScaleEngine *>(scaleLeft));

	replot();
}

const QwtScaleWidget *dBgraph::getAxisWidget(QwtAxisId id)
{
	return axisWidget(id);
}

void dBgraph::setNumSamples(int num)
{
	if (numSamples == num) {
		return;
	}

	numSamples = (unsigned int) num;

	reset();

	replot();
}

void dBgraph::reset()
{
	xdata.clear();
	ydata.clear();
	markerIntersection1->detach();
	markerIntersection2->detach();
	d_plotPosition = 0;
}

void dBgraph::setColor(const QColor& color)
{
	this->color = color;
	curve.setPen(QPen(color,thickness));
}

const QColor& dBgraph::getColor() const
{
	return this->color;
}

void dBgraph::setThickness(int index)
{
	double thickness = 0.5 * (index + 1);
	this->thickness = thickness;
	curve.setPen(QPen(color,thickness));
	replot();
}

double dBgraph::getThickness()
{
	return this->thickness;
}

QString dBgraph::xTitle() const
{
	return axisTitle(QwtPlot::xTop).text();
}

QString dBgraph::yTitle() const
{
	return axisTitle(QwtPlot::yLeft).text();
}

void dBgraph::setXTitle(const QString& title)
{
	QwtText xTitle(title);
	QFont font = axisTitle(QwtPlot::xTop).font();
	font.setWeight(QFont::Normal);
	xTitle.setFont(font);

	setAxisTitle(QwtPlot::xTop, xTitle);
}

void dBgraph::setYTitle(const QString& title)
{
	QwtText yTitle(title);
	QFont font = axisTitle(QwtPlot::xTop).font();
	font.setWeight(QFont::Normal);
	yTitle.setFont(font);

	setAxisTitle(QwtPlot::yLeft, yTitle);
	d_cursorReadouts->setVoltageCursor1LabelText(title.mid(0,3)+"1= ");
	d_cursorReadouts->setVoltageCursor2LabelText(title.mid(0,3)+"2= ");
	d_cursorReadouts->setDeltaVoltageLabelText("Δ"+title.mid(0,3)+"= ");
}

void dBgraph::setXMin(double val)
{
	zoomer->resetZoom();
	setAxisScale(QwtPlot::xTop, val, xmax);
	xmin = val;
	draw_x->invalidateCache();

	zoomer->setZoomBase();
	replot();
}

void dBgraph::setXMax(double val)
{
	zoomer->resetZoom();
	setAxisScale(QwtPlot::xTop, xmin, val);
	xmax = val;
	draw_x->invalidateCache();

	zoomer->setZoomBase();
	replot();
}

void dBgraph::setYMin(double val)
{
	setAxisScale(QwtPlot::yLeft, val, ymax);
	ymin = val;
	replot();

	double width = xmax - xmin;
	double height = ymax - ymin;
	zoomer->setZoomBase(QRectF(xmin, ymin, width, height));
}

void dBgraph::setYMax(double val)
{
	setAxisScale(QwtPlot::yLeft, ymin, val);
	ymax = val;
	replot();

	double width = xmax - xmin;
	double height = ymax - ymin;
	zoomer->setZoomBase(QRectF(xmin, ymin, width, height));
}

QString dBgraph::xUnit() const
{
	return draw_x->getUnitType();
}

QString dBgraph::yUnit() const
{
	return draw_y->getUnitType();
}

void dBgraph::setXUnit(const QString& unit)
{
	draw_x->setUnitType(unit);
}

void dBgraph::setYUnit(const QString& unit)
{
	draw_y->setUnitType(unit);
}

void dBgraph::useLogFreq(bool use_log_freq)
{
	if (use_log_freq) {
		this->setAxisScaleEngine(QwtPlot::xTop, new QwtLogScaleEngine);
	} else {
		auto scaleTop = new OscScaleEngine;
		scaleTop->setMajorTicksCount(9);
		this->setAxisScaleEngine(QwtPlot::xTop, scaleTop);
	}

	this->log_freq = use_log_freq;

	if (d_cursorsEnabled && isVisible()) {
		onCursor1Moved(d_vBar1->transform(d_vBar1->plotCoord()).x());
		onCursor2Moved(d_vBar2->transform(d_vBar2->plotCoord()).x());
	}

	// Use delta only when log scale is disabled and delta
	// label mode is enabled
	auto sw = axisWidget(QwtPlot::xTop);
	auto *sd = dynamic_cast<OscScaleDraw *>(sw->scaleDraw());
	sd->enableDeltaLabel(delta_label & (!use_log_freq));
	sw->repaint();

	replot();
}

void dBgraph::useDeltaLabel(bool use_delta)
{
	if (delta_label != use_delta) {
		delta_label = use_delta;

		if (!log_freq) {
			auto sw = axisWidget(QwtPlot::xTop);
			auto *sd = dynamic_cast<OscScaleDraw *>(sw->scaleDraw());
			sd->enableDeltaLabel(use_delta);
			sw->repaint();
		}
	}
}

void dBgraph::sweepDone()
{
	if (d_plotBarEnabled) {
		d_plotBar->setVisible(false);
	}
}

void dBgraph::onFrequencyCursorPositionChanged(int pos)
{
	d_frequencyBar->setPixelPosition(pos);

	Q_EMIT frequencySelected(d_frequencyBar->plotCoord().x());
}

void dBgraph::onFrequencyBarMoved(double frequency)
{
	auto oldY = d_frequencyBar->plotCoord().y();
	d_frequencyBar->setPlotCoord(QPointF(frequency, oldY));
}

void dBgraph::addReferenceWaveform(QVector<double> xData, QVector<double> yData)
{
	reference.setSamples(xData, yData);
	reference.attach(this);
	replot();
}

void dBgraph::removeReferenceWaveform()
{
	reference.detach();
	replot();
}

bool dBgraph::addReferenceWaveformFromPlot()
{
	if (xdata.size() == 0 || ydata.size() == 0) {
		return false;
	}

	addReferenceWaveform(xdata, ydata);

	return true;
}

void dBgraph::onCursor1PositionChanged(int pos)
{
	pos = std::min(pos,QwtPlot::canvas()->width()-1);
	d_vBar1->setPixelPosition(pos);
	onCursor1Moved(pos);
}

void dBgraph::onCursor2PositionChanged(int pos)
{
	pos = std::min(pos,QwtPlot::canvas()->width()-1);
	d_vBar2->setPixelPosition(pos);
	onCursor2Moved(pos);
}

void dBgraph::onVbar1PixelPosChanged(int pos)
{
	Q_EMIT VBar1PixelPosChanged(pos);
}

void dBgraph::onVbar2PixelPosChanged(int pos)
{
	Q_EMIT VBar2PixelPosChanged(pos);
}

void dBgraph::toggleCursors(bool en)
{
	d_frequencyBar->setPixelPosition(0);
	if (d_cursorsEnabled != en) {
		if (!d_cursorsCentered) {
			d_cursorsCentered=true;
			d_vBar1->setPixelPosition(canvas()->width()/2-30);
			d_vBar2->setPixelPosition(canvas()->width()/2+30);
		}

		d_cursorsEnabled = en;
		d_vBar1->setVisible(en);
		d_vBar2->setVisible(en);

		d_cursorReadouts->setTimeReadoutVisible(en);
		d_cursorReadouts->setVoltageReadoutVisible(en);

		if (en) {
			onCursor1Moved(d_vBar1->transform(d_vBar1->plotCoord()).x());
			onCursor2Moved(d_vBar2->transform(d_vBar2->plotCoord()).x());
		} else {
			markerIntersection1->detach();
			markerIntersection2->detach();
			replot();
		}
	}
}

CustomPlotPositionButton::ReadoutsPosition
dBgraph::getCursorReadoutCurrentPosition()
{
	return d_cursorReadouts->getCurrentPosition();
}

void dBgraph::onCursor1Moved(int value)
{
	QString text;

	auto point = picker->pointCoordinates(QPoint(value,0));
	text = formatter->format(point.x(),"Hz",2);

	d_cursorReadouts->setTimeCursor1Text(text);
	text = cursorIntersection(point.x());
	d_cursorReadouts->setVoltageCursor1Text(text);

	double d1 = d_cursorReadouts->voltageCursor1Text().split(" ")[0].toDouble();
	double d2 = d_cursorReadouts->voltageCursor2Text().split(" ")[0].toDouble();

	if (text == "-") {
		markerIntersection1->detach();
	} else {
		if (d_cursorsEnabled) {
			markerIntersection1->attach(this);
			markerIntersection1->setValue(point.x(), d1);
		}
	}

	replot();

	d_cursorReadouts->setVoltageDeltaText(QString::number(d2-d1)+" "+
					      draw_y->getUnitType());
}

void dBgraph::onCursor2Moved(int value)
{
	QString text;

	auto point = picker->pointCoordinates(QPoint(value,0));
	text = formatter->format(point.x(),"Hz",2);

	d_cursorReadouts->setTimeCursor2Text(text);
	text = cursorIntersection(point.x());
	d_cursorReadouts->setVoltageCursor2Text(text);

	double d1 = d_cursorReadouts->voltageCursor1Text().split(" ")[0].toDouble();
	double d2 = d_cursorReadouts->voltageCursor2Text().split(" ")[0].toDouble();

	if (text == "-") {
		markerIntersection2->detach();
	} else {
		if (d_cursorsEnabled) {
			markerIntersection2->attach(this);
			markerIntersection2->setValue(point.x(), d2);
		}
	}

	replot();

	d_cursorReadouts->setVoltageDeltaText(QString::number(d2-d1)+" "+
					      draw_y->getUnitType());
}

QString dBgraph::cursorIntersection(qreal freq)
{
	if (xdata.size() == 0 || xdata.data()[xdata.size()-1] < freq) {
		return QString("-");//for the case when there is no plot
	} else {
		double leftFreq,rightFreq,leftCustom,rightCustom;
		int rightIndex = -1;
		int leftIndex = -1;

		for (int i=1; i<xdata.size(); i++) {
			if (xdata.data()[i-1] <= freq && freq <= xdata.data()[i]) {
				leftIndex=i-1;
				rightIndex=i;
			}
		}

		if (leftIndex == -1 || rightIndex == -1) {
			return QString("-");
		}

		if (!log_freq) {
			leftFreq = xdata.data()[leftIndex];
			rightFreq = xdata.data()[rightIndex];
		} else {
			freq = log10(freq);
			leftFreq = log10(xdata.data()[leftIndex]);
			rightFreq = log10(xdata.data()[rightIndex]);
		}

		leftCustom = ydata.data()[leftIndex];
		rightCustom = ydata.data()[rightIndex];

		double val = (rightCustom - leftCustom)/(rightFreq - leftFreq)*
			     (freq-leftFreq)+leftCustom;

		return QString::number(val,'f',2) +" "+ draw_y->getUnitType();
	}
}

void dBgraph::setCursorReadoutsTransparency(int value)
{
	d_cursorReadouts->setTransparency(value);
}

void dBgraph::moveCursorReadouts(CustomPlotPositionButton::ReadoutsPosition
				 position)
{
	d_cursorReadouts->moveToPosition(position);
}

QVector<double> dBgraph::getXAxisData()
{
	QVector<double> data;

	for (int i = 0; i < curve.data()->size(); ++i) {
		data.push_back(curve.data()->sample(i).x());
	}

	return data;
}

QVector<double> dBgraph::getYAxisData()
{
	QVector<double> data;

	for (int i = 0; i < curve.data()->size(); ++i) {
		data.push_back(curve.data()->sample(i).y());
	}

	return data;
}

void dBgraph::enableFrequencyBar(bool enable)
{
	d_frequencyBar->setVisible(enable);
	d_frequencyBar->setPixelPosition(0);
}

void dBgraph::setYAxisInterval(double min, double max, double correction)
{
	for (size_t i = 0; i < ydata.size(); ++i) {
		double value = ydata[i];
		bool modified = false;
		if (value > max) {
			value = value - correction;
			modified = true;
		} else if (value < min) {
			value = value + correction;
			modified = true;
		}

		if (modified) {
			ydata[i] = value;
			curve.setRawSamples(xdata.data(), ydata.data(), xdata.size());
			replot();
		}
	}
}

void dBgraph::scaleDivChanged()
{
	QwtPlot *plt = static_cast<QwtPlot *>((sender())->parent());
	QwtInterval intv = plt->axisInterval(QwtPlot::xTop);
	this->setAxisScale(QwtPlot::xTop, intv.minValue(), intv.maxValue());
	this->replot();

	onCursor1Moved(d_vBar1->transform(d_vBar1->plotCoord()).x());
	onCursor2Moved(d_vBar2->transform(d_vBar2->plotCoord()).x());
}

void dBgraph::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		Q_EMIT resetZoom();
	}
}

void dBgraph::onResetZoom()
{
	zoomer->resetZoom();
}

void dBgraph::showEvent(QShowEvent *event)
{
	auto sw = axisWidget(QwtPlot::xTop);
	sw->scaleDraw()->invalidateCache();
	sw->repaint();
}

void dBgraph::setPlotBarEnabled(bool enabled)
{
	d_plotBarEnabled = enabled;
}
