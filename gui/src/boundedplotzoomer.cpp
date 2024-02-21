#include "boundedplotzoomer.hpp"
#include <qwt_scale_div.h>
#include <stylehelper.h>

using namespace scopy;

BoundedPlotZoomer::BoundedPlotZoomer(QwtPlot *plot)
	: QObject(plot)
	, m_plot(plot)
	, m_canvas(plot->canvas())
	, m_baseRect(QRectF())
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
	, m_xAxisEn(true)
	, m_yAxisEn(true)
	, m_isZoomed(false)
	, m_en(false)
	, m_bounded(true)
	, m_rubberBand(nullptr)
	, m_minZoom(10)
	, m_drawZoom(true)
	, m_blockReset(false)
	, m_blockZoom(false)
{
	setEnabled(true);
	connect(this, &BoundedPlotZoomer::reset, this, &BoundedPlotZoomer::zoomToBase);
}

BoundedPlotZoomer::~BoundedPlotZoomer() {}

void BoundedPlotZoomer::setXAxis(QwtAxisId axisId) { m_xAxis = axisId; }

void BoundedPlotZoomer::setYAxis(QwtAxisId axisId) { m_yAxis = axisId; }

void BoundedPlotZoomer::setXAxisEn(bool en) { m_xAxisEn = en; }

void BoundedPlotZoomer::setYAxisEn(bool en) { m_yAxisEn = en; }

bool BoundedPlotZoomer::isXAxisEn() const { return m_xAxisEn; }

bool BoundedPlotZoomer::isYAxisEn() const { return m_yAxisEn; }

QRectF BoundedPlotZoomer::zoomBase() const { return QRectF(m_baseRect); }

bool BoundedPlotZoomer::isZoomed() const { return m_isZoomed; }

void BoundedPlotZoomer::setMinimumZoom(uint pixels) { m_minZoom = pixels; }

uint BoundedPlotZoomer::getMinimumZoom() { return m_minZoom; }

bool BoundedPlotZoomer::isDrawZoomRectEn() { return m_drawZoom; }

bool BoundedPlotZoomer::isBlockZoomEn() { return m_blockZoom; }

void BoundedPlotZoomer::setDrawZoomRectEn(bool en) { m_drawZoom = en; }

void BoundedPlotZoomer::setBlockZoomEn(bool en) { m_blockZoom = en; }

void BoundedPlotZoomer::setBlockZoomResetEn(bool en) { m_blockReset = en; }

bool BoundedPlotZoomer::isBlockZoomResetEn() { return m_blockReset; }

QwtAxisId BoundedPlotZoomer::getXAxis() { return m_xAxis; }

QwtAxisId BoundedPlotZoomer::getYAxis() { return m_yAxis; }

QwtPlot *BoundedPlotZoomer::plot() { return m_plot; }

void BoundedPlotZoomer::setBaseRect(const QRectF &rect) { m_baseRect = rect; }

void BoundedPlotZoomer::setBaseRect() { m_baseRect = getCurrentRect(); }

void BoundedPlotZoomer::setEnabled(bool en)
{
	m_en = en;

	if(m_en) {
		m_canvas->installEventFilter(this);
	} else {
		m_canvas->removeEventFilter(this);
	}
}

bool BoundedPlotZoomer::isEnabled() const { return m_en; }

void BoundedPlotZoomer::silentZoom(const QRectF &rect)
{
	blockSignals(true);
	zoom(rect);
	blockSignals(false);
}

void BoundedPlotZoomer::zoom(const QRectF &rect)
{
	if(!m_blockZoom) {
		rescale(rect);
	}
}

void BoundedPlotZoomer::setBounded(bool en) { m_bounded = en; }

bool BoundedPlotZoomer::isBoundEn() { return m_bounded; }

QRectF BoundedPlotZoomer::getCurrentRect()
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

bool BoundedPlotZoomer::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) { // reset to base rect
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);

		if(!m_blockReset && m_isZoomed && mouseEvent->button() == Qt::MouseButton::RightButton) {
			Q_EMIT reset();
		} else if(m_rubberBand && mouseEvent->button() == Qt::MouseButton::LeftButton) {
			onZoomEnd();
		}
	} else if(event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::MouseButton::LeftButton) {
			onZoomStart(mouseEvent);
		}
	} else if(m_rubberBand && event->type() == QEvent::MouseMove) {
		onZoomResize(dynamic_cast<QMouseEvent *>(event));
	}

	return QObject::eventFilter(object, event);
}

void BoundedPlotZoomer::rescale(const QRectF &rect)
{
	if(m_xAxisEn) {
		plot()->setAxisScale(m_xAxis, rect.left(), rect.right());
	}
	if(m_yAxisEn) {
		plot()->setAxisScale(m_yAxis, rect.top(), rect.bottom());
	}

	plot()->replot();
	m_isZoomed = true;
	Q_EMIT zoomed(rect);
}

void BoundedPlotZoomer::onZoomResize(QMouseEvent *event)
{
	double x = m_xAxisEn ? event->pos().x() : m_canvas->rect().right();
	double y = m_yAxisEn ? event->pos().y() : m_canvas->rect().bottom();

	x = std::min(x, double(m_canvas->rect().right()));
	x = std::max(x, double(m_canvas->rect().left()));

	y = std::min(y, double(m_canvas->rect().bottom()));
	y = std::max(y, double(m_canvas->rect().top()));

	m_rubberBand->setGeometry(QRect(m_origin, QPoint(x, y)).normalized());

	QPalette palette;
	if(m_rubberBand->geometry().size().width() < m_minZoom ||
	   m_rubberBand->geometry().size().height() < m_minZoom) {
		palette.setBrush(QPalette::Highlight, QBrush(QColor(StyleHelper::getColor("ProgressBarError"))));
	} else {
		palette.setBrush(QPalette::Highlight, QBrush(QColor(StyleHelper::getColor("ScopyBackground"))));
	}
	m_rubberBand->setPalette(palette);
}

void BoundedPlotZoomer::onZoomStart(QMouseEvent *event)
{
	m_origin = QPoint(m_xAxisEn ? event->pos().x() : 0, m_yAxisEn ? event->pos().y() : 0);
	m_rubberBand = new QRubberBand(QRubberBand::Rectangle, m_canvas);
	m_rubberBand->setGeometry(QRect(m_origin, QSize()));

	if(m_drawZoom) {
		m_rubberBand->show();
	}
}

void BoundedPlotZoomer::onZoomEnd()
{
	m_rubberBand->hide();
	if(m_rubberBand->palette().color(QPalette::Highlight) == QColor(StyleHelper::getColor("ScopyBackground"))) {
		QRectF rect = m_rubberBand->geometry();
		QwtScaleMap xScaleMap = plot()->canvasMap(m_xAxis);
		QwtScaleMap yScaleMap = plot()->canvasMap(m_yAxis);
		QRectF zoomRect = QwtScaleMap::invTransform(xScaleMap, yScaleMap, rect);

		if(m_bounded && !m_baseRect.isNull()) {
			zoomRect.setX(std::min(zoomRect.x(), double(m_baseRect.right())));
			zoomRect.setX(std::max(zoomRect.x(), double(m_baseRect.left())));

			zoomRect.setY(std::min(zoomRect.y(), double(m_baseRect.bottom())));
			zoomRect.setY(std::max(zoomRect.y(), double(m_baseRect.top())));
		}

		zoom(zoomRect);
	}
}

void BoundedPlotZoomer::zoomToBase()
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
