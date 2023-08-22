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
#include "widgets/measurementlabel.h"
#include <QLabel>

using namespace scopy;

/*
 * Class MeasurementLabel implementation
 */

MeasurementLabel::MeasurementLabel(QWidget *parent) : QWidget(parent) {
	QHBoxLayout *lay = new QHBoxLayout(this);
	m_unit = "";
	setLayout(lay);
	lay->setStretch(0,3); // 75 %
	lay->setStretch(1,1); // 25 %
	m_nameLabel = new QLabel(this);
	setName("None");
	m_valueLabel = new QLabel("---", this);
	m_formatter = nullptr;

	lay->addWidget(m_nameLabel);
	lay->addWidget(m_valueLabel);

}

MeasurementLabel::~MeasurementLabel() {

}

void MeasurementLabel::setName(QString str) {
	m_name = str;
	m_nameLabel->setText(str+" : ");
}

void MeasurementLabel::setUnit(QString str) {
	m_unit = str;
}

void MeasurementLabel::setPrecision(int val) {
	m_precision = val;
}

void MeasurementLabel::setColor(QColor color) {
	m_color = color;
	StyleHelper::MeasurementPanelLabel(this, m_name + "MeasurementLabel");
}

void MeasurementLabel::setValue(double val) {
	if(!m_formatter) {
		m_valueLabel->setText( QString::number(val,'g', m_precision) + " " + m_unit);
	} else {
		m_valueLabel->setText( m_formatter->format(val, m_unit, m_precision));
	}
}

void MeasurementLabel::setMeasurementValueFormatter(PrefixFormatter *f) {
	m_formatter = f;
}

QString MeasurementLabel::name() const
{
	return m_name;
}
