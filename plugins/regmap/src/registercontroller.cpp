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

#include "registercontroller.hpp"

#include "textspinbox.hpp"
#include "regmapstylehelper.hpp"
#include "utils.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdebug.h>
#include <QPushButton>
#include <QSpinBox>
#include <QAbstractSpinBox>
#include <titlespinbox.hpp>
#include <utils.h>
#include <style.h>

using namespace scopy;
using namespace regmap;

RegisterController::RegisterController(QWidget *parent)
	: QWidget{parent}
{
	mainLayout = new QHBoxLayout(this);
	Utils::removeLayoutMargins(mainLayout);
	setLayout(mainLayout);

	QWidget *readWidget = new QWidget(this);
	readWidget->setFixedHeight(72);
	mainLayout->addWidget(readWidget);
	QHBoxLayout *readWidgetLayout = new QHBoxLayout(readWidget);
	Utils::removeLayoutMargins(readWidgetLayout);
	readWidget->setLayout(readWidgetLayout);
	readWidgetLayout->setMargin(16);

	QVBoxLayout *readWidgetLeftLayout = new QVBoxLayout();
	Utils::removeLayoutMargins(readWidgetLeftLayout);
	adrPck = new TitleSpinBox("Address 1: ", readWidget);
	adrPck->setMaximumHeight(40);
	readWidgetLeftLayout->addWidget(adrPck);

	addressPicker = adrPck->getSpinBox();

	addressPicker->setDisplayIntegerBase(16);
	addressPicker->setMinimum(0);
	addressPicker->setMaximum(INT_MAX);
	addressPicker->setPrefix("0x");
	addressPicker->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QObject::connect(addressPicker, &QSpinBox::textChanged, this, [=](QString address) {
		Q_EMIT registerAddressChanged(Utils::convertQStringToUint32(address));
	});
	readWidgetLayout->addLayout(readWidgetLeftLayout, 3);

	readButton = new QPushButton("Read", readWidget);
	// request read
	QObject::connect(readButton, &QPushButton::clicked, this,
			 [=]() { Q_EMIT requestRead(Utils::convertQStringToUint32(addressPicker->text())); });

	readWidgetLayout->addWidget(readButton, 1, Qt::AlignRight);

	QWidget *writeWidget = new QWidget(this);
	writeWidget->setFixedHeight(72);
	mainLayout->addWidget(writeWidget);
	writeWidgetLayout = new QHBoxLayout(writeWidget);
	Utils::removeLayoutMargins(writeWidgetLayout);
	writeWidget->setLayout(writeWidgetLayout);
	writeWidgetLayout->setMargin(16);

	QVBoxLayout *writeWidgetLeftLayout = new QVBoxLayout();
	Utils::removeLayoutMargins(writeWidgetLeftLayout);
	writeWidgetLeftLayout->setSpacing(0);
	valueLabel = new QLabel("Value: ", writeWidget);
	writeWidgetLeftLayout->addWidget(valueLabel);

	regValue = new QLineEdit(writeWidget);
	regValue->setText("N/R");
	regValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QObject::connect(regValue, &QLineEdit::textChanged, this, &RegisterController::valueChanged);

	writeWidgetLeftLayout->addWidget(regValue);
	writeWidgetLayout->addLayout(writeWidgetLeftLayout, 3);

	writeButton = new QPushButton("Write", writeWidget);
	// request write on register
	QObject::connect(writeButton, &QPushButton::clicked, this, [=]() {
		uint32_t address = addressPicker->value();
		uint32_t value = Utils::convertQStringToUint32(regValue->text());
		Q_EMIT requestWrite(address, value);
	});

	writeWidgetLayout->addWidget(writeButton, 1, Qt::AlignRight);

	applyStyle();
}

RegisterController::~RegisterController()
{

	delete regValue;
	delete addressPicker;
	delete mainLayout;
	if(detailedRegisterToggle) {
		delete detailedRegisterToggle;
	}
}

void RegisterController::registerChanged(uint32_t address)
{
	if(!addressChanged) {
		addressPicker->setValue(address);
	} else {
		addressChanged = false;
	}
}

void RegisterController::registerValueChanged(QString value) { regValue->setText(value); }

QString RegisterController::getAddress() { return addressPicker->text(); }

void RegisterController::setHasMap(bool hasMap)
{
	this->hasMap = hasMap;

	if(hasMap) {
		detailedRegisterToggle = new QPushButton(this);
		detailedRegisterToggle->setCheckable(true);
		QIcon detailedRegisterToggleIcon;
		detailedRegisterToggleIcon.addPixmap(
			Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					 "/icons/tool_calibration.svg"));
		detailedRegisterToggle->setIcon(detailedRegisterToggleIcon);
		detailedRegisterToggle->setChecked(true);
		QObject::connect(detailedRegisterToggle, &QPushButton::toggled, this,
				 &RegisterController::toggleDetailedMenu);
		writeWidgetLayout->addWidget(detailedRegisterToggle, 0.5, Qt::AlignRight);
		RegmapStyleHelper::smallBlueButton(detailedRegisterToggle);
		detailedRegisterToggle->setFixedSize(40, 40);
	}
}

void RegisterController::applyStyle()
{
	setFixedHeight(72);

	RegmapStyleHelper::BlueButton(readButton);
	readButton->setFixedHeight(40);
	RegmapStyleHelper::BlueButton(writeButton);
	writeButton->setFixedHeight(40);
	valueLabel->setStyleSheet(RegmapStyleHelper::grayLabelStyle());

	setStyleSheet(RegmapStyleHelper::regmapControllerStyle(nullptr));
}
