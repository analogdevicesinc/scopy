/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "interactivelabel.h"
#include <qevent.h>
#include <style.h>

using namespace scopy;

InteractiveLabel::InteractiveLabel(QString text, QWidget *parent)
	: QLabel(text, parent)
{
	Style::setStyle(this, style::properties::label::interactive);
}

InteractiveLabel::~InteractiveLabel() {}

void InteractiveLabel::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton) {
		Q_EMIT clickEvent(text());
	}
}

#include "moc_interactivelabel.cpp"
