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

#ifndef EXTERNALLOADLINEEDIT_H
#define EXTERNALLOADLINEEDIT_H

#include <QLineEdit>

namespace scopy::m2k {
class ExternalLoadLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	static constexpr double MAX_EXTERNAL_LOAD = 100000.0;
	static constexpr double MIN_EXTERNAL_LOAD = 50.0;
	static constexpr double OUTPUT_AWG_RESISTANCE = 50.0;

	ExternalLoadLineEdit(QWidget *parent = nullptr);
	~ExternalLoadLineEdit();

	Q_PROPERTY(double value READ getValue WRITE setValue);

	double getValue();
public Q_SLOTS:
	void setValue();
	void setValue(double val);
	void setValue(QString val);

Q_SIGNALS:
	void valueChanged(double);

protected:
	double value;
};
} // namespace scopy::m2k
#endif // EXTERNALLOADLINEEDIT
