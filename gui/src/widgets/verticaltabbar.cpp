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

#include <style.h>
#include <QStylePainter>
#include <QStyleOptionTab>
#include <QPainter>
#include <verticaltabbar.h>

using namespace scopy;

VerticalTabBar::VerticalTabBar(QWidget *parent)
	: QTabBar(parent)
{}

void VerticalTabBar::paintEvent(QPaintEvent *event)
{
	QStylePainter painter(this);
	QStyleOptionTab opt;

	QColor tab_text = Style::getColor(json::theme::content_default);
	Style::getColor(json::theme::interactive_primary_idle);
	QColor tab_selected =
		Style::getColor(json::theme::interactive_primary_idle); // Background color of the selected tab
	QColor tab_text_selected = Style::getColor(json::theme::content_inverse); // Text color of the tabs
	int borderRadius = Style::getDimension(json::global::radius_1);		  // Radius for the rounded corners
	int padding = Style::getDimension(json::global::padding_1);		  // Padding for the tab

	for(int i = 0; i < count(); ++i) {
		initStyleOption(&opt, i);

		QRect rect = tabRect(i);
		rect.adjust(padding, padding, -padding, -padding); // Adjust the rectangle to add padding

		if(currentIndex() == i) {
			painter.setBrush(tab_selected);
			painter.setPen(Qt::NoPen);
			painter.drawRoundedRect(rect, borderRadius, borderRadius);
			painter.setPen(tab_text_selected);
		} else {
			painter.setPen(tab_text);
		}

		painter.drawText(rect, Qt::AlignCenter, tabText(i));
	}
}
