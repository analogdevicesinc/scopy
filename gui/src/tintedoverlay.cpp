/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "tintedoverlay.h"

using namespace scopy::gui;
TintedOverlay::TintedOverlay(QWidget *parent, QColor color)
	: QWidget(parent)
	, color(color)
{
	setAttribute(Qt::WA_TranslucentBackground);
	this->parent = parent;
	holes.clear();

	QRect geo = parent->rect();
	setGeometry(geo);
	setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

TintedOverlay::~TintedOverlay() {}

const QList<QWidget *> &TintedOverlay::getHoles() const { return holes; }

void TintedOverlay::setHoles(const QList<QWidget *> &newHoles) { holes = newHoles; }

void TintedOverlay::clearHoles() { holes.clear(); }

void TintedOverlay::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	if(!holes.isEmpty()) {
		QPainterPath p1, p2;
		p1.addRect(rect());
		for(QWidget *holeWidget : qAsConst(holes)) {
			QPoint offset = holeWidget->mapTo(parent, QPoint(0, 0));
			QRect hole = holeWidget->geometry();
			hole.moveTo(offset);

			p2.addRect(hole);
			p1 -= p2;
		}
		QRegion r(p1.toFillPolygon().toPolygon());
		setMask(r);
	} else {
		setMask(rect());
	}
	painter.fillRect(rect(), color);
}

void TintedOverlay::mousePressEvent(QMouseEvent *ev)
{
	// prevent clicks from being sent to parents
	ev->accept();
}

#include "moc_tintedoverlay.cpp"
