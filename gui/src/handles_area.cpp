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

#include "handles_area.hpp"

#include "handlesareaextension.h"
#include "plot_line_handle.h"

#include <QDebug>
#include <QtWidgets>

/*
 * Class HandlesArea implementation
 */

HandlesArea::HandlesArea(QWidget *parent)
	: QWidget(parent)
	, canvas(parent)
	, selectedItem(NULL)
{
	this->setStyleSheet("QWidget {background-color: transparent}");
}

void HandlesArea::installExtension(std::unique_ptr<HandlesAreaExtension> extension)
{
	m_extensions.push_back(std::move(extension));
}

void HandlesArea::mousePressEvent(QMouseEvent *event)
{
	PlotLineHandle *child = static_cast<PlotLineHandle *>(childAt(event->pos()));

	if(child) {
		child->raise();
		selectedItem = child;
		hotspot = event->pos() - child->pos();
		child->setGrabbed(true);
	}
	event->accept();
}

void HandlesArea::mouseReleaseEvent(QMouseEvent *)
{
	if(selectedItem) {
		selectedItem->setGrabbed(false);
		selectedItem = NULL;
	}
}

void HandlesArea::mouseMoveEvent(QMouseEvent *event)
{
	if(selectedItem) {
		int x = event->pos().x() - hotspot.x();
		int y = event->pos().y() - hotspot.y();

		selectedItem->moveWithinParent(x, y);
	}
}

void HandlesArea::resizeEvent(QResizeEvent *event)
{
	resizeMask(event->size());

	Q_EMIT sizeChanged(event->size());
}

void HandlesArea::paintEvent(QPaintEvent *event)
{
	QPainter p(this);

	for(std::unique_ptr<HandlesAreaExtension> &extension : m_extensions) {
		if(extension->draw(&p, this)) {
			break;
		}
	}

	QWidget::paintEvent(event);
}

/*
 * Class VertHandlesArea implementation
 */

VertHandlesArea::VertHandlesArea(QWidget *parent)
	: HandlesArea(parent)
{
	largest_child_height = 0;
	top_padding = 50;
	bottom_padding = 50;
}

int VertHandlesArea::topPadding() { return top_padding; }

void VertHandlesArea::setTopPadding(int new_padding)
{
	if(top_padding != new_padding) {
		top_padding = new_padding;
	}
}

int VertHandlesArea::bottomPadding() { return bottom_padding; }

void VertHandlesArea::setLargestChildHeight(int height)
{
	if(largest_child_height != height) {
		largest_child_height = height;
		resizeMask(this->size());
	}
}

int VertHandlesArea::largestChildHeight() { return largest_child_height; }

void VertHandlesArea::setBottomPadding(int new_padding)
{
	if(bottom_padding != new_padding) {
		bottom_padding = new_padding;
	}
}

void VertHandlesArea::resizeMask(QSize size)
{
	QRegion activeRegion(0, largest_child_height / 2, size.width(), size.height() - largest_child_height);
	setMask(activeRegion);
}

/*
 * Class HorizHandlesArea implementation
 */

HorizHandlesArea::HorizHandlesArea(QWidget *parent)
	: HandlesArea(parent)
{
	largest_child_width = 0;
	left_padding = 50;
	right_padding = 50;
}

int HorizHandlesArea::leftPadding() { return left_padding; }

void HorizHandlesArea::setLeftPadding(int new_padding)
{
	if(left_padding != new_padding) {
		left_padding = new_padding;
	}
}

int HorizHandlesArea::rightPadding() { return right_padding; }

void HorizHandlesArea::setRightPadding(int new_padding)
{
	if(right_padding != new_padding) {
		right_padding = new_padding;
	}
}

void HorizHandlesArea::setLargestChildWidth(int width)
{
	if(largest_child_width != width) {
		largest_child_width = width;
		resizeMask(this->size());
	}
}

int HorizHandlesArea::largestChildWidth() { return largest_child_width; }

void HorizHandlesArea::resizeMask(QSize size)
{
	QRegion activeRegion(largest_child_width / 2, 0, size.width() - largest_child_width, size.height());
	setMask(activeRegion);
}

/*
 * Class GateHandlesArea implementation
 */
GateHandlesArea::GateHandlesArea(QWidget *parent)
	: HorizHandlesArea(parent)
{}

void GateHandlesArea::mousePressEvent(QMouseEvent *event)
{
	PlotLineHandle *child = static_cast<PlotLineHandle *>(childAt(event->pos()));
	if(child) {
		child->raise();
		selectedItem = child;

		PlotGateHandle *handle = static_cast<PlotGateHandle *>(child);

		if(handle->reachedLimit())
			/*if the handle reached the limit update the hotspot so we can still change the bar position */
			hotspot = event->pos() - QPoint(handle->getCurrentPos(), handle->y());
		else {
			hotspot = event->pos() - child->pos();
		}
		child->setGrabbed(true);
	}
}

#include "moc_handles_area.cpp"
