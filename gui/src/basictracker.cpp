#include "basictracker.hpp"
#include "stylehelper.h"
#include <QwtText>

using namespace scopy;

BasicTracker::BasicTracker(QwtPlot *plot)
	: QwtWidgetOverlay(plot->canvas())
	, m_plot(plot)
	, m_canvas(plot->canvas())
	, m_xAxisEn(true)
	, m_yAxisEn(true)
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
	, m_color(QColor(StyleHelper::getColor("LabelText")))
	, m_xAxisUnit("")
	, m_yAxisUnit("")
	, m_xFormatter(nullptr)
	, m_yFormatter(nullptr)
{
	m_canvas->setMouseTracking(true);
	m_canvas->installEventFilter(this);
	hide();
}

BasicTracker::~BasicTracker() {}

QwtPlot *BasicTracker::plot() const { return m_plot; }

void BasicTracker::setXAxis(QwtAxisId axisId) { m_xAxis = axisId; }

QwtAxisId BasicTracker::getXAxis() { return m_xAxis; }

void BasicTracker::setYAxis(QwtAxisId axisId) { m_yAxis = axisId; }

QwtAxisId BasicTracker::getYAxis() { return m_yAxis; }

void BasicTracker::setXAxisEn(bool en) { m_xAxisEn = en; }

void BasicTracker::setYAxisEn(bool en) { m_yAxisEn = en; }

bool BasicTracker::isXAxisEn() const { return m_xAxisEn; }

bool BasicTracker::isYAxisEn() const { return m_yAxisEn; }

void BasicTracker::setXFormatter(PrefixFormatter *formatter) { m_xFormatter = formatter; }

PrefixFormatter *BasicTracker::getXFormatter() { return m_xFormatter; }

void BasicTracker::setYFormatter(PrefixFormatter *formatter) { m_yFormatter = formatter; }

PrefixFormatter *BasicTracker::getYFormatter() { return m_yFormatter; }

void BasicTracker::setXAxisUnit(QString unit) { m_xAxisUnit = unit; }

QString BasicTracker::getXAxisUnit() { return m_xAxisUnit; }

void BasicTracker::setYAxisUnit(QString unit) { m_yAxisUnit = unit; }

QString BasicTracker::getYAxisUnit() { return m_yAxisUnit; }

void BasicTracker::setColor(QColor color) { m_color = color; }

QRect BasicTracker::trackerRect(QSizeF size) const
{
	QPoint bottomRight = QPoint(m_mousePos.x(), m_mousePos.y() - 10);
	QPoint topLeft = QPoint(bottomRight.x() - size.width(), bottomRight.y() - size.height());

	// calculate offset if text goes outside the canvas
	int xOffset = 0;
	if(topLeft.x() < 0) {
		xOffset = -topLeft.x();
	} else if(bottomRight.x() > m_canvas->width()) {
		xOffset = m_canvas->width() - bottomRight.x();
	}

	int yOffset = 0;
	if(topLeft.y() < 0) {
		yOffset = -topLeft.y();
	} else if(bottomRight.y() > m_canvas->height()) {
		yOffset = m_canvas->height() - bottomRight.y();
	}

	topLeft.rx() += xOffset;
	bottomRight.rx() += xOffset;
	topLeft.ry() += yOffset;
	bottomRight.ry() += yOffset;

	return QRect(topLeft, bottomRight);
}

QwtText *BasicTracker::trackerText(QPoint pos) const
{
	QString xText = "";
	QString yText = "";
	QString separator = "";
	const uint precision = 3;
	const char format = 'g';

	if(m_xAxisEn) {
		double value = plot()->canvasMap(m_xAxis).invTransform(pos.x());
		if(m_xFormatter && !m_xAxisUnit.isEmpty()) {
			xText = m_xFormatter->format(value, m_xAxisUnit, precision);
		} else {
			xText = QString::number(value, format, precision) + " " + m_xAxisUnit;
		}
	}
	if(m_yAxisEn) {
		double value = plot()->canvasMap(m_yAxis).invTransform(pos.y());
		if(m_yFormatter && !m_yAxisUnit.isEmpty()) {
			yText = m_yFormatter->format(value, m_yAxisUnit, precision);
		} else {
			yText = QString::number(value, format, precision) + " " + m_yAxisUnit;
		}
	}
	if(m_xAxisEn && m_yAxisEn) {
		separator = ", ";
	}

	QwtText *text = new QwtText(xText + separator + yText);
	text->setColor(m_color);

	return text;
}

void BasicTracker::drawOverlay(QPainter *painter) const
{
	if(isEnabled()) {
		QwtText *label = trackerText(m_mousePos);

		if(!label->isEmpty()) {
			label->draw(painter, trackerRect(label->textSize()));
		}
	}
}

bool BasicTracker::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseMove) {
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
		m_mousePos = mouseEvent->pos();
	}
	if(event->type() == QEvent::Leave && isVisible()) {
		hide();
	}
	if(event->type() == QEvent::Enter && !isVisible()) {
		show();
	}

	return QwtWidgetOverlay::eventFilter(object, event);
}
