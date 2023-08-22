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
#ifndef MEASUREMENT_GUI_H
#define MEASUREMENT_GUI_H

#include "plot_utils.hpp"
#include <QString>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QColor>
#include <cmath>
#include <stylehelper.h>

namespace scopy {

class MeasurementData;

class SCOPY_GUI_EXPORT MeasurementLabel : public QWidget {
	friend class scopy::StyleHelper;
	Q_OBJECT;
public:
	MeasurementLabel(QWidget *parent = nullptr);
	~MeasurementLabel();


	void setName(QString str);
	void setUnit(QString str);
	void setPrecision(int val);
	void setColor(QColor color);
	void setValue(double val);
	void setMeasurementValueFormatter(PrefixFormatter* f);

	QString name() const;

private:
	QColor m_color;
	QString m_name;
	QString m_unit;
	int m_precision;
	QLabel *m_nameLabel;
	QLabel *m_valueLabel;
	PrefixFormatter * m_formatter;
};

} // namespace scopy

#endif // MEASUREMENT_GUI_H
