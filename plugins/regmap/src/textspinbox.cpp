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

#include "textspinbox.hpp"

TextSpinBox::TextSpinBox(QWidget *parent)
	: QSpinBox(parent)
{}

void TextSpinBox::setValues(QList<QString> *acceptedValues)
{
	this->acceptedValues = acceptedValues;
	setRange(1, acceptedValues->size());
}

QString TextSpinBox::textFromValue(int value) const { return acceptedValues->value(value); }

int TextSpinBox::valueFromText(const QString &text) const { return acceptedValues->indexOf(text); }

QValidator::State TextSpinBox::validate(QString &text, int &pos) const
{
	if(acceptedValues->contains(text)) {
		return QValidator::Acceptable;
	}

	if(!text.isNull()) {
		return QValidator::Intermediate;
	}

	return QValidator::Invalid;
}
