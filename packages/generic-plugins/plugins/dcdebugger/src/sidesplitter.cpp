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

#include "sidesplitter.h"

#include <gui/mapstackedwidget.h>

#include <algorithm>

using namespace scopy;
using namespace scopy::dcdebugger;

SideSplitter::SideSplitter(int leftWidth, int rightWidth, QWidget *parent)
	: QSplitter(Qt::Horizontal, parent)
	, m_leftWidth(leftWidth)
	, m_rightWidth(rightWidth)
{
	setChildrenCollapsible(true);
	if(leftWidth > 0) {
		m_left = new MapStackedWidget(this);
		addWidget(m_left);
	}
	m_right = new MapStackedWidget(this);
	addWidget(m_right);

	connect(this, &QSplitter::splitterMoved, this, &SideSplitter::onMoved);
	connect(m_right, &QStackedWidget::currentChanged, this, [this]() { setRightOpen(true); });
}

void SideSplitter::setCentralWidget(QWidget *central)
{
	m_central = central;
	insertWidget(indexOf(m_right), central);
	// Extra width goes to the central widget, which never collapses.
	QList<int> s;
	for(int i = 0; i < count(); ++i) {
		QWidget *w = widget(i);
		setStretchFactor(i, w == central ? 1 : 0);
		s.append(w == m_left ? m_leftWidth : w == m_right ? m_rightWidth : central->sizeHint().width());
	}
	setCollapsible(indexOf(central), false);
	setSizes(s);
}

bool SideSplitter::isRightOpen() const { return m_rightOpen; }

void SideSplitter::setLeftOpen(bool open)
{
	if(m_left) {
		setOpen(m_left, m_leftWidth, open);
	}
}

void SideSplitter::setRightOpen(bool open)
{
	setOpen(m_right, m_rightWidth, open);
	onMoved();
}

void SideSplitter::setOpen(QWidget *side, int &width, bool open)
{
	// Before the first layout the sizes are not real yet; the initial ones apply.
	if(!m_central || !isVisible()) {
		return;
	}
	QList<int> s = sizes();
	const int i = indexOf(side);
	const int c = indexOf(m_central);
	if(open == (s[i] > 0)) {
		return;
	}
	if(open) {
		s[i] = width;
		s[c] = std::max(0, s[c] - width);
	} else {
		width = s[i];
		s[c] += s[i];
		s[i] = 0;
	}
	setSizes(s);
}

void SideSplitter::onMoved()
{
	if(!m_central || !isVisible()) {
		return;
	}
	const QList<int> s = sizes();
	// The widths a hidden stack comes back at.
	if(m_left && s[indexOf(m_left)] > 0) {
		m_leftWidth = s[indexOf(m_left)];
	}
	const int right = s[indexOf(m_right)];
	if(right > 0) {
		m_rightWidth = right;
	}
	if((right > 0) != m_rightOpen) {
		m_rightOpen = right > 0;
		Q_EMIT rightOpenChanged(m_rightOpen);
	}
}

#include "moc_sidesplitter.cpp"
