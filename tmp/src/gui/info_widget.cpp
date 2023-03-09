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
#include "info_widget.h"
#include "ui_info_widget.h"
#include <QTimer>

using namespace adiscope;

InfoWidget::InfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InfoWidget)
{
	ui->setupUi(this);
	timer =  new QTimer(this);
	connect(timer, SIGNAL(timeout()), this,
		SLOT(updatePosition()));
	this->hide();
}

InfoWidget::~InfoWidget()
{
	delete ui;
}

void InfoWidget::updatePosition()
{
	QPoint p = this->parentWidget()->mapFromGlobal(QCursor::pos());
	//Position the widget in the bottom right corner of the pixmap
	//that is dragged
	this->move(p.x() + 159, p.y() + 54);
}

void InfoWidget::setText(QString text)
{
	ui->label->setText(text);
}

void InfoWidget::enable(bool on)
{
	if (on){
		timer->start(1);
		this->show();
	} else {
		timer->stop();
		this->hide();
	}
}


