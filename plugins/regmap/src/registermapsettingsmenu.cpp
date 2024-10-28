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

#include "registermapsettingsmenu.hpp"

#include "regmapstylehelper.hpp"
#include "utils.hpp"

#include <QColor>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <src/readwrite/fileregisterreadstrategy.hpp>

#include <menusectionwidget.h>

using namespace scopy;
using namespace regmap;

RegisterMapSettingsMenu::RegisterMapSettingsMenu(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	setLayout(layout);

	header = new MenuHeaderWidget("Settings", QPen(RegmapStyleHelper::getColor("ScopyBlue")), this);

	layout->addWidget(header);

	MenuSectionWidget *menuSection = new MenuSectionWidget(this);
	layout->addWidget(menuSection);

	autoread = new QCheckBox("Autoread", menuSection);

	QObject::connect(autoread, &QCheckBox::toggled, this, &RegisterMapSettingsMenu::autoreadToggled);
	menuSection->contentLayout()->addWidget(autoread);

	QWidget *setIntervalWidget = new QWidget();
	QVBoxLayout *setIntervalWidgetLayout = new QVBoxLayout(setIntervalWidget);
	setIntervalWidget->setLayout(setIntervalWidgetLayout);

	QHBoxLayout *startReadIntervalLayout = new QHBoxLayout();
	hexaPrefix1 = new QLabel("0x");
	startReadIntervalLayout->addWidget(hexaPrefix1);
	setIntervalWidgetLayout->addLayout(startReadIntervalLayout);

	startReadInterval = new QLineEdit();
	startReadInterval->setPlaceholderText("From register");
	startReadIntervalLayout->addWidget(startReadInterval);
	hexaPrefix2 = new QLabel("0x");
	QHBoxLayout *endReadIntervalLayout = new QHBoxLayout();
	endReadIntervalLayout->addWidget(hexaPrefix2);
	setIntervalWidgetLayout->addLayout(endReadIntervalLayout);

	endReadInterval = new QLineEdit();
	endReadInterval->setPlaceholderText("To register");
	endReadIntervalLayout->addWidget(endReadInterval);

	readInterval = new QPushButton("Read interval");

	readInterval->setEnabled(false);

	QObject::connect(readInterval, &QPushButton::clicked, this, [=]() {
		int startInterval = Utils::convertQStringToUint32(startReadInterval->text());
		int endInterval = Utils::convertQStringToUint32(endReadInterval->text());
		for(int i = startInterval; i <= endInterval; i++) {
			Q_EMIT requestRead(i);
		}
	});

	QObject::connect(startReadInterval, &QLineEdit::textChanged, this, [=]() {
		if(!startReadInterval->text().isEmpty() && !endReadInterval->text().isEmpty()) {
			readInterval->setEnabled(true);
		} else {
			readInterval->setEnabled(false);
		}
	});

	QObject::connect(endReadInterval, &QLineEdit::textChanged, this, [=]() {
		if(!startReadInterval->text().isEmpty() && !endReadInterval->text().isEmpty()) {
			readInterval->setEnabled(true);
		} else {
			readInterval->setEnabled(false);
		}
	});

	menuSection->contentLayout()->addWidget(setIntervalWidget);
	menuSection->contentLayout()->addWidget(readInterval);
	menuSection->contentLayout()->setSpacing(10);

	QWidget *findPathWidget = new QWidget();
	QHBoxLayout *findPathLayout = new QHBoxLayout();
	Utils::removeLayoutMargins(findPathLayout);
	findPathWidget->setLayout(findPathLayout);

	filePath = new QLineEdit();
	filePath->setPlaceholderText("File path");
	pathButton = new QPushButton("Find path");

	QObject::connect(pathButton, &QPushButton::clicked, this,
			 [=]() { filePath->setText(QFileDialog::getOpenFileName(this, ("Open File"), "")); });

	findPathLayout->addWidget(filePath);
	findPathLayout->addWidget(pathButton);
	menuSection->contentLayout()->addWidget(findPathWidget);

	writeListOfValuesButton = new QPushButton("Write values");
	writeListOfValuesButton->setEnabled(false);

	QObject::connect(writeListOfValuesButton, &QPushButton::clicked, this, [=]() {
		FileRegisterReadStrategy *fileRead = new FileRegisterReadStrategy(filePath->text());

		QObject::connect(fileRead, &FileRegisterReadStrategy::readDone, this,
				 &RegisterMapSettingsMenu::requestWrite);

		fileRead->readAll();
	});

	menuSection->contentLayout()->addWidget(writeListOfValuesButton);

	registerDump = new QPushButton("Register dump");
	registerDump->setEnabled(false);

	QObject::connect(registerDump, &QPushButton::clicked, this, [=]() {
		if(autoread->isChecked()) {
			readInterval->click();
		}
		QFile::remove(filePath->text());
		Q_EMIT requestRegisterDump(filePath->text());
	});

	menuSection->contentLayout()->addWidget(registerDump);

	QObject::connect(filePath, &QLineEdit::textChanged, this, [=](QString text) {
		writeListOfValuesButton->setEnabled(!text.isEmpty());
		registerDump->setEnabled(!text.isEmpty());
	});

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	this->layout()->addItem(spacer);

	applyStyle();
}

void RegisterMapSettingsMenu::applyStyle() { RegmapStyleHelper::regmapSettingsMenu(this); }
