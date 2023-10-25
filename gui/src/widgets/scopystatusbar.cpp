/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "scopystatusbar.h"
#include "stylehelper.h"
#include <pluginbase/statusbarmanager.h>
#include <QLoggingCategory>
#include <QApplication>
#include <QTimer>
#include <QSizeGrip>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCOPYSTATUSBAR, "ScopyStatusBar")

ScopyStatusBar::ScopyStatusBar(QWidget *parent)
	: MenuVAnim(parent)
{
	initUi();

	auto statusManager = StatusBarManager::GetInstance();
	connect(statusManager, &StatusBarManager::sendStatus, this, &ScopyStatusBar::displayStatusMessage);
	connect(statusManager, &StatusBarManager::clearDisplay, this, &ScopyStatusBar::clearStatusMessage);
}

void ScopyStatusBar::initUi()
{
	// general layout
	auto mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setAnimMin(0);
	setAnimMax(20);
	setMinimumHeight(20);
	setMaximumHeight(20);

	m_leftWidget = new QWidget(this);
	auto leftLayout = new QHBoxLayout(m_leftWidget);
	leftLayout->setContentsMargins(0, 0, 0, 0);
	m_leftWidget->setLayout(leftLayout);

	m_rightWidget = new QWidget(this);
	auto rightLayout = new QHBoxLayout(m_rightWidget);
	rightLayout->setContentsMargins(0, 3, 0, 3);
	m_rightWidget->setLayout(rightLayout);

	layout()->addWidget(m_leftWidget);
	layout()->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout()->addWidget(m_rightWidget);

	// right layout
	StyleHelper::ScopyStatusBar(this, "ScopyStatusBar");

	hide();
}

void ScopyStatusBar::addToRight(QWidget *widget) { m_rightWidget->layout()->addWidget(widget); }

void ScopyStatusBar::addToLeft(QWidget *widget) { m_leftWidget->layout()->addWidget(widget); }

void ScopyStatusBar::displayStatusMessage(StatusMessage *statusMessage)
{
	m_message = statusMessage;
	addToLeft(statusMessage->getWidget());
	this->toggleMenu(true);
	this->show();
}

void ScopyStatusBar::clearStatusMessage()
{
	this->toggleMenu(false);
	delete m_message;
	m_message = nullptr;
}

#include "moc_scopystatusbar.cpp"
