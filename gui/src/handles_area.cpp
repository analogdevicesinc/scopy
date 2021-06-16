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

#include <QDebug>
#include <QtWidgets>

#include <scopy/gui/handles_area.hpp>
#include <scopy/gui/handles_area_extension.hpp>
#include <scopy/gui/plot_line_handle.hpp>

/*
 * Class HandlesArea implementation
 */

HandlesArea::HandlesArea(QWidget* parent)
	: QWidget(parent)
	, m_canvas(parent)
	, m_selectedItem(NULL)
{
	this->setStyleSheet("QWidget {background-color: transparent}");
}

void HandlesArea::installExtension(std::unique_ptr<HandlesAreaExtension> extension)
{
	m_extensions.push_back(std::move(extension));
}

void HandlesArea::mousePressEvent(QMouseEvent* event)
{
	PlotLineHandle* child = static_cast<PlotLineHandle*>(childAt(event->pos()));

	if (child) {
		child->raise();
		m_selectedItem = child;
		m_hotspot = event->pos() - child->pos();
		child->setGrabbed(true);
	}
}

void HandlesArea::mouseReleaseEvent(QMouseEvent*)
{
	if (m_selectedItem) {
		m_selectedItem->setGrabbed(false);
		m_selectedItem = NULL;
	}
}

void HandlesArea::mouseMoveEvent(QMouseEvent* event)
{
	if (m_selectedItem) {
		int x = event->pos().x() - m_hotspot.x();
		int y = event->pos().y() - m_hotspot.y();

		m_selectedItem->moveWithinParent(x, y);
	}
}

void HandlesArea::resizeEvent(QResizeEvent* event)
{
	resizeMask(event->size());

	Q_EMIT sizeChanged(event->size());
}

void HandlesArea::paintEvent(QPaintEvent* event)
{
	QPainter p(this);

	for (std::unique_ptr<HandlesAreaExtension>& extension : m_extensions) {
		if (extension->draw(&p, this)) {
			break;
		}
	}

	QWidget::paintEvent(event);
}

/*
 * Class VertHandlesArea implementation
 */

VertHandlesArea::VertHandlesArea(QWidget* parent)
	: HandlesArea(parent)
{
	m_largestChildHeight = 0;
	m_topPadding = 50;
	m_bottomPadding = 50;
}

int VertHandlesArea::topPadding() { return m_topPadding; }

void VertHandlesArea::setTopPadding(int new_padding)
{
	if (m_topPadding != new_padding) {
		m_topPadding = new_padding;
	}
}

int VertHandlesArea::bottomPadding() { return m_bottomPadding; }

void VertHandlesArea::setLargestChildHeight(int height)
{
	if (m_largestChildHeight != height) {
		m_largestChildHeight = height;
		resizeMask(this->size());
	}
}

int VertHandlesArea::largestChildHeight() { return m_largestChildHeight; }

void VertHandlesArea::setBottomPadding(int new_padding)
{
	if (m_bottomPadding != new_padding) {
		m_bottomPadding = new_padding;
	}
}

void VertHandlesArea::resizeMask(QSize size)
{
	QRegion activeRegion(0, m_largestChildHeight / 2, size.width(), size.height() - m_largestChildHeight);
	setMask(activeRegion);
}

/*
 * Class HorizHandlesArea implementation
 */

HorizHandlesArea::HorizHandlesArea(QWidget* parent)
	: HandlesArea(parent)
{
	m_largestChildWidth = 0;
	m_leftPadding = 50;
	m_rightPadding = 50;
}

int HorizHandlesArea::leftPadding() { return m_leftPadding; }

void HorizHandlesArea::setLeftPadding(int new_padding)
{
	if (m_leftPadding != new_padding) {
		m_leftPadding = new_padding;
	}
}

int HorizHandlesArea::rightPadding() { return m_rightPadding; }

void HorizHandlesArea::setRightPadding(int new_padding)
{
	if (m_rightPadding != new_padding) {
		m_rightPadding = new_padding;
	}
}

void HorizHandlesArea::setLargestChildWidth(int width)
{
	if (m_largestChildWidth != width) {
		m_largestChildWidth = width;
		resizeMask(this->size());
	}
}

int HorizHandlesArea::largestChildWidth() { return m_largestChildWidth; }

void HorizHandlesArea::resizeMask(QSize size)
{
	QRegion activeRegion(m_largestChildWidth / 2, 0, size.width() - m_largestChildWidth, size.height());
	setMask(activeRegion);
}

/*
 * Class GateHandlesArea implementation
 */
GateHandlesArea::GateHandlesArea(QWidget* parent)
	: HorizHandlesArea(parent)
{}

void GateHandlesArea::mousePressEvent(QMouseEvent* event)
{
	PlotLineHandle* child = static_cast<PlotLineHandle*>(childAt(event->pos()));
	if (child) {
		child->raise();
		m_selectedItem = child;

		PlotGateHandle* handle = static_cast<PlotGateHandle*>(child);

		if (handle->reachedLimit())
			/*if the handle reached the limit update the hotspot so we can still change the bar position */
			m_hotspot = event->pos() - QPoint(handle->getCurrentPos(), handle->y());
		else {
			m_hotspot = event->pos() - child->pos();
		}
		child->setGrabbed(true);
	}
}
