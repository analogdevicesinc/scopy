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

#include "widgets/pagenavigationwidget.h"

#include <QBoxLayout>
#include <QDebug>
#include <QLoggingCategory>

#include <stylehelper.h>

using namespace scopy;
Q_LOGGING_CATEGORY(CAT_NAVIGATIONWIDGET, "PageNavigationWidget")

PageNavigationWidget::PageNavigationWidget(bool hasHome, bool hasOpen, QWidget *parent)
	: QWidget(parent)
{
	initUI(parent);

	setHomeBtnVisible(hasHome);
	setOpenBtnVisible(hasOpen);
}

PageNavigationWidget::~PageNavigationWidget() {}

void PageNavigationWidget::initUI(QWidget *parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *buttonsLayout = new QHBoxLayout();
	setLayout(buttonsLayout);
	buttonsLayout->setSpacing(10);

	homeButton = new QPushButton(this);
	StyleHelper::BlueIconButton(homeButton, QIcon(":/gui/icons/launcher_home.svg"), "homeButton");
	buttonsLayout->addWidget(homeButton);
	homeButton->hide();

	backwardButton = new QPushButton(this);
	StyleHelper::BlueIconButton(backwardButton, QIcon(":/gui/icons/handle_left_arrow.svg"), "backwardButton");
	buttonsLayout->addWidget(backwardButton);

	forwardButton = new QPushButton(this);
	StyleHelper::BlueIconButton(forwardButton, QIcon(":/gui/icons/handle_right_arrow.svg"), "forwardButton");
	buttonsLayout->addWidget(forwardButton);

	openButton = new QPushButton(this);
	StyleHelper::BlueIconButton(openButton, QIcon(":/gui/icons/sba_up_btn_pressed.svg"), "openButton");
	buttonsLayout->addWidget(openButton);
	openButton->hide();

	buttonsLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void PageNavigationWidget::setHomeBtnVisible(bool en)
{
	homeButton->setVisible(en);
	setMinimumSize(sizeHint());
}

void PageNavigationWidget::setOpenBtnVisible(bool en)
{
	openButton->setVisible(en);
	setMinimumSize(sizeHint());
}

QPushButton *PageNavigationWidget::getHomeBtn() { return homeButton; }

QPushButton *PageNavigationWidget::getBackwardBtn() { return backwardButton; }

QPushButton *PageNavigationWidget::getForwardBtn() { return forwardButton; }

QPushButton *PageNavigationWidget::getOpenBtn() { return openButton; }

#include "moc_pagenavigationwidget.cpp"
