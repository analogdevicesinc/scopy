/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef MEASUREMENTSTABLE_H
#define MEASUREMENTSTABLE_H

#include "signalmath.h"

#include <QColor>
#include <QTableWidget>

namespace scopy::dcdebugger {

struct MeasurementRow
{
	QString channel;
	QColor color;
	Measurements values;
	QString peak; // spectrum only
};

// Channel / Min / Max / Mean / RMS / AC RMS / Pk-Pk / Freq / Peak, one row per plotted channel.
class MeasurementsTable : public QTableWidget
{
	Q_OBJECT
public:
	explicit MeasurementsTable(QWidget *parent = nullptr);
	void setRows(const QList<MeasurementRow> &rows, const QString &unit);
};

// v with an SI prefix and `unit`, e.g. 1.0008 kHz.
QString siFormat(double v, const QString &unit = {}, int digits = 5);

} // namespace scopy::dcdebugger

#endif // MEASUREMENTSTABLE_H
