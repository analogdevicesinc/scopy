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

#include "movingaverage.h"

MovingAverage::MovingAverage() { sum = 0; }

int MovingAverage::getCapacity() const { return capacity; }

void MovingAverage::setCapacity(int newCapacity) { capacity = newCapacity; }

const QVector<double> &MovingAverage::getHistory() const { return history; }

double MovingAverage::pushValueReturnAverage(double newVal)
{
	sum += newVal;
	if(history.count() == capacity && capacity > 0) {
		sum -= history.front();
		history.pop_front();
	}
	history.push_back(newVal);
	return (sum / (double)history.count());
}

void MovingAverage::clearHistory()
{
	history.clear();
	sum = 0;
}
