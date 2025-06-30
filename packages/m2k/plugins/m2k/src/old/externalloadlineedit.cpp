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

#include <externalloadlineedit.h>

using namespace scopy::m2k;
constexpr double ExternalLoadLineEdit::MAX_EXTERNAL_LOAD;
constexpr double ExternalLoadLineEdit::MIN_EXTERNAL_LOAD;
constexpr double ExternalLoadLineEdit::OUTPUT_AWG_RESISTANCE;

ExternalLoadLineEdit::ExternalLoadLineEdit(QWidget *parent)
	: QLineEdit(parent)
{
	value = MAX_EXTERNAL_LOAD;
	connect(this, SIGNAL(editingFinished()), this, SLOT(setValue()));
}

ExternalLoadLineEdit::~ExternalLoadLineEdit(){

};

double ExternalLoadLineEdit::getValue() { return value; }

void ExternalLoadLineEdit::setValue() { setValue(text().toDouble()); }

void ExternalLoadLineEdit::setValue(QString val) { setValue(val.toDouble()); }
void ExternalLoadLineEdit::setValue(double val)
{
	QString textVal;
	val = std::max(val, MIN_EXTERNAL_LOAD);
	if(val >= MAX_EXTERNAL_LOAD) {
		textVal = "inf";
		val = MAX_EXTERNAL_LOAD;
	} else {
		textVal = QString::number(val);
	}
	setText(textVal);
	value = val;
	Q_EMIT valueChanged(value);
}
