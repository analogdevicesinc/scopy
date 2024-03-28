#include "plotmagnifier.hpp"
#include "qevent.h"
#include <qwt_interval.h>
#include <qwt_scale_div.h>
#include <cmath>

using namespace scopy;

PlotMagnifier::PlotMagnifier(QwtPlot *plot)
	: QObject(plot)
	, m_plot(plot)
	, m_canvas(plot->canvas())
	, m_factor(0.95)
	, m_cursorPos(QPoint())
	, m_baseRect(QRectF())
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
	, m_xAxisEn(true)
	, m_yAxisEn(true)
	, m_isZoomed(false)
	, m_en(false)
	, m_bounded(true)
	, m_zoomModifier(Qt::NoModifier)
	, m_panModifier(Qt::ShiftModifier)
	, m_blockReset(false)
{
	m_canvas->setFocusPolicy(Qt::WheelFocus);
	setEnabled(true);

	connect(this, &PlotMagnifier::reset, this, &PlotMagnifier::zoomToBase);
}

PlotMagnifier::~PlotMagnifier() {}

void PlotMagnifier::setXAxis(QwtAxisId axisId) { m_xAxis = axisId; }

void PlotMagnifier::setYAxis(QwtAxisId axisId) { m_yAxis = axisId; }

void PlotMagnifier::setXAxisEn(bool en) { m_xAxisEn = en; }

void PlotMagnifier::setYAxisEn(bool en) { m_yAxisEn = en; }

bool PlotMagnifier::isXAxisEn() const { return m_xAxisEn; }

bool PlotMagnifier::isYAxisEn() const { return m_yAxisEn; }

QRectF PlotMagnifier::zoomBase() const { return QRectF(m_baseRect); }

bool PlotMagnifier::isZoomed() const { return m_isZoomed; }

void PlotMagnifier::setZoomModifier(Qt::KeyboardModifier modifier) { m_zoomModifier = modifier; }

Qt::KeyboardModifier PlotMagnifier::getZoomModifier() { return m_zoomModifier; }

void PlotMagnifier::setPanModifier(Qt::KeyboardModifier modifier) { m_panModifier = modifier; }

Qt::KeyboardModifier PlotMagnifier::getPanModifier() { return m_panModifier; }

double PlotMagnifier::getFactor() const { return m_factor; }

void PlotMagnifier::setFactor(double factor) { m_factor = factor; }

QwtAxisId PlotMagnifier::getXAxis() { return m_xAxis; }

QwtAxisId PlotMagnifier::getYAxis() { return m_yAxis; }

QwtPlot *PlotMagnifier::plot() { return m_plot; }

void PlotMagnifier::setBaseRect(const QRectF &rect) { m_baseRect = rect; }

void PlotMagnifier::setBaseRect() { m_baseRect = getCurrentRect(); }

QRectF PlotMagnifier::getCurrentRect()
{
	double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

	if(m_xAxisEn) {
		x1 = plot()->axisScaleDiv(m_xAxis).lowerBound();
		x2 = plot()->axisScaleDiv(m_xAxis).upperBound();
	}
	if(m_yAxisEn) {
		y1 = plot()->axisScaleDiv(m_yAxis).lowerBound();
		y2 = plot()->axisScaleDiv(m_yAxis).upperBound();
	}

	return QRectF(x1, y1, x2 - x1, y2 - y1);
}

void PlotMagnifier::setEnabled(bool en)
{
	m_en = en;

	if(m_en) {
		m_canvas->installEventFilter(this);
	} else {
		m_canvas->removeEventFilter(this);
	}
}

bool PlotMagnifier::isEnabled() const { return m_en; }

void PlotMagnifier::silentZoom(double factor, QPointF cursorPos)
{
	blockSignals(true);
	zoom(factor, cursorPos);
	blockSignals(false);
}

void PlotMagnifier::silentPan(double factor)
{
	blockSignals(true);
	pan(factor);
	blockSignals(false);
}

void PlotMagnifier::zoom(double factor, QPointF cursorPos)
{
	if(factor == -1) {
		zoomToBase();
	} else {
		m_cursorPos = cursorPos;
		zoomRescale(factor);
	}
}

void PlotMagnifier::pan(double factor)
{
	if(factor == -1) {
		zoomToBase();
	} else {
		panRescale(factor);
	}
}

void PlotMagnifier::setBounded(bool en) { m_bounded = en; }

bool PlotMagnifier::isBounded() { return m_bounded; }

void PlotMagnifier::setBlockZoomResetEn(bool en) { m_blockReset = en; }

bool PlotMagnifier::isBlockZoomResetEn() { return m_blockReset; }

bool PlotMagnifier::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) { // reset to base rect
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);

		if(!m_blockReset && m_isZoomed && mouseEvent->button() == Qt::MouseButton::RightButton) {
			Q_EMIT reset();
		}
	} else if(event->type() == QEvent::Wheel) { // zoom or pan
		QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent *>(event);
		m_cursorPos = wheelEvent->position();

		if(getFactor() != 0.0) {
			/*
			    A positive delta indicates that the wheel was
			    rotated forwards away from the user; a negative
			    value indicates that the wheel was rotated
			    backwards toward the user.
			    Most mouse types work in steps of 15 degrees,
			    in which case the delta value is a multiple
			    of 120 (== 15 * 8).
			 */

			const QPoint delta = wheelEvent->angleDelta();
			const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
			double factor = std::pow(getFactor(), qAbs(wheelDelta / 120.0));

			if(wheelDelta <= 0) {
				factor = 1 / factor;
			}

			if(wheelEvent->modifiers().testFlag(getZoomModifier())) {
				zoomRescale(factor);
			} else if(wheelEvent->modifiers().testFlag(getPanModifier())) {
				panRescale(factor);
			}
		}
	}

	return QObject::eventFilter(object, event);
}

void PlotMagnifier::panRescale(double factor)
{
	if(isXAxisEn()) {
		m_isZoomed = true;
		const QwtAxisId axisId = m_xAxis;

		double v1 = plot()->axisInterval(axisId).minValue();
		double v2 = plot()->axisInterval(axisId).maxValue();
		double pan_amount = (((v2 - v1) - (v2 - v1) * factor) * 0.5);
		bool isInverted = v1 > v2;

		if(isInverted) {
			pan_amount *= -1;
		}

		v1 += pan_amount;
		v2 += pan_amount;

		// limit zoom to zoomBase
		if(m_bounded && !m_baseRect.isNull()) {
			if((isInverted && v1 > m_baseRect.left()) || (!isInverted && v1 < m_baseRect.left())) {
				v2 = m_baseRect.left() + (v2 - v1);
				v1 = m_baseRect.left();
			} else if((isInverted && v2 < m_baseRect.right()) || (!isInverted && v2 > m_baseRect.right())) {
				v1 = m_baseRect.right() - (v2 - v1);
				v2 = m_baseRect.right();
			}
		}

		plot()->setAxisScale(axisId, v1, v2);

		plot()->replot();
		Q_EMIT panned(factor);
	}
	Q_EMIT pannedRect(getCurrentRect());
}

void PlotMagnifier::zoomRescale(double factor)
{
	bool zoom = false;

	for(QwtAxisId axisId : {m_xAxis, m_yAxis}) {
		if((QwtAxis::isXAxis(axisId) && isXAxisEn()) || (QwtAxis::isYAxis(axisId) && isYAxisEn())) {
			const QwtScaleMap scaleMap = plot()->canvasMap(axisId);
			double cursor = 0, v1 = 0, v2 = 0; // position of the cursor in the axis coordinates

			// we use the painted plot as reference
			if(QwtAxis::isXAxis(axisId)) {
				cursor = m_cursorPos.x();
				v2 = m_canvas->width();
			}
			if(QwtAxis::isYAxis(axisId)) {
				cursor = m_cursorPos.y();
				v1 = m_canvas->height();
			}

			// calculate new axis scale
			const double center = 0.5 * (v1 + v2);
			const double width = 0.5 * (v2 - v1) * factor;
			const double newCenter = cursor - factor * (cursor - center);
			v1 = scaleMap.invTransform(newCenter - width);
			v2 = scaleMap.invTransform(newCenter + width);

			// limit zoom to zoomBase
			if(m_bounded && !m_baseRect.isNull()) {
				bool isInverted = v1 > v2;
				if(QwtAxis::isXAxis(axisId)) {
					v1 = isInverted ? std::min(v1, m_baseRect.left())
							: std::max(v1, m_baseRect.left());
					v2 = isInverted ? std::max(v2, m_baseRect.right())
							: std::min(v2, m_baseRect.right());
				}
				if(QwtAxis::isYAxis(axisId)) {
					v1 = isInverted ? std::min(v1, m_baseRect.top())
							: std::max(v1, m_baseRect.top());
					v2 = isInverted ? std::max(v2, m_baseRect.bottom())
							: std::min(v2, m_baseRect.bottom());
				}
			}

			plot()->setAxisScale(axisId, v1, v2);
			zoom = true;
		}
	}

	if(zoom) {
		plot()->replot();
		m_isZoomed = true;
		Q_EMIT zoomed(factor, m_cursorPos);
	}
	Q_EMIT zoomedRect(getCurrentRect());
}

void PlotMagnifier::zoomToBase()
{
	if(m_baseRect.isNull()) {
		return;
	}

	if(isXAxisEn()) {
		plot()->setAxisScale(m_xAxis, m_baseRect.left(), m_baseRect.right());
	}
	if(isYAxisEn()) {
		plot()->setAxisScale(m_yAxis, m_baseRect.top(), m_baseRect.bottom());
	}

	plot()->replot();
	m_isZoomed = false;
}

#include "moc_plotmagnifier.cpp"
