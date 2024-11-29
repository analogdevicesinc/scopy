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
#include "plotpickerwrapper.h"
#include "x_axis_scale_zoomer.h"

#include <QIcon>
#include <qwt_plot_layout.h>

using namespace scopy;

void dBgraph::setupVerticalBars()
{
	d_plotBar = new VertBar(this, true);
	d_frequencyBar = new VertBar(this, true);
	d_symbolCtrl->attachSymbol(d_plotBar);
	d_symbolCtrl->attachSymbol(d_frequencyBar);

	QPen plotLinePen = QPen(QColor(211, 211, 211, 50), 5, Qt::SolidLine);
	QPen frequencyLinePen = QPen(QColor(74, 100, 255, 150), 2, Qt::DashLine);

	d_plotBar->setVisible(false);

	d_frequencyBar->setPen(frequencyLinePen);
	d_frequencyBar->setVisible(true);
	d_frequencyBar->setMobileAxis(QwtAxis::XTop);
	d_frequencyBar->setPixelPosition(0);

	d_plotBar->setPen(plotLinePen);
	d_plotBar->setMobileAxis(QwtAxis::XTop);

	connect(d_frequencyBar, &VertBar::pixelPositionChanged, this, &dBgraph::frequencyBarPositionChanged);

	d_vBar1->setMobileAxis(QwtAxis::XTop);
	d_vBar2->setMobileAxis(QwtAxis::XTop);
}

void dBgraph::setupReadouts()
{
	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	d_cursorReadouts->setTimeCursor1LabelText("F1 = ");
	d_cursorReadouts->setTimeCursor2LabelText("F2 = ");
	d_cursorReadouts->setVoltageCursor1LabelText("Mag1 = ");
	d_cursorReadouts->setVoltageCursor2LabelText("Mag2 = ");
	d_cursorReadouts->setDeltaVoltageLabelText("ΔMag = ");

	d_cursorReadouts->setFrequencyDeltaVisible(false);
	d_cursorReadouts->setTimeDeltaVisible(false);
	d_cursorReadouts->setTransparency(0);
}

dBgraph::dBgraph(QWidget *parent, bool isdBgraph)
	: DisplayPlot(0, parent, isdBgraph)
	, curve("data")
	, reference("reference")
	, xmin(10)
	, xmax(10)
	, ymin(10)
	, ymax(10)
	, d_plotPosition(0)
	, numSamples(0)
	, delta_label(false)
	, d_plotBarEnabled(true)
{
	setAxisVisible(QwtAxis::XBottom, false);
	setAxisVisible(QwtAxis::XTop, true);

	setAxisAutoScale(QwtAxis::YLeft, false);
	setAxisAutoScale(QwtAxis::XTop, false);

	QColor plotColor;
	if(QIcon::themeName() == "scopy-default") {
		plotColor = QColor("#6E6E6F");
	} else {
		plotColor = QColor("#D3D3D3");
	}

	EdgelessPlotGrid *grid = new EdgelessPlotGrid;
	grid->setMajorPen(plotColor, 1.0, Qt::DashLine);

	grid->setXAxis(QwtAxis::XTop);
	grid->attach(this);

	plotLayout()->setAlignCanvasToScales(true);

	curve.attach(this);
	curve.setRenderHint(QwtPlotItem::RenderAntialiased);
	curve.setXAxis(QwtAxis::XTop);
	curve.setYAxis(QwtAxis::YLeft);

	reference.setRenderHint(QwtPlotItem::RenderAntialiased);
	reference.setXAxis(QwtAxis::XTop);
	reference.setYAxis(QwtAxis::YLeft);
	reference.setPen(Qt::red, 1.5);

	thickness = 1;

	d_formatter = static_cast<PrefixFormatter *>(new MetricPrefixFormatter);

	OscScaleEngine *scaleLeft = new OscScaleEngine;
	setYaxisNumDiv(7);
	scaleLeft->setMajorTicksCount(7);
	this->setAxisScaleEngine(QwtAxis::YLeft, static_cast<QwtScaleEngine *>(scaleLeft));
	/* draw_x / draw_y: Outmost X / Y scales. Only draw the labels */

	draw_x = new OscScaleDraw(d_formatter, "Hz");
	draw_x->setFloatPrecision(2);
	draw_x->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	draw_x->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	setAxisScaleDraw(QwtAxis::XTop, draw_x);

	draw_y = new OscScaleDraw("dB");
	draw_y->setFloatPrecision(2);
	draw_y->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	draw_y->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	draw_y->setMinimumExtent(50);
	setAxisScaleDraw(QwtAxis::YLeft, draw_y);

	d_leftHandlesArea->setMinimumWidth(60);
	d_leftHandlesArea->setTopPadding(10);
	d_leftHandlesArea->setBottomPadding(0);
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());

	d_rightHandlesArea->setMinimumWidth(40);
	d_rightHandlesArea->setMinimumHeight(this->minimumHeight());

	d_topHandlesArea->setMinimumHeight(20);
	d_topHandlesArea->setLargestChildWidth(60);

	useLogFreq(false);

	/* Create 4 scales within the plot itself. Only draw the ticks */
	for(unsigned int i = 0; i < 4; i++) {
		EdgelessPlotScaleItem *scaleItem = new EdgelessPlotScaleItem(static_cast<QwtScaleDraw::Alignment>(i));

		/* Top/bottom scales must be sync'd to xTop; left/right scales
		 * must be sync'd to yLeft */
		if(i < 2) {
			scaleItem->setXAxis(QwtAxis::XTop);
		} else {
			scaleItem->setYAxis(QwtAxis::YLeft);
		}

		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);

		QPalette palette = scaleItem->palette();
		palette.setBrush(QPalette::WindowText, QColor(plotColor));
		palette.setBrush(QPalette::Text, QColor(plotColor));
		scaleItem->setPalette(palette);
		scaleItem->setBorderDistance(0);
		scaleItem->attach(this);
	}

	zoomer = new XAxisScaleZoomer(canvas());
	zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
	zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);

	d_magnifier.push_back(new scopy::MousePlotMagnifier(canvas()));
	d_magnifier[0]->setXAxis(QwtAxis::XTop);
	d_magnifier[0]->setYAxisEnabled(false);
	d_magnifier[0]->setEnabled(true);

	installEventFilter(this);

	static_cast<QFrame *>(canvas())->setLineWidth(0);
	setContentsMargins(10, 10, 24, 20);
	QMargins margins = contentsMargins();
	margins.setBottom(0);
	setContentsMargins(margins);

	setAxisVisible(QwtAxis::YLeft, false);
	setAxisVisible(QwtAxis::XTop, false);

	QwtScaleWidget *scaleWidget = axisWidget(QwtAxis::XTop);
	const int fmw = QFontMetrics(scaleWidget->font()).horizontalAdvance("-XXXX.XX XX");
	scaleWidget->setMinBorderDist(fmw / 2, fmw / 2);

	setupVerticalBars();
	setupReadouts();

	markerIntersection1->setAxes(QwtAxis::XTop, QwtAxis::YLeft);
	markerIntersection2->setAxes(QwtAxis::XTop, QwtAxis::YLeft);
}

dBgraph::~dBgraph()
{
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
	delete d_formatter;
}

void dBgraph::replot()
{
	if(!d_leftHandlesArea || !d_topHandlesArea) {
		return;
	}

	d_leftHandlesArea->repaint();
	d_topHandlesArea->repaint();

	BasicPlot::replot();
}

void dBgraph::enableXaxisLabels()
{
	d_topHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new XTopRuller(this)));
}

void dBgraph::enableYaxisLabels()
{
	d_leftHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new YLeftRuller(this)));
}

void dBgraph::parametersOverrange(bool enable)
{
	if(enable) {
		d_plotBar->setPen(QPen(QColor(250, 0, 0, 50), 5, Qt::SolidLine));
	} else {
		d_plotBar->setPen(QPen(QColor(211, 211, 211, 50), 5, Qt::SolidLine));
	}
}

void dBgraph::setAxesScales(double xmin, double xmax, double ymin, double ymax)
{
	setAxisScale(QwtAxis::XTop, xmin, xmax);
	setAxisScale(QwtAxis::YLeft, ymin, ymax);
}

void dBgraph::setAxesTitles(const QString &x, const QString &y)
{
	QwtText xTitle(x);
	QwtText yTitle(y);

	QFont font = axisTitle(QwtAxis::XTop).font();
	font.setWeight(QFont::Normal);
	xTitle.setFont(font);
	yTitle.setFont(font);

	setAxisTitle(QwtAxis::XTop, xTitle);
	setAxisTitle(QwtAxis::YLeft, yTitle);
}

void dBgraph::plot(double x, double y)
{
	if(!d_plotBar->isVisible() && !xdata.size()) {
		if(d_plotBarEnabled) {
			d_plotBar->setVisible(true);
		}
	}

	if(xdata.size() == numSamples) {
		xdata[d_plotPosition] = x;
		ydata[d_plotPosition] = y;

		// Advance to next point
		d_plotPosition++;

		// Return to first point when reaching the end
		if(d_plotPosition == numSamples) {
			d_plotPosition = 0;
		}
	} else {
		xdata.push_back(x);
		ydata.push_back(y);
	}

	d_plotBar->setPlotCoord(QPointF(x, d_plotBar->plotCoord().y()));

	curve.setRawSamples(xdata.data(), ydata.data(), xdata.size());

	if(d_cursorsEnabled) {
		onVCursor1Moved(d_vBar1->plotCoord().x());
		onVCursor2Moved(d_vBar2->plotCoord().x());
	}
	replot();
}

bool dBgraph::eventFilter(QObject *object, QEvent *event)
{
	if(object == canvas() && event->type() == QEvent::Resize) {

		d_leftHandlesArea->repaint();
		d_bottomHandlesArea->setLeftPadding(d_leftHandlesArea->width() + 10);
		d_bottomHandlesArea->setRightPadding(d_rightHandlesArea->width() + 24);
	}
	return QObject::eventFilter(object, event);
}

int dBgraph::getNumSamples() const { return numSamples; }

QString dBgraph::getScaleValueFormat(double value, QwtAxisId scale) const
{
	auto *scaleDraw = static_cast<const OscScaleDraw *>(axisScaleDraw(scale));

	return d_formatter->format(value, scaleDraw->getUnitType(), scaleDraw->getFloatPrecison());
}

QString dBgraph::getScaleValueFormat(double value, QwtAxisId scale, int precision) const
{
	auto *scaleDraw = static_cast<const OscScaleDraw *>(axisScaleDraw(scale));

	return d_formatter->format(value, scaleDraw->getUnitType(), precision);
}

void dBgraph::setShowZero(bool en)
{
	OscScaleEngine *scaleLeft = new OscScaleEngine();
	scaleLeft->setMajorTicksCount(7);
	scaleLeft->showZero(en);

	this->setAxisScaleEngine(QwtAxis::YLeft, static_cast<QwtScaleEngine *>(scaleLeft));

	replot();
}

const QwtScaleWidget *dBgraph::getAxisWidget(QwtAxisId id) { return axisWidget(id); }

void dBgraph::setNumSamples(int num)
{
	if(numSamples == num) {
		return;
	}

	numSamples = (unsigned int)num;

	reset();

	replot();
}

void dBgraph::reset()
{
	xdata.clear();
	ydata.clear();
	d_plotPosition = 0;
}

void dBgraph::setColor(const QColor &color)
{
	this->color = color;
	curve.setPen(QPen(color, thickness));
}

const QColor &dBgraph::getColor() const { return this->color; }

void dBgraph::setThickness(int index)
{
	double thickness = 0.5 * (index + 1);
	this->thickness = thickness;
	curve.setPen(QPen(color, thickness));
	replot();
}

double dBgraph::getThickness() { return this->thickness; }

QString dBgraph::xTitle() const { return axisTitle(QwtAxis::XTop).text(); }

QString dBgraph::yTitle() const { return axisTitle(QwtAxis::YLeft).text(); }

void dBgraph::setXTitle(const QString &title)
{
	QwtText xTitle(title);
	QFont font = axisTitle(QwtAxis::XTop).font();
	font.setWeight(QFont::Normal);
	xTitle.setFont(font);

	setAxisTitle(QwtAxis::XTop, xTitle);
}

void dBgraph::setYTitle(const QString &title)
{
	QwtText yTitle(title);
	QFont font = axisTitle(QwtAxis::XTop).font();
	font.setWeight(QFont::Normal);
	yTitle.setFont(font);

	setAxisTitle(QwtAxis::YLeft, yTitle);
	d_cursorReadouts->setVoltageCursor1LabelText(title.mid(0, 3) + "1 = ");
	d_cursorReadouts->setVoltageCursor2LabelText(title.mid(0, 3) + "2 = ");
	d_cursorReadouts->setDeltaVoltageLabelText("Δ" + title.mid(0, 3) + " = ");
}

void dBgraph::setXMin(double val)
{
	zoomer->resetZoom();
	Q_EMIT d_magnifier[0]->reset();
	setAxisScale(QwtAxis::XTop, val, xmax);
	xmin = val;
	draw_x->invalidateCache();

	zoomer->setZoomBase();
	d_magnifier[0]->setBaseRect(zoomer->zoomBase());
	replot();
	auto div = axisScaleDiv(QwtAxis::XTop);
	setXaxisNumDiv((div.ticks(2)).size() - 1);
	setXaxisMajorTicksPos(div.ticks(2));
}

void dBgraph::setXMax(double val)
{
	zoomer->resetZoom();
	Q_EMIT d_magnifier[0]->reset();
	setAxisScale(QwtAxis::XTop, xmin, val);
	xmax = val;
	draw_x->invalidateCache();

	zoomer->setZoomBase();
	d_magnifier[0]->setBaseRect(zoomer->zoomBase());
	replot();
	auto div = axisScaleDiv(QwtAxis::XTop);
	setXaxisNumDiv((div.ticks(2)).size() - 1);
	setXaxisMajorTicksPos(div.ticks(2));
}

void dBgraph::setYMin(double val)
{
	setAxisScale(QwtAxis::YLeft, val, ymax);
	ymin = val;
	replot();
	d_leftHandlesArea->repaint();
	d_topHandlesArea->repaint();

	double width = xmax - xmin;
	double height = ymax - ymin;
	zoomer->setZoomBase(QRectF(xmin, ymin, width, height));
	d_magnifier[0]->setBaseRect(zoomer->zoomBase());
}

void dBgraph::setYMax(double val)
{
	setAxisScale(QwtAxis::YLeft, ymin, val);
	ymax = val;
	replot();

	double width = xmax - xmin;
	double height = ymax - ymin;
	zoomer->setZoomBase(QRectF(xmin, ymin, width, height));
	d_magnifier[0]->setBaseRect(zoomer->zoomBase());
}

QString dBgraph::xUnit() const
{
	return draw_x->getUnitType();
	;
}

QString dBgraph::yUnit() const
{
	return draw_y->getUnitType();
	;
}

void dBgraph::setXUnit(const QString &unit) { draw_x->setUnitType(unit); }

void dBgraph::setYUnit(const QString &unit) { draw_y->setUnitType(unit); }

void dBgraph::useLogFreq(bool use_log_freq)
{
	if(use_log_freq) {
		setPlotLogaritmic(true);
		this->setAxisScaleEngine(QwtAxis::XTop, new QwtLogScaleEngine);
		replot();
		auto div = axisScaleDiv(QwtAxis::XTop);
		setXaxisNumDiv((div.ticks(2)).size() - 1);
		setXaxisMajorTicksPos(div.ticks(2));
	} else {
		setPlotLogaritmic(false);
		auto scaleTop = new OscScaleEngine;
		scaleTop->setMajorTicksCount(9);
		setXaxisNumDiv(8);
		this->setAxisScaleEngine(QwtAxis::XTop, scaleTop);
	}

	this->log_freq = use_log_freq;

	if(d_cursorsEnabled && isVisible()) {
		onVCursor1Moved(d_vBar1->plotCoord().x());
		onVCursor2Moved(d_vBar2->plotCoord().x());
	}

	// Use delta only when log scale is disabled and delta
	// label mode is enabled
	auto sw = axisWidget(QwtAxis::XTop);
	auto *sd = dynamic_cast<OscScaleDraw *>(sw->scaleDraw());
	sd->enableDeltaLabel(delta_label & (!use_log_freq));
	sw->repaint();

	replot();
}

void dBgraph::useDeltaLabel(bool use_delta)
{
	if(delta_label != use_delta) {
		delta_label = use_delta;

		if(!log_freq) {
			auto sw = axisWidget(QwtAxis::XTop);
			auto *sd = dynamic_cast<OscScaleDraw *>(sw->scaleDraw());
			sd->enableDeltaLabel(use_delta);
			sw->repaint();
		}
	}
}

void dBgraph::sweepDone()
{
	if(d_plotBarEnabled) {
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
	if(xdata.size() == 0 || ydata.size() == 0) {
		return false;
	}

	addReferenceWaveform(xdata, ydata);

	return true;
}

QString dBgraph::formatXValue(double value, int precision) const { return d_formatter->format(value, "Hz", precision); }

QString dBgraph::formatYValue(double value, int precision) const
{
	return d_formatter->format(value, draw_y->getUnitType(), precision);
}

void dBgraph::onVCursor1Moved(double value)
{
	QString text;
	text = d_formatter->format(value, "Hz", 2);

	d_cursorReadouts->setTimeCursor1Text(text);
	text = cursorIntersection(value);
	d_cursorReadouts->setVoltageCursor1Text(text);

	double d1 = d_cursorReadouts->voltageCursor1Text().split(" ")[0].toDouble();
	double d2 = d_cursorReadouts->voltageCursor2Text().split(" ")[0].toDouble();

	if(text == "-") {
		markerIntersection1->detach();
	} else {
		if(d_cursorsEnabled) {
			markerIntersection1->attach(this);
			markerIntersection1->setValue(value, d1);
		}
	}
	replot();

	d_cursorReadouts->setVoltageDeltaText(QString::number(d2 - d1) + " " + draw_y->getUnitType());
}

void dBgraph::onVCursor2Moved(double value)
{
	QString text;
	text = d_formatter->format(value, "Hz", 2);

	d_cursorReadouts->setTimeCursor2Text(text);
	text = cursorIntersection(value);
	d_cursorReadouts->setVoltageCursor2Text(text);

	double d1 = d_cursorReadouts->voltageCursor1Text().split(" ")[0].toDouble();
	double d2 = d_cursorReadouts->voltageCursor2Text().split(" ")[0].toDouble();

	if(text == "-") {
		markerIntersection2->detach();
	} else {
		if(d_cursorsEnabled) {
			markerIntersection2->attach(this);
			markerIntersection2->setValue(value, d2);
		}
	}
	replot();

	d_cursorReadouts->setVoltageDeltaText(QString::number(d2 - d1) + " " + draw_y->getUnitType());
}

QString dBgraph::cursorIntersection(qreal freq)
{
	if(xdata.size() == 0 || xdata.data()[xdata.size() - 1] < freq) {
		return QString("-"); // for the case when there is no plot
	} else {
		double leftFreq, rightFreq, leftCustom, rightCustom;
		int rightIndex = -1;
		int leftIndex = -1;

		for(int i = 1; i < xdata.size(); i++) {
			if(xdata.data()[i - 1] <= freq && freq <= xdata.data()[i]) {
				leftIndex = i - 1;
				rightIndex = i;
			}
		}

		if(leftIndex == -1 || rightIndex == -1) {
			return QString("-");
		}

		if(!log_freq) {
			leftFreq = xdata.data()[leftIndex];
			rightFreq = xdata.data()[rightIndex];
		} else {
			freq = log10(freq);
			leftFreq = log10(xdata.data()[leftIndex]);
			rightFreq = log10(xdata.data()[rightIndex]);
		}

		leftCustom = ydata.data()[leftIndex];
		rightCustom = ydata.data()[rightIndex];

		double val = (rightCustom - leftCustom) / (rightFreq - leftFreq) * (freq - leftFreq) + leftCustom;

		return QString::number(val, 'f', 2) + " " + draw_y->getUnitType();
	}
}

QVector<double> dBgraph::getXAxisData()
{
	QVector<double> data;

	for(size_t i = 0; i < curve.data()->size(); ++i) {
		data.push_back(curve.data()->sample(i).x());
	}

	return data;
}

QVector<double> dBgraph::getYAxisData()
{
	QVector<double> data;

	for(size_t i = 0; i < curve.data()->size(); ++i) {
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
	for(size_t i = 0; i < ydata.size(); ++i) {
		double value = ydata[i];
		bool modified = false;
		if(value > max) {
			value = value - correction;
			modified = true;
		} else if(value < min) {
			value = value + correction;
			modified = true;
		}

		if(modified) {
			ydata[i] = value;
			curve.setRawSamples(xdata.data(), ydata.data(), xdata.size());
			replot();
		}
	}
}

void dBgraph::scaleDivChanged()
{
	QwtPlot *plt = static_cast<QwtPlot *>((sender())->parent());
	QwtInterval intv = plt->axisInterval(QwtAxis::XTop);
	this->setAxisScale(QwtAxis::XTop, intv.minValue(), intv.maxValue());
	this->replot();

	onVCursor1Moved(d_vBar1->plotCoord().x());
	onVCursor2Moved(d_vBar2->plotCoord().x());
}

void dBgraph::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton) {
		Q_EMIT resetZoom();
#ifdef __ANDROID__
	} else if(event->type() == QEvent::MouseButtonDblClick) {
		zoomer->popZoom();
		Q_EMIT zoomOut();
#endif
	}
}

void dBgraph::onResetZoom()
{
	zoomer->resetZoom();
	Q_EMIT d_magnifier[0]->reset();
}

#ifdef __ANDROID__
void dBgraph::onZoomOut() { zoomer->popZoom(); }
#endif

void dBgraph::showEvent(QShowEvent *event)
{
	d_hCursorHandle1->updatePosition();
	d_hCursorHandle2->updatePosition();
	auto sw = axisWidget(QwtAxis::XTop);
	sw->scaleDraw()->invalidateCache();
	sw->repaint();
}

void dBgraph::setPlotBarEnabled(bool enabled) { d_plotBarEnabled = enabled; }

#include "moc_dbgraph.cpp"
