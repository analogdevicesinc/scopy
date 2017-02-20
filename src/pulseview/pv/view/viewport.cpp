/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <cassert>
#include <cmath>
#include <algorithm>
#include <limits>

#include "signal.hpp"
#include "view.hpp"
#include "viewitempaintparams.hpp"
#include "viewport.hpp"

#include "../session.hpp"

#include <QMouseEvent>

using std::abs;
using std::back_inserter;
using std::copy;
using std::dynamic_pointer_cast;
using std::max;
using std::min;
using std::none_of;
using std::numeric_limits;
using std::shared_ptr;
using std::stable_sort;
using std::vector;

namespace pv {
namespace view {

Viewport::Viewport(View &parent) :
	ViewWidget(parent),
	pinch_zoom_active_(false),
	timeTriggerPos(0)
{
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Base);
}

shared_ptr<ViewItem> Viewport::get_mouse_over_item(const QPoint &pt)
{
	const ViewItemPaintParams pp(rect(), view_.scale(), view_.offset(), view_.divisionCount());
	const vector< shared_ptr<ViewItem> > items(this->items());
	for (auto i = items.rbegin(); i != items.rend(); i++)
		if ((*i)->enabled() &&
			(*i)->hit_box_rect(pp).contains(pt))
			return *i;
	return nullptr;
}

void Viewport::disableDrag()
{
	dragEnabled = false;
}

void Viewport::enableDrag()
{
	dragEnabled = true;
}

void Viewport::item_hover(const shared_ptr<ViewItem> &item)
{
	if (item && item->is_draggable())
		setCursor(dynamic_pointer_cast<RowItem>(item) ?
			Qt::SizeVerCursor : Qt::SizeHorCursor);
	else
		unsetCursor();
}

void Viewport::drag()
{
	if(dragEnabled)
	{
		drag_offset_ = view_.offset();
		drag_v_offset_ = view_.owner_visual_v_offset();
	}
}

void Viewport::drag_by(const QPoint &delta)
{
	if(dragEnabled)
	{
		if (drag_offset_ == boost::none)
			return;

		view_.set_scale_offset(view_.scale(),
				       (*drag_offset_ - delta.x() * view_.scale() / (geometry().width() / divisionCount)));

		view_.set_v_offset(-drag_v_offset_ - delta.y());
	}
}

void Viewport::drag_release()
{
	drag_offset_ = boost::none;
}

vector< shared_ptr<ViewItem> > Viewport::items()
{
	vector< shared_ptr<ViewItem> > items;
	const std::vector< shared_ptr<ViewItem> > view_items(
		view_.list_by_type<ViewItem>());
	copy(view_items.begin(), view_items.end(), back_inserter(items));
	const vector< shared_ptr<TimeItem> > time_items(view_.time_items());
	copy(time_items.begin(), time_items.end(), back_inserter(items));
	return items;
}

bool Viewport::touch_event(QTouchEvent *event)
{
	QList<QTouchEvent::TouchPoint> touchPoints = event->touchPoints();

	if (touchPoints.count() != 2) {
		pinch_zoom_active_ = false;
		return false;
	}

	const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
	const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

	if (!pinch_zoom_active_ ||
	    (event->touchPointStates() & Qt::TouchPointPressed)) {
		pinch_offset0_ = (view_.offset() + view_.scale() * touchPoint0.pos().x()).convert_to<double>();
		pinch_offset1_ = (view_.offset() + view_.scale() * touchPoint1.pos().x()).convert_to<double>();
		pinch_zoom_active_ = true;
	}

	double w = touchPoint1.pos().x() - touchPoint0.pos().x();
	if (abs(w) >= 1.0) {
		const double scale =
			fabs((pinch_offset1_ - pinch_offset0_) / w);
		double offset = pinch_offset0_ - touchPoint0.pos().x() * scale;
		if (scale > 0)
			view_.set_scale_offset(scale, offset);
	}

	if (event->touchPointStates() & Qt::TouchPointReleased) {
		pinch_zoom_active_ = false;

		if (touchPoint0.state() & Qt::TouchPointReleased) {
			// Primary touch released
			drag_release();
		} else {
			// Update the mouse down fields so that continued
			// dragging with the primary touch will work correctly
			mouse_down_point_ = touchPoint0.pos().toPoint();
			drag();
		}
	}

	return true;
}

void Viewport::paintEvent(QPaintEvent*)
{
	vector< shared_ptr<RowItem> > row_items(view_.list_by_type<RowItem>());
	assert(none_of(row_items.begin(), row_items.end(),
		[](const shared_ptr<RowItem> &r) { return !r; }));

	stable_sort(row_items.begin(), row_items.end(),
		[](const shared_ptr<RowItem> &a, const shared_ptr<RowItem> &b) {
			return a->point(QRect()).y() < b->point(QRect()).y(); });

	const vector< shared_ptr<TimeItem> > time_items(view_.time_items());
	assert(none_of(time_items.begin(), time_items.end(),
		[](const shared_ptr<TimeItem> &t) { return !t; }));

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	const ViewItemPaintParams pp(rect(), view_.scale(), view_.offset(), view_.divisionCount());

	for (const shared_ptr<TimeItem> t : time_items)
		t->paint_back(p, pp);
	for (const shared_ptr<RowItem> r : row_items)
//		if (!r->isInitial())
			r->paint_back(p, pp);

	for (const shared_ptr<TimeItem> t : time_items)
		t->paint_mid(p, pp);
	for (const shared_ptr<RowItem> r : row_items)
//		if (!r->isInitial())
			r->paint_mid(p, pp);

	paint_grid(p, pp);

	for (const shared_ptr<RowItem> r : row_items)
//		if (!r->isInitial())
			r->paint_fore(p, pp);

	p.setRenderHint(QPainter::Antialiasing, false);
	for (const shared_ptr<TimeItem> t : time_items)
		t->paint_fore(p, pp);

	if( timeTriggerPos != 0 )
	{
		paint_time_trigger_line(p, pp, timeTriggerPos);
	}

	p.end();
}

void Viewport::setTimeTriggerPos(int value)
{
	timeTriggerPos = value;
	view_.time_item_appearance_changed(true, true);
}

int Viewport::getTimeTriggerPos() const
{
	return timeTriggerPos;
}

void Viewport::paint_time_trigger_line(QPainter &p, const ViewItemPaintParams &pp, int pos)
{
	QPen pen = QPen(QColor(74, 100, 255));
	p.setPen(pen);
	const int y = view_.owner_visual_v_offset();
	const int h = pp.height();
	int row_count = view_.height() / divisionHeight;

	QPoint p1 = QPoint(pos, y);
	QPoint p2 = QPoint(pos, y + h * row_count);
	p.drawLine(p1, p2);
}

void Viewport::paint_grid(QPainter &p, const ViewItemPaintParams &pp)
{
	const int x = pp.left();
	const int w = pp.right() -pp.left();
	const int h = pp.height();
    const int y = view_.owner_visual_v_offset();//pp.top();

    int division_height = divisionHeight;
    int division_count = divisionCount;
    int division_offset = divisionOffset;

	int division_width = w / division_count;    
    int row_count = view_.height() / division_height;

	QPointF p1, p2;

	p.setRenderHint(QPainter::Antialiasing, false);
	QPen pen = QPen(QColor(255, 255, 255, 30*256/100));
	for (int i = 0; i <= division_count; i++) {
		pen.setStyle((i % 2) ? Qt::SolidLine : Qt::DashLine);
		p.setPen(pen);

		p1 = QPointF(x + i * division_width, y);
		p2 = QPointF(x + i * division_width, y + h * row_count);
		p.drawLine(p1, p2);
	}

	p.setRenderHint(QPainter::Antialiasing, true);    
    paint_axis(p,pp,pp.top());
   /* for (int i = 0; i <= row_count; i++) {
        if(y + divisionOffset+ division_height * i != 0)
            paint_axis(p, pp, y + divisionOffset + division_height * i);
	}*/
}

void Viewport::paint_axis(QPainter &p, const ViewItemPaintParams &pp, int y)
{
	p.setRenderHint(QPainter::Antialiasing, false);

	p.setPen(QPen(QColor(255, 255, 255, 30*256/100)));
	p.drawLine(QPointF(pp.left(), y), QPointF(pp.right(), y));

	p.setRenderHint(QPainter::Antialiasing, true);
}

void Viewport::mouseDoubleClickEvent(QMouseEvent *event)
{
	assert(event);

	if (event->buttons() & Qt::LeftButton)
		view_.zoom(2.0, event->x());
	else if (event->buttons() & Qt::RightButton)
		view_.zoom(-2.0, event->x());
}

void Viewport::wheelEvent(QWheelEvent *event)
{
	assert(event);

	if (event->orientation() == Qt::Vertical) {
		if (event->modifiers() & Qt::ControlModifier) {
			// Vertical scrolling with the control key pressed
			// is intrepretted as vertical scrolling
			view_.set_v_offset(-view_.owner_visual_v_offset() -
				(event->delta() * height()) / (8 * 120));
		} else {
			// Vertical scrolling is interpreted as zooming in/out
			view_.zoom(event->delta() / 120, event->x());
		}
	} else if (event->orientation() == Qt::Horizontal) {
		// Horizontal scrolling is interpreted as moving left/right
		view_.set_scale_offset(view_.scale(),
			event->delta() * view_.scale() + view_.offset());
	}
}

int Viewport::getDivisionOffset() const
{
    return divisionOffset;
}

void Viewport::setDivisionOffset(int value)
{
    divisionOffset = value;
}

int Viewport::getDivisionCount() const
{
    return divisionCount;
}

void Viewport::setDivisionCount(int value)
{
    divisionCount = value;
}

int Viewport::getDivisionHeight() const
{
    return divisionHeight;
}

void Viewport::setDivisionHeight(int value)
{
    divisionHeight = value;
}

} // namespace view
} // namespace pv
