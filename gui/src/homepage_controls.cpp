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

#include "homepage_controls.h"
#include "ui_homepage_controls.h"
#include <QDebug>

using namespace adiscope;

HomepageControls::HomepageControls(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HomepageControls)
{
	ui->setupUi(this);

	connect(ui->leftBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT goLeft();
	});
	connect(ui->rightBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT goRight();
	});
	connect(ui->openBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT openFile();
	});

	ui->openBtn->hide();

	updatePosition();
}

HomepageControls::~HomepageControls()
{
	delete ui;
}

void HomepageControls::updatePosition()
{
	move(parentWidget()->width() - 120, 5);
}

bool HomepageControls::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updatePosition();
		return false;
	}

	return QObject::eventFilter(watched, event);
}

void HomepageControls::enableLeft(bool en)
{
	ui->leftBtn->setEnabled(en);
}

void HomepageControls::enableRight(bool en)
{
	ui->rightBtn->setEnabled(en);
}
