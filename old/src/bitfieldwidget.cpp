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

#include "bitfieldwidget.h"
#include "ui_bitfieldwidget.h"

#include "math.h"

using namespace adiscope;

BitfieldWidget::BitfieldWidget(QWidget *parent, QDomElement *bitfield) :
	QWidget(parent),
	ui(new Ui::BitfieldWidget()),
	bitfield(bitfield)

{
	ui->setupUi(this);

	/*get bitfield information from the element*/
	name = bitfield->firstChildElement("Name").text();
	width = bitfield->firstChildElement("Width").text().toInt();
	access = bitfield->firstChildElement("Access").text();
	description = bitfield->firstChildElement("Description").text();
	notes = bitfield->firstChildElement("Notes").text();
	regOffset = bitfield->firstChildElement("RegOffset").text().toInt();
	sliceWidth = bitfield->firstChildElement("SliceWidth").text().toInt();
	defaultValue = bitfield->firstChildElement("DefaultValue").text().toInt();

	options = bitfield->firstChildElement("Options");

	createWidget(); //build the widget
}

BitfieldWidget::BitfieldWidget(QWidget *parent, int bitNumber) :
	QWidget(parent),
	ui(new Ui::BitfieldWidget())
{
	ui->setupUi(this);

	ui->bitLabel->setText(QString(" Bit %1 ").arg(bitNumber, 0, 10));
	ui->descriptionLabel->hide();
	ui->stackedWidget->setCurrentIndex(1);

	sliceWidth = 1;
	regOffset = bitNumber;
	width = 1;
	defaultValue = 0;

	ui->valueSpinBox->setEnabled(false);
	ui->valueSpinBox->setMaximum(1);
	connect(ui->valueSpinBox, SIGNAL(valueChanged(int)), this,
	        SLOT(setValue(int))); //connect spinBox singnal to the value changed signal
}

BitfieldWidget::~BitfieldWidget()
{
	delete ui;
}

void BitfieldWidget::createWidget()
{
	QLabel *label;

	ui->descriptionLabel->setText(name);
	ui->bitLabel->setText(QString("Bit %1 ").arg(regOffset));

	for (int i = 1; i < width; i++) {
		label = new QLabel(this);
		label->setText(QString("Bit %1 ").arg(regOffset + i));
		ui->bitHorizontalLayout->insertWidget(1,label);
	}

	/*Set comboBox or spinBox*/
	if (options.isNull()) {
		/*set spinBox*/
		ui->stackedWidget->setCurrentIndex(1);
		int temp = (int)pow(2, width) - 1;
		ui->valueSpinBox->setMaximum(temp);
		connect(ui->valueSpinBox, SIGNAL(valueChanged(int)), this,
		        SLOT(setValue(int))); //connect spinBox singnal to the value changed signal
	} else {
		/*set comboBox*/
		ui->stackedWidget->setCurrentIndex(0);

		QDomElement temp = options.firstChildElement("Option");

		while (options.lastChildElement() != temp) {
			ui->valueComboBox->addItem(temp.firstChildElement("Description").text());
			temp = temp.nextSiblingElement();
		}

		ui->valueComboBox->addItem(temp.firstChildElement("Description").text());

		connect(ui->valueComboBox,SIGNAL(currentIndexChanged(int)), this,
		        SLOT(setValue(int))); //connect comboBox signal to the value changed signal
	}
}

void BitfieldWidget::updateValue(uint32_t& value)
{
	int temp = value & ((uint32_t)pow(2, width)  - 1);

	if (ui->stackedWidget->currentIndex() == 1) {
		ui->valueSpinBox->setValue(temp);
	} else {
		ui->valueComboBox->setCurrentIndex(value & ((uint32_t)pow(2, width) - 1));
	}

	value = (value >> width);
}

int BitfieldWidget::getRegOffset() const
{
	return regOffset;
}

int BitfieldWidget::getSliceWidth() const
{
	return sliceWidth;
}

void BitfieldWidget::setValue(int value)
{
	this->value = value << regOffset;
	uint32_t mask = (uint32_t)(pow(2, width) - 1) << regOffset;

	Q_EMIT valueChanged(this->value, mask);
}

uint32_t BitfieldWidget::getDefaultValue(void) const
{
	return defaultValue;
}
