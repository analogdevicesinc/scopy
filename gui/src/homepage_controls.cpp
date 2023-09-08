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

using namespace scopy;

HomepageControls::HomepageControls(QWidget *parent) :
	HoverWidget(),
	ui(new Ui::HomepageControls)
{
	QWidget *controls = new QWidget(this);
	ui->setupUi(controls);
	ui->openBtn->hide();

	setContent(controls);
	setAnchor(parent);
	setParent(parent);
	setAnchorPos(HoverPosition::HP_TOPRIGHT);
	setContentPos(HoverPosition::HP_BOTTOMLEFT);
	setAnchorOffset(QPoint(-6, 6));
	show();

	connectSignals();
}

HomepageControls::~HomepageControls()
{
	delete ui;
}

void HomepageControls::connectSignals()
{
	connect(ui->leftBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT goLeft();
	});
	connect(ui->rightBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT goRight();
	});
	connect(ui->openBtn, &QPushButton::clicked, this, [=](){
		Q_EMIT openFile();
	});
}

void HomepageControls::enableLeft(bool en)
{
	ui->leftBtn->setEnabled(en);
}

void HomepageControls::enableRight(bool en)
{
	ui->rightBtn->setEnabled(en);
}

#include "moc_homepage_controls.cpp"
#include <widgets/hoverwidget.h>
