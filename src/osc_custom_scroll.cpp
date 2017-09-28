#include "osc_custom_scroll.h"

#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QPointf>

using namespace adiscope;

OscCustomScrollArea::OscCustomScrollArea(QWidget *parent):
	QScrollArea(parent),
	inside(false),
	disableCursor(true)
{
	QScroller::grabGesture(this->viewport(), QScroller::LeftMouseButtonGesture);

	scroll = QScroller::scroller(this->viewport());

	QScrollerProperties properties = QScroller::scroller(this->viewport())->scrollerProperties();

	QVariant overshootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootWhenScrollable);
	properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, overshootPolicy);

	QScroller::scroller(this->viewport())->setScrollerProperties(properties);

	connect(scroll, &QScroller::stateChanged, [=](QScroller::State newstate){
		if (disableCursor)
				return;
		switch (newstate) {
		case QScroller::Inactive :
			if (inside)
				QApplication::setOverrideCursor(Qt::OpenHandCursor);
			break;
		case QScroller::Pressed :
			QApplication::setOverrideCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Dragging :
			QApplication::setOverrideCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Scrolling :
			if (inside)
				QApplication::setOverrideCursor(Qt::OpenHandCursor);
			break;
		default:
			QApplication::setOverrideCursor(Qt::ArrowCursor);
			break;
		}
	});

	connect(horizontalScrollBar(), &QScrollBar::rangeChanged, [=](int v1, int v2){
		if (v2 - v1 == 0)
			disableCursor = true;
		else
			disableCursor = false;
	});
}

void OscCustomScrollArea::enterEvent(QEvent *event)
{
	if (!disableCursor)
		QApplication::setOverrideCursor(Qt::OpenHandCursor);
	inside = true;
}

void OscCustomScrollArea::leaveEvent(QEvent *event)
{
	if (!disableCursor)
		QApplication::setOverrideCursor(Qt::ArrowCursor);
	inside = false;
}



