/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
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

	connect(ui->leftBtn, &QPushButton::clicked, [=](){
		Q_EMIT goLeft();
	});
	connect(ui->rightBtn, &QPushButton::clicked, [=](){
		Q_EMIT goRight();
	});
	connect(ui->openBtn, &QPushButton::clicked, [=](){
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
	move(parentWidget()->width() - 150, geometry().topLeft().y());
}

bool HomepageControls::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updatePosition();
		return false;
	}

	return QObject::eventFilter(watched, event);
}
