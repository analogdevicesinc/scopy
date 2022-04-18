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
#include "ComboBoxLineEdit.h"

using namespace adiscope;

ComboBoxLineEdit::ComboBoxLineEdit(QWidget *parent):
	QLineEdit(parent)
{
}

void ComboBoxLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
	QWidget::mouseReleaseEvent(event);
}

void ComboBoxLineEdit::mousePressEvent(QMouseEvent *event)
{
	QWidget::mousePressEvent(event);
}

void ComboBoxLineEdit::enterEvent(QEnterEvent *event)
{
	QWidget::enterEvent(event);
}

void ComboBoxLineEdit::leaveEvent(QEvent *event)
{
	QWidget::leaveEvent(event);
}
