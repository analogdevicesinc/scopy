#include "mouseplotmagnifier.hpp"
#include <DisplayPlot.h>

using namespace scopy;

MousePlotMagnifier::MousePlotMagnifier(QWidget *canvas)
	: QObject(canvas)
	, m_canvas(canvas)
	, m_factor(0.95)
	, m_cursorPos(QPoint())
	, m_zoomBase(QRectF())
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
	, m_xAxisEn(true)
	, m_yAxisEn(true)
	, m_isZoomed(false)
	, m_en(false)
	, m_bounded(true)
	, m_zoomModifier(Qt::NoModifier)
	, m_panModifier(Qt::ShiftModifier)
{
	canvas->setFocusPolicy(Qt::WheelFocus);
	setEnabled(true);

	connect(this, &MousePlotMagnifier::reset, this, &MousePlotMagnifier::zoomToBase);
}

MousePlotMagnifier::~MousePlotMagnifier() {}

void MousePlotMagnifier::setXAxis(QwtAxisId axisId) { m_xAxis = axisId; }

void MousePlotMagnifier::setYAxis(QwtAxisId axisId) { m_yAxis = axisId; }

void MousePlotMagnifier::setXAxisEnabled(bool en) { m_xAxisEn = en; }

void MousePlotMagnifier::setYAxisEnabled(bool en) { m_yAxisEn = en; }

bool MousePlotMagnifier::isXAxisEnabled() const { return m_xAxisEn; }

bool MousePlotMagnifier::isYAxisEnabled() const { return m_yAxisEn; }

QRectF MousePlotMagnifier::zoomBase() const { return QRectF(m_zoomBase); }

bool MousePlotMagnifier::isZoomed() const { return m_isZoomed; }

void MousePlotMagnifier::setZoomModifier(Qt::KeyboardModifier modifier) { m_zoomModifier = modifier; }

Qt::KeyboardModifier MousePlotMagnifier::getZoomModifier() { return m_zoomModifier; }

void MousePlotMagnifier::setPanModifier(Qt::KeyboardModifier modifier) { m_panModifier = modifier; }

Qt::KeyboardModifier MousePlotMagnifier::getPanModifier() { return m_panModifier; }

double MousePlotMagnifier::getFactor() const { return m_factor; }

void MousePlotMagnifier::setFactor(double factor) { m_factor = factor; }

QwtPlot *MousePlotMagnifier::getPlot()
{
	QWidget *plot = m_canvas;
	if(plot) {
		plot = plot->parentWidget();
	}

	return qobject_cast<QwtPlot *>(plot);
}

void MousePlotMagnifier::setBaseRect(const QRectF &rect) { m_zoomBase = rect; }

void MousePlotMagnifier::setEnabled(bool en)
{
	m_en = en;

	if(m_en) {
		m_canvas->installEventFilter(this);
	} else {
		m_canvas->removeEventFilter(this);
	}
}

bool MousePlotMagnifier::isEnabled() const { return m_en; }

void MousePlotMagnifier::silentZoom(double factor, QPointF cursorPos)
{
	blockSignals(true);
	zoom(factor, cursorPos);
	blockSignals(false);
}

void MousePlotMagnifier::silentPan(double factor)
{
	blockSignals(true);
	pan(factor);
	blockSignals(false);
}

void MousePlotMagnifier::zoom(double factor, QPointF cursorPos)
{
	if(factor == -1) {
		zoomToBase();
	} else {
		m_cursorPos = cursorPos;
		zoomRescale(factor);
	}
}

void MousePlotMagnifier::pan(double factor)
{
	if(factor == -1) {
		zoomToBase();
	} else {
		panRescale(factor);
	}
}

void MousePlotMagnifier::setBounded(bool en) { m_bounded = en; }

bool MousePlotMagnifier::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) { // reset to base rect
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);

		if(mouseEvent->button() == Qt::MouseButton::RightButton && m_isZoomed && m_en) {
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

void MousePlotMagnifier::panRescale(double factor)
{
	if(isXAxisEnabled()) {
		m_isZoomed = true;
		const QwtAxisId axisId = m_xAxis;

		double v1 = getPlot()->axisInterval(axisId).minValue();
		double v2 = getPlot()->axisInterval(axisId).maxValue();
		double pan_amount = ((v2 - v1) - (v2 - v1) * factor) * 0.5;
		v1 += pan_amount;
		v2 += pan_amount;

		// limit zoom to zoomBase
		if(m_bounded) {
			if(v1 < m_zoomBase.left()) {
				v2 = m_zoomBase.left() + (v2 - v1);
				v1 = m_zoomBase.left();
			} else if(v2 > m_zoomBase.right()) {
				v1 = m_zoomBase.right() - (v2 - v1);
				v2 = m_zoomBase.right();
			}
		}

		setPlotAxisScale(axisId, v1, v2);

		getPlot()->replot();
		Q_EMIT panned(factor);
	}
}

void MousePlotMagnifier::zoomRescale(double factor)
{
	QwtPlot *plot = getPlot();
	bool zoom = false;

	for(QwtAxisId axisId : {m_xAxis, m_yAxis}) {
		if((QwtAxis::isXAxis(axisId) && isXAxisEnabled()) || (QwtAxis::isYAxis(axisId) && isYAxisEnabled())) {
			const QwtScaleMap scaleMap = plot->canvasMap(axisId);
			double cursor = 0, v1 = 0, v2 = 0; // position of the cursor in the axis coordinates

			// we use the painted plot as reference
			if(QwtAxis::isXAxis(axisId)) {
				cursor = m_cursorPos.x();
				v2 = plot->canvas()->width();
			}
			if(QwtAxis::isYAxis(axisId)) {
				cursor = m_cursorPos.y();
				v1 = plot->canvas()->height();
			}

			// calculate new axis scale
			const double center = 0.5 * (v1 + v2);
			const double width = 0.5 * (v2 - v1) * factor;
			const double newCenter = cursor - factor * (cursor - center);
			v1 = scaleMap.invTransform(newCenter - width);
			v2 = scaleMap.invTransform(newCenter + width);

			// limit zoom to zoomBase
			if(QwtAxis::isXAxis(axisId) && m_bounded) {
				v1 = std::max(v1, m_zoomBase.left());
				v2 = std::min(v2, m_zoomBase.right());
			}
			if(QwtAxis::isYAxis(axisId) && m_bounded) {
				v1 = std::max(v1, m_zoomBase.top());
				v2 = std::min(v2, m_zoomBase.bottom());
			}

			setPlotAxisScale(axisId, v1, v2);
			zoom = true;
		}
	}

	if(zoom) {
		plot->replot();
		m_isZoomed = true;
		Q_EMIT zoomed(factor, m_cursorPos);
	}
}

void MousePlotMagnifier::setPlotAxisScale(QwtAxisId axisId, double min, double max)
{
	getPlot()->setAxisScale(axisId, min, max);

	adiscope::DisplayPlot *plt = dynamic_cast<adiscope::DisplayPlot *>(getPlot());
	if(plt) {
		if(QwtAxis::isXAxis(axisId)) {
			double width = qAbs(max - min);
			plt->setHorizUnitsPerDiv(width / plt->xAxisNumDiv());
			plt->setHorizOffset(min + (width / 2));
		}
		if(QwtAxis::isYAxis(axisId)) {
			double height = qAbs(max - min);
			plt->setVertUnitsPerDiv(height / plt->yAxisNumDiv(), axisId.id);
			plt->setVertOffset(min + (height / 2), axisId.id);
		}
	}
}

void MousePlotMagnifier::zoomToBase()
{
	if(isXAxisEnabled()) {
		setPlotAxisScale(m_xAxis, m_zoomBase.left(), m_zoomBase.right());
	}
	if(isYAxisEnabled()) {
		setPlotAxisScale(m_yAxis, m_zoomBase.top(), m_zoomBase.bottom());
	}

	getPlot()->replot();
	m_isZoomed = false;
	Q_EMIT zoomed(-1);
}
