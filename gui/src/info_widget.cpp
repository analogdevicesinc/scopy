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
#include "ui_info_widget.h"

#include <QTimer>

#include <scopy/gui/info_widget.hpp>

using namespace scopy::gui;

InfoWidget::InfoWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::InfoWidget)
{
	m_ui->setupUi(this);
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updatePosition()));
	this->hide();
}

InfoWidget::~InfoWidget() { delete m_ui; }

void InfoWidget::updatePosition()
{
	const QPoint p = this->parentWidget()->mapFromGlobal(QCursor::pos());

	// Position the widget in the bottom right corner of the pixmap
	// that is dragged
	this->move(p.x() + 159, p.y() + 54);
}

void InfoWidget::setText(const QString &text) { m_ui->lbl->setText(text); }

void InfoWidget::enable(bool on)
{
	if (on) {
		m_timer->start(1);
		this->show();
	} else {
		m_timer->stop();
		this->hide();
	}
}
