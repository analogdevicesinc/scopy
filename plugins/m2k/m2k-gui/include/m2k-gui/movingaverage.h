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

#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include "scopy-m2k-gui_export.h"
#include <QVector>

class SCOPY_M2K_GUI_EXPORT MovingAverage
{
public:
	MovingAverage();
	double pushValueReturnAverage(double newVal);

	int getCapacity() const;
	void setCapacity(int newCapacity);
	void clearHistory();
	const QVector<double> &getHistory() const;

private:
	QVector<double> history;
	int capacity;
	double sum;
};

#endif // MOVINGAVERAGE_H
