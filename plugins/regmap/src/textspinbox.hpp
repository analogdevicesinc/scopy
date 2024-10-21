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

#ifndef TEXTSPINBOX_H
#define TEXTSPINBOX_H

#include "scopy-regmap_export.h"

#include <QObject>
#include <QSpinBox>
#include <QWidget>

class SCOPY_REGMAP_EXPORT TextSpinBox : public QSpinBox
{
public:
	explicit TextSpinBox(QWidget *parent = 0);

	void setValues(QList<QString> *acceptedValues);

private:
	QString textFromValue(int value) const;
	int valueFromText(const QString &text) const;
	QValidator::State validate(QString &text, int &pos) const;
	QList<QString> *acceptedValues;
};

#endif // TEXTSPINBOX_H
