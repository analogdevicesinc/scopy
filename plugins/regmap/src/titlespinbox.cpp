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

#include "titlespinbox.hpp"
#include "regmapstylehelper.hpp"
#include "regmapstylehelper.hpp"

#include <QBoxLayout>
#include <utils.h>

using namespace scopy;
using namespace regmap;

TitleSpinBox::TitleSpinBox(QString title, QWidget *parent)
	: QWidget(parent)
{

	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(4);
	mainLayout->setMargin(0);
	setLayout(mainLayout);

	QWidget *spinboxWidget = new QWidget(this);
	QVBoxLayout *spinboxWidgetLayout = new QVBoxLayout(spinboxWidget);
	spinboxWidgetLayout->setSpacing(0);
	spinboxWidgetLayout->setMargin(0);

	QWidget *buttonWidget = new QWidget(this);
	QVBoxLayout *buttonWidgetLayout = new QVBoxLayout(buttonWidget);
	buttonWidgetLayout->setSpacing(0);
	buttonWidgetLayout->setMargin(0);

	spinBoxUpButton = new QPushButton("+", buttonWidget);
	RegmapStyleHelper::smallBlueButton(spinBoxUpButton);

	spinBoxDownButton = new QPushButton("-", buttonWidget);
	RegmapStyleHelper::smallBlueButton(spinBoxDownButton);

	buttonWidgetLayout->addWidget(spinBoxUpButton);
	buttonWidgetLayout->addWidget(spinBoxDownButton);

	titleLabel = new QLabel(title);

	spinBox = new QSpinBox(spinboxWidget);
	spinBox->setButtonSymbols(spinBox->ButtonSymbols::NoButtons);
	connect(spinBoxUpButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() + 1); });
	connect(spinBoxDownButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() - 1); });

	spinboxWidgetLayout->addWidget(titleLabel);
	spinboxWidgetLayout->addWidget(spinBox);

	mainLayout->addWidget(buttonWidget);
	mainLayout->addWidget(spinboxWidget);

	RegmapStyleHelper::titleSpinBoxStyle(this);
}

TitleSpinBox::~TitleSpinBox() {}

void TitleSpinBox::setTitle(QString title) { titleLabel->setText(title); }

QPushButton *TitleSpinBox::getSpinBoxUpButton() { return spinBoxUpButton; }

QPushButton *TitleSpinBox::getSpinBoxDownButton() { return spinBoxDownButton; }

QSpinBox *TitleSpinBox::getSpinBox() { return spinBox; }
