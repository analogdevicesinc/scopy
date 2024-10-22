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

#include "bitfieldsimplewidget.hpp"

#include "dynamicWidget.h"

#include <QBoxLayout>
#include <QStyle>
#include <QVariant>
#include <qlabel.h>

#include <pluginbase/preferences.h>
#include <regmapstylehelper.hpp>
#include <src/utils.hpp>
#include <utils.h>

using namespace scopy;
using namespace regmap;

BitFieldSimpleWidget::BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width,
					   QString notes, int regOffset, int streach, QWidget *parent)
	: width(width)
	, name(name)
	, description(description)
	, streach(streach)
	, QFrame{parent}
{
	mainFrame = new QFrame;

	setMinimumWidth(60);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	layout = new QHBoxLayout();

	QVBoxLayout *rightLayout = new QVBoxLayout();
	rightLayout->setAlignment(Qt::AlignRight);

	value = new QLabel("N/R");
	value->setAlignment(Qt::AlignRight);
	value->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	value->setMinimumWidth(25);
	QLabel *bitfieldWidth = new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset));
	bitfieldWidth->setAlignment(Qt::AlignRight);
	rightLayout->addWidget(bitfieldWidth);
	rightLayout->addWidget(value);

	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->setAlignment(Qt::AlignTop);
	QLabel *descriptionLabel = new QLabel(name);
	descriptionLabel->setWordWrap(true);
	descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	leftLayout->addWidget(descriptionLabel);

	layout->addLayout(leftLayout);
	layout->addLayout(rightLayout);

	QString toolTip = "Name : " + name + "\n" + QString::number(regOffset + width - 1) + ":" +
		QString::number(regOffset) + "\n" + "Description : " + description + "\n" + "Notes : " + notes + "\n" +
		"Default Value : " + Utils::convertToHexa(defaultValue, width);

	setToolTip(toolTip);

	mainFrame->setLayout(layout);
	QVBoxLayout *mainFrameLayout = new QVBoxLayout();
	mainFrameLayout->setMargin(0);
	mainFrameLayout->setSpacing(0);
	mainFrameLayout->addWidget(mainFrame);
	setLayout(mainFrameLayout);

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=](QString id, QVariant var) {
		if(id.contains("regmap")) {
			applyStyle();
		}
	});
}

BitFieldSimpleWidget::~BitFieldSimpleWidget()
{
	delete value;
	delete layout;
}

void BitFieldSimpleWidget::updateValue(QString newValue)
{
	value->setText(newValue);
	applyStyle();
}

int BitFieldSimpleWidget::getWidth() const { return width; }

QString BitFieldSimpleWidget::getDescription() const { return description; }

int BitFieldSimpleWidget::getStreach() const { return streach; }

void BitFieldSimpleWidget::applyStyle() { RegmapStyleHelper::BitFieldSimpleWidgetStyle(this); }

void BitFieldSimpleWidget::setSelected(bool selected) { scopy::setDynamicProperty(mainFrame, "is_selected", selected); }

QString BitFieldSimpleWidget::getName() const { return name; }
