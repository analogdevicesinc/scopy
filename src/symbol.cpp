/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "symbol.h"

#include <qwt_scale_div.h>

/*
 * Abstract Class Symbol
 */
Symbol::Symbol(QObject *parent, const QSize &size, QwtAxisId fixedAxis, QwtAxisId mobileAxis, bool opposedToFixed,
	       bool floats)
	: QObject(parent)
	, d_surface(QRect(QPoint(0, 0), size))
	, d_anchor(0, 0)
	, d_plotCoord(QPointF(0, 0))
	, d_selected(false)
	, d_fixedAxis(fixedAxis)
	, d_mobileAxis(mobileAxis)
	, d_stepSize(0)
	, d_oppToFixed(opposedToFixed)
	, d_floats(floats)
	, d_within_plot(true)
	, d_visible(true)
	, d_pen(Qt::black) {
	const QwtScaleWidget *fAxis = plot()->axisWidget(d_fixedAxis);
	const QwtScaleWidget *mAxis = plot()->axisWidget(d_mobileAxis);

	QObject::connect((const QObject *)fAxis, SIGNAL(scaleDivChanged()), this, SLOT(onFixedScaleChanged()));
	QObject::connect((const QObject *)mAxis, SIGNAL(scaleDivChanged()), this, SLOT(onMobileScaleChanged()));

	onFixedScaleChanged();
	onMobileScaleChanged();
}

Symbol::~Symbol() {}

QwtPlot *Symbol::plot() { return static_cast<QwtPlot *>(parent()); }

const QwtPlot *Symbol::plot() const { return static_cast<const QwtPlot *>(parent()); }

void Symbol::setSurface(const QRect &rect) { d_surface = rect; }

QRect Symbol::surface() { return d_surface; }

void Symbol::setAnchor(const QPoint &anchor) { d_anchor = anchor; }

QPoint Symbol::anchor() { return d_anchor; }

void Symbol::setPlotCoord(const QPointF &pos) {
	if (pos != d_plotCoord) {
		d_plotCoord = pos;
		updateSurfacePos();

		Q_EMIT positionChanged(d_plotCoord.x(), d_plotCoord.y());
	}
}

QPointF Symbol::plotCoord() { return d_plotCoord; }

void Symbol::setSelected(bool sel) {
	d_selected = sel;
	Q_EMIT selected(sel);
}

bool Symbol::isSelected() { return d_selected; }

QwtAxisId Symbol::fixedAxis() const { return d_fixedAxis; }

QwtAxisId Symbol::mobileAxis() const { return d_mobileAxis; }

void Symbol::setMobileAxis(QwtAxisId newAxis) {
	if (d_mobileAxis != newAxis) {
		const QwtScaleWidget *mAxis = plot()->axisWidget(d_mobileAxis);
		disconnect((const QObject *)mAxis, SIGNAL(scaleDivChanged()), this, SLOT(onMobileScaleChanged()));

		d_mobileAxis = newAxis;
		mAxis = plot()->axisWidget(newAxis);
		connect((const QObject *)mAxis, SIGNAL(scaleDivChanged()), this, SLOT(onMobileScaleChanged()));
		onMobileScaleChanged();
	}
}

void Symbol::setStepSize(double step) { d_stepSize = step; }

double Symbol::stepSize() { return d_stepSize; }

bool Symbol::opposedToFixed() { return d_oppToFixed; }

bool Symbol::floats() { return d_floats; }

void Symbol::setCanLeavePlot(bool on) { d_within_plot = !on; }

bool Symbol::canLeavePlot() { return (!d_within_plot); }

void Symbol::setVisible(bool en) {
	if (d_visible != en) {
		d_visible = en;
		Q_EMIT visibilityChanged(en);
	}
}

bool Symbol::isVisible() { return d_visible; }

void Symbol::setPen(const QPen &pen) { d_pen = pen; }

const QPen &Symbol::pen() { return d_pen; }

QPointF Symbol::invTransform(const QPointF &point) const {
	QwtAxisId xAxis = d_fixedAxis.isXAxis() ? d_fixedAxis : d_mobileAxis;
	QwtAxisId yAxis = d_fixedAxis.isYAxis() ? d_fixedAxis : d_mobileAxis;
	const QwtScaleMap xMap = plot()->canvasMap(xAxis);
	const QwtScaleMap yMap = plot()->canvasMap(yAxis);

	return QwtScaleMap::invTransform(xMap, yMap, point);
}

QPointF Symbol::transform(const QPointF &point) const {
	QwtAxisId xAxis = d_fixedAxis.isXAxis() ? d_fixedAxis : d_mobileAxis;
	QwtAxisId yAxis = d_fixedAxis.isYAxis() ? d_fixedAxis : d_mobileAxis;
	const QwtScaleMap xMap = plot()->canvasMap(xAxis);
	const QwtScaleMap yMap = plot()->canvasMap(yAxis);

	return QwtScaleMap::transform(xMap, yMap, point);
}

void Symbol::updateSurfacePos() {
	QPointF plotCoord = d_plotCoord;
	QPoint pixelPos;
	QwtInterval interval = plot()->axisInterval(d_mobileAxis);

	if (d_within_plot) {
		if (d_mobileAxis.pos == QwtPlot::yLeft) {
			if (plotCoord.y() < interval.minValue())
				plotCoord.setY(interval.minValue());
			else if (plotCoord.y() > interval.maxValue())
				plotCoord.setY(interval.maxValue());
		} else {
			if (plotCoord.x() < interval.minValue())
				plotCoord.setX(interval.minValue());
			else if (plotCoord.x() > interval.maxValue())
				plotCoord.setX(interval.maxValue());
		}
	}

	pixelPos = transform(plotCoord).toPoint();
	d_surface.moveTo(pixelPos - d_anchor);
	Q_EMIT pixelPositionChanged(pixelPos.x(), pixelPos.y());
}

void Symbol::updatePlotCoordFromSurfacePos() {
	QPoint pixelPos = surface().topLeft() + anchor();
	QPointF plotCoord = invTransform(pixelPos);

	setPlotCoord(plotCoord);
}

/* emit a signal for the handle to update position */
void Symbol::triggerMove() {
	QPoint pixelPos = transform(d_plotCoord).toPoint();
	Q_EMIT pixelPositionChanged(pixelPos.x(), pixelPos.y());
}

void Symbol::onFixedScaleChanged() {
	QwtInterval interval = plot()->axisInterval(fixedAxis());
	QPointF pos = plotCoord();
	double f;

	if (opposedToFixed())
		f = interval.maxValue();
	else
		f = interval.minValue();

	if (fixedAxis().isXAxis())
		pos.setX(f);
	else
		pos.setY(f);

	setPlotCoord(pos);
}

void Symbol::onMobileScaleChanged() {
	QwtScaleDiv scaleDiv = plot()->axisScaleDiv(mobileAxis());
	QList<double> minorTicks = scaleDiv.ticks(QwtScaleDiv::MinorTick);

	if (minorTicks.size() < 2)
		return;

	setStepSize(qAbs(minorTicks[1] - minorTicks[0]));

	if (this->floats())
		updatePlotCoordFromSurfacePos();
	else
		updateSurfacePos();
}

/*
 * Class VertDebugSymbol
 */

VertDebugSymbol::VertDebugSymbol(QObject *parent, const QSize &size, bool opposedToFixed, bool floats)
	: Symbol(parent, size, QwtPlot::xBottom, QwtPlot::yLeft, opposedToFixed, floats) {
	int x = opposedToFixed ? surface().width() : 0;

	setAnchor(QPoint(x, surface().height() / 2));

	// Pass the base positionChanged(double, double) signal as
	// positionChanged(double)
	connect(this, SIGNAL(positionChanged(double, double)), this, SLOT(onBasePositionChanged(double, double)));
	connect(this, SIGNAL(pixelPositionChanged(int, int)), this, SLOT(onBasePixelPositionChanged(int, int)));
}

void VertDebugSymbol::draw(QPainter *painter) const {
	int w = d_surface.width();
	int h = d_surface.height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();
	int e1 = d_oppToFixed ? -10 : 0;
	int e2 = d_oppToFixed ? 0 : 10;

	painter->setPen(QPen(Qt::gray));
	painter->setBrush(QBrush(Qt::gray));
	QwtPainter::drawRect(painter, x0, y0, w, h);
	painter->setPen(QPen(Qt::red));
	QwtPainter::drawLine(painter, x0 + e1, y0 + h / 2, x0 + w + e2, y0 + h / 2);
}

bool VertDebugSymbol::moveWith(double plotDeltaX, double plotDeltaY) {
	Q_UNUSED(plotDeltaX);

	QPointF deltaPoint;
	bool canMove = false;

	if (qAbs(d_stepSize) > 0) {
		int stepsInDelta = (int)(plotDeltaY / d_stepSize);

		if (qAbs(stepsInDelta) > 0) {
			deltaPoint = QPointF(0, stepsInDelta * d_stepSize);
			canMove = true;
		}
	} else {
		deltaPoint = QPointF(0, plotDeltaY);
		canMove = true;
	}

	if (canMove)
		setPlotCoord(plotCoord() + deltaPoint);

	return canMove;
}

void VertDebugSymbol::setPosition(double vertPos) {
	QPointF p = plotCoord();
	if (p.y() != vertPos) {
		p.setY(vertPos);
		setPlotCoord(p);
	}
}

void VertDebugSymbol::setPixelPosition(int vertPos) {
	QPointF pixelP(0, vertPos);
	double y = invTransform(pixelP).y();

	setPosition(y);
}

void VertDebugSymbol::onBasePositionChanged(double x, double y) {
	Q_UNUSED(x);

	Q_EMIT positionChanged(y);
}

void VertDebugSymbol::onBasePixelPositionChanged(int x, int y) {
	Q_UNUSED(x);

	Q_EMIT pixelPositionChanged(y);
}

/*
 * Class HorizDebugSymbol
 */

HorizDebugSymbol::HorizDebugSymbol(QObject *parent, const QSize &size, bool opposedToFixed, bool floats)
	: Symbol(parent, size, QwtPlot::yLeft, QwtPlot::xBottom, opposedToFixed, floats) {
	int y = opposedToFixed ? 0 : surface().height();
	setAnchor(QPoint(surface().width() / 2, y));

	// Pass the base positionChanged(double, double) signal as
	// positionChanged(double)
	connect(this, SIGNAL(positionChanged(double, double)), this, SLOT(onBasePositionChanged(double, double)));
	connect(this, SIGNAL(pixelPositionChanged(int, int)), this, SLOT(onBasePixelPositionChanged(int, int)));
}

void HorizDebugSymbol::draw(QPainter *painter) const {
	int w = d_surface.width();
	int h = d_surface.height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();
	int e1 = d_oppToFixed ? 0 : -10;
	int e2 = d_oppToFixed ? 10 : 0;

	painter->setPen(QPen(Qt::gray));
	painter->setBrush(QBrush(Qt::gray));
	QwtPainter::drawRect(painter, x0, y0, w, h);
	painter->setPen(QPen(Qt::red));
	QwtPainter::drawLine(painter, x0 + w / 2, y0 + e1, x0 + w / 2, y0 + h + e2);
}

bool HorizDebugSymbol::moveWith(double plotDeltaX, double plotDeltaY) {
	Q_UNUSED(plotDeltaY);

	QPointF deltaPoint;
	bool canMove = false;

	if (qAbs(d_stepSize) > 0) {
		int stepsInDelta = (int)(plotDeltaX / d_stepSize);

		if (qAbs(stepsInDelta) > 0) {
			deltaPoint = QPointF(stepsInDelta * d_stepSize, 0);
			canMove = true;
		}
	} else {
		deltaPoint = QPointF(plotDeltaX, 0);
		canMove = true;
	}

	if (canMove)
		Q_EMIT positionChanged(deltaPoint.x());

	return canMove;
}

void HorizDebugSymbol::setPosition(double horizPos) {
	QPointF p = plotCoord();
	if (p.x() != horizPos) {
		p.setX(horizPos);
		setPlotCoord(p);
	}
}

void HorizDebugSymbol::setPixelPosition(int horizPos) {
	QPointF pixelP(horizPos, 0);
	double x = invTransform(pixelP).x();

	setPosition(x);
}

void HorizDebugSymbol::onBasePositionChanged(double x, double y) {
	Q_UNUSED(y);

	Q_EMIT positionChanged(x);
}

void HorizDebugSymbol::onBasePixelPositionChanged(int x, int y) {
	Q_UNUSED(y);

	Q_EMIT pixelPositionChanged(x);
}

/*
 * Class TriggerLevelCursor
 */

TriggerLevelCursor::TriggerLevelCursor(QObject *parent, const QPixmap &pixmap)
	: VertDebugSymbol(parent, QSize(pixmap.size()), true), d_cursorPixmap(pixmap) {
	setAnchor(QPoint(surface().width() - 7, surface().height() / 2));
}

void TriggerLevelCursor::draw(QPainter *painter) const {
	int w = d_surface.width();
	int h = d_surface.height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();

	QwtInterval interval = plot()->axisInterval(mobileAxis());
	if (d_plotCoord.y() < interval.minValue()) {
		painter->setPen(d_pen);
		QRect r(x0 + 13, y0, w - 13, h);
		QPainterPath path;
		path.moveTo(r.topLeft());
		path.lineTo(r.topRight());
		path.lineTo(r.left() + r.width() / 2, r.bottom());
		path.lineTo(r.topLeft());
		painter->fillPath(path, QBrush(d_pen.color()));
	} else if (d_plotCoord.y() > interval.maxValue()) {
		painter->setPen(d_pen);
		QRect r(x0 + 13, y0, w - 13, h);
		QPainterPath path;
		path.moveTo(r.bottomLeft());
		path.lineTo(r.bottomRight());
		path.lineTo(r.left() + r.width() / 2, r.top());
		path.lineTo(r.bottomLeft());
		painter->fillPath(path, QBrush(d_pen.color()));
	} else {
		painter->drawPixmap(x0, y0, w, h, d_cursorPixmap);
		if (d_selected) {
			QPointF p = d_plotCoord;
			p.setX(plot()->axisInterval(fixedAxis()).minValue());

			painter->setPen(d_pen);
			painter->drawLine(x0, y0 + d_anchor.y(), transform(p).toPoint().x(), y0 + d_anchor.y());
		}
	}
}

/*
 * Class TriggerLevelCursor
 */

TriggerDelayCursor::TriggerDelayCursor(QObject *parent, const QPixmap &pixmap)
	: HorizDebugSymbol(parent, QSize(pixmap.size()), true), d_cursorPixmap(pixmap) {
	setAnchor(QPoint(surface().width() / 2, surface().height() / 2 - 6));
}

void TriggerDelayCursor::draw(QPainter *painter) const {
	int w = d_surface.width();
	int h = d_surface.height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();

	QwtInterval interval = plot()->axisInterval(mobileAxis());

	if (d_plotCoord.x() < interval.minValue()) {
		painter->setPen(d_pen);
		QRect r(x0 + 2, y0 + 2, w, h - 12);
		QPainterPath path;
		path.moveTo(r.topRight());
		path.lineTo(r.bottomRight());
		path.lineTo(r.left(), r.top() + r.height() / 2);
		path.lineTo(r.topRight());
		painter->fillPath(path, QBrush(d_pen.color()));
	} else if (d_plotCoord.x() > interval.maxValue()) {
		painter->setPen(d_pen);
		QRect r(x0 + 2, y0 + 2, w, h - 12);
		QPainterPath path;
		path.moveTo(r.topLeft());
		path.lineTo(r.bottomLeft());
		path.lineTo(r.right(), r.top() + r.height() / 2);
		path.lineTo(r.topLeft());
		painter->fillPath(path, QBrush(d_pen.color()));
	} else {
		QPointF p = d_plotCoord;
		p.setY(plot()->axisInterval(fixedAxis()).minValue());

		painter->drawPixmap(x0, y0, w, h, d_cursorPixmap);
		painter->setPen(d_pen);
		painter->drawLine(x0 + d_anchor.x(), y0 + h - 1, x0 + d_anchor.x(), transform(p).toPoint().y());
	}
}

/*
 * Class VertBar
 */

HorizBar::HorizBar(QObject *parent, bool floats) : VertDebugSymbol(parent, QSize(0, 0), false, floats) {}

void HorizBar::draw(QPainter *painter) const {
	int w = plot()->canvas()->width();
	int h = d_surface.height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();

	painter->setPen(d_pen);
	QwtPainter::drawLine(painter, x0, y0 + h / 2, x0 + w, y0 + h / 2);
}

/*
 * Class HorizBar
 */

VertBar::VertBar(QObject *parent, bool floats) : HorizDebugSymbol(parent, QSize(0, 0), true, floats) {}

void VertBar::draw(QPainter *painter) const {
	int w = d_surface.width();
	int h = plot()->canvas()->height();
	int x0 = d_surface.x();
	int y0 = d_surface.y();

	painter->setPen(d_pen);
	QwtPainter::drawLine(painter, x0 + w / 2, y0, x0 + w / 2, y0 + h);
}
