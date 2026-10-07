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
 */

#include "valuebarwidget.h"

using namespace scopy;

ValueBarWidget::ValueBarWidget(QWidget *parent)
	: QwtThermo(parent)
{
	setOrientation(Qt::Horizontal);
	setScalePosition(QwtThermo::LeadingScale);
	// OriginCustom with Qwt's default origin of 0.0: the bar grows out of zero, so a
	// negative range fills leftward from the right end of its scale without needing
	// an inverted appearance.
	setOriginMode(QwtThermo::OriginCustom);
	setBorderWidth(0);
}

void ValueBarWidget::setRange(double lower, double upper) { setScale(lower, upper); }

void ValueBarWidget::setTickCounts(int major, int minor)
{
	setScaleMaxMajor(major);
	setScaleMaxMinor(minor);
}

void ValueBarWidget::setBarColor(const QColor &color)
{
	m_barColor = color;
	// global.qss styles every QWidget's `color` and a stylesheet beats a hand-set
	// palette, so the colour cannot go through QPalette even though Qwt fills the
	// bar from palette ButtonText.
	setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));
}

QColor ValueBarWidget::barColor() const { return m_barColor; }

#include "moc_valuebarwidget.cpp"
