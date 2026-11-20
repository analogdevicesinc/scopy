/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "flowlayout.h"

#include <QWidget>

using namespace scopy;

FlowLayout::FlowLayout(QWidget *parent, int hSpacing, int vSpacing)
	: QLayout(parent)
	, m_hSpacing(hSpacing)
	, m_vSpacing(vSpacing)
{
	setContentsMargins(0, 0, 0, 0);
}

FlowLayout::~FlowLayout() { qDeleteAll(m_items); }

void FlowLayout::addItem(QLayoutItem *item) { m_items.append(item); }

int FlowLayout::count() const { return m_items.size(); }

QLayoutItem *FlowLayout::itemAt(int index) const { return m_items.value(index); }

QLayoutItem *FlowLayout::takeAt(int index)
{
	return index >= 0 && index < m_items.size() ? m_items.takeAt(index) : nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const { return {}; }

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const { return doLayout(QRect(0, 0, width, 0), false); }

QSize FlowLayout::minimumSize() const
{
	QSize size;
	for(const QLayoutItem *item : m_items) {
		size = size.expandedTo(item->minimumSize());
	}
	const QMargins m = contentsMargins();
	return size + QSize(m.left() + m.right(), m.top() + m.bottom());
}

QSize FlowLayout::sizeHint() const { return minimumSize(); }

void FlowLayout::setGeometry(const QRect &rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, true);
}

int FlowLayout::doLayout(const QRect &rect, bool apply) const
{
	const QRect area = rect.marginsRemoved(contentsMargins());
	int x = area.x();
	int y = area.y();
	int rowHeight = 0;
	for(QLayoutItem *item : m_items) {
		if(item->widget() && item->widget()->isHidden()) {
			continue;
		}
		const QSize hint = item->sizeHint();
		if(x + hint.width() > area.right() + 1 && rowHeight > 0) {
			x = area.x();
			y += rowHeight + m_vSpacing;
			rowHeight = 0;
		}
		if(apply) {
			item->setGeometry(QRect(QPoint(x, y), hint));
		}
		x += hint.width() + m_hSpacing;
		rowHeight = std::max(rowHeight, hint.height());
	}
	return y + rowHeight - rect.y() + contentsMargins().bottom();
}

#include "moc_flowlayout.cpp"
