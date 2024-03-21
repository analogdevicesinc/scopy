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
	m_statusManager = StatusBarManager::GetInstance();
	connect(m_statusManager, &StatusBarManager::sendStatus, this, &ScopyStatusBar::displayStatusMessage);
	connect(m_statusManager, &StatusBarManager::clearDisplay, this, &ScopyStatusBar::clearStatusMessage);
}

ScopyStatusBar::~ScopyStatusBar()
{
	clearStatusMessage();
	disconnect(m_statusManager, &StatusBarManager::sendStatus, this, &ScopyStatusBar::displayStatusMessage);
	disconnect(m_statusManager, &StatusBarManager::clearDisplay, this, &ScopyStatusBar::clearStatusMessage);
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

	m_stackedWidget = new QStackedWidget(this);

	m_rightWidget = new QWidget(this);
	auto rightLayout = new QHBoxLayout(m_rightWidget);
	rightLayout->setContentsMargins(0, 3, 0, 3);
	m_rightWidget->setLayout(rightLayout);

	layout()->addWidget(m_stackedWidget);
	layout()->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout()->addWidget(m_rightWidget);

	// right layout
	StyleHelper::ScopyStatusBar(this, "ScopyStatusBar");

	hide();
}

void ScopyStatusBar::addToRight(QWidget *widget) { m_rightWidget->layout()->addWidget(widget); }

void ScopyStatusBar::addToLeft(QWidget *widget)
{
	m_stackedWidget->addWidget(widget);
	m_stackedWidget->setCurrentWidget(widget);
}

void ScopyStatusBar::displayStatusMessage(StatusMessage *statusMessage)
{
	m_messages.append(statusMessage);
	addToLeft(statusMessage->getWidget());
	this->toggleMenu(true);
	this->show();
}

void ScopyStatusBar::clearStatusMessage()
{
	this->toggleMenu(false);
	m_stackedWidget->removeWidget(m_stackedWidget->currentWidget());
	removeLastStatusMessage();
	if(m_stackedWidget->count() > 0) {
		this->toggleMenu(true);
		this->show();
	}
}

void ScopyStatusBar::removeLastStatusMessage()
{
	StatusMessage *statusMessage = nullptr;
	if(!m_messages.isEmpty()) {
		statusMessage = m_messages.takeLast();
		delete statusMessage;
		statusMessage = nullptr;
	}
}

#include "moc_scopystatusbar.cpp"
