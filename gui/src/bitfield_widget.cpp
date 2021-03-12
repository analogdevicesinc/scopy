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

#include "math.h"

#include "ui_bitfield_widget.h"

#include <scopy/gui/bitfield_widget.hpp>

using namespace scopy::gui;

BitfieldWidget::BitfieldWidget(QWidget* parent, QDomElement* bitfield)
	: QWidget(parent)
	, m_ui(new Ui::BitfieldWidget())
	, m_bitfield(bitfield)

{
	m_ui->setupUi(this);

	/*get bitfield information from the element*/
	m_name = bitfield->firstChildElement("Name").text();
	m_width = bitfield->firstChildElement("Width").text().toInt();
	m_access = bitfield->firstChildElement("Access").text();
	m_description = bitfield->firstChildElement("Description").text();
	m_notes = bitfield->firstChildElement("Notes").text();
	m_regOffset = bitfield->firstChildElement("RegOffset").text().toInt();
	m_sliceWidth = bitfield->firstChildElement("SliceWidth").text().toInt();
	m_defaultValue = bitfield->firstChildElement("DefaultValue").text().toInt();

	m_options = bitfield->firstChildElement("Options");

	createWidget(); // build the widget
}

BitfieldWidget::BitfieldWidget(QWidget* parent, int bitNumber)
	: QWidget(parent)
	, m_ui(new Ui::BitfieldWidget())
{
	m_ui->setupUi(this);

	m_ui->bitLabel->setText(QString(" Bit %1 ").arg(bitNumber, 0, 10));
	m_ui->descriptionLabel->hide();
	m_ui->stackedWidget->setCurrentIndex(1);

	m_sliceWidth = 1;
	m_regOffset = bitNumber;
	m_width = 1;
	m_defaultValue = 0;

	m_ui->valueSpinBox->setEnabled(false);
	m_ui->valueSpinBox->setMaximum(1);
	connect(m_ui->valueSpinBox, SIGNAL(valueChanged(int)), this,
		SLOT(setValue(int))); // connect spinBox singnal to the value changed signal
}

BitfieldWidget::~BitfieldWidget() { delete m_ui; }

void BitfieldWidget::createWidget()
{
	QLabel* label;

	m_ui->descriptionLabel->setText(m_name);
	m_ui->bitLabel->setText(QString("Bit %1 ").arg(m_regOffset));

	for (int i = 1; i < m_width; i++) {
		label = new QLabel(this);
		label->setText(QString("Bit %1 ").arg(m_regOffset + i));
		m_ui->bitHorizontalLayout->insertWidget(1, label);
	}

	/*Set comboBox or spinBox*/
	if (m_options.isNull()) {
		/*set spinBox*/
		m_ui->stackedWidget->setCurrentIndex(1);
		int temp = (int)pow(2, m_width) - 1;
		m_ui->valueSpinBox->setMaximum(temp);
		connect(m_ui->valueSpinBox, SIGNAL(valueChanged(int)), this,
			SLOT(setValue(int))); // connect spinBox singnal to the value changed signal
	} else {
		/*set comboBox*/
		m_ui->stackedWidget->setCurrentIndex(0);

		QDomElement temp = m_options.firstChildElement("Option");

		while (m_options.lastChildElement() != temp) {
			m_ui->valueComboBox->addItem(temp.firstChildElement("Description").text());
			temp = temp.nextSiblingElement();
		}

		m_ui->valueComboBox->addItem(temp.firstChildElement("Description").text());

		connect(m_ui->valueComboBox, SIGNAL(currentIndexChanged(int)), this,
			SLOT(setValue(int))); // connect comboBox signal to the value changed signal
	}
}

void BitfieldWidget::updateValue(uint32_t& value)
{
	int temp = value & ((uint32_t)pow(2, m_width) - 1);

	if (m_ui->stackedWidget->currentIndex() == 1) {
		m_ui->valueSpinBox->setValue(temp);
	} else {
		m_ui->valueComboBox->setCurrentIndex(value & ((uint32_t)pow(2, m_width) - 1));
	}

	value = (value >> m_width);
}

int BitfieldWidget::getRegOffset() const { return m_regOffset; }

int BitfieldWidget::getSliceWidth() const { return m_sliceWidth; }

void BitfieldWidget::setValue(int value)
{
	this->m_value = value << m_regOffset;
	uint32_t mask = (uint32_t)(pow(2, m_width) - 1) << m_regOffset;

	Q_EMIT valueChanged(this->m_value, mask);
}

uint32_t BitfieldWidget::getDefaultValue(void) const { return m_defaultValue; }
