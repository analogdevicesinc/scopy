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

#include "autoScaler.hpp"

#include <qwt_scale_engine.h>

#include <cstdio>

using namespace adiscope;

AutoScaler::AutoScaler(QObject *parent,
		const QVector<QwtScaleDiv> &divs, unsigned int timeout_ms) :
	QObject(parent), divs(divs), timer(this)
{
	timer.setSingleShot(true);
	timer.setInterval(timeout_ms);

	if (divs.empty())
		throw std::runtime_error("AutoScaler called with empty divs vector");

	changeScaleDiv(&this->divs.at(0));

	connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

AutoScaler::~AutoScaler()
{
}

void AutoScaler::setValue(double val)
{
	if (val < min)
		min = val;
	else if (val > max)
		max = val;

	if (val < current_div->lowerBound()) {
		for (auto it = divs.cbegin(); it != divs.cend(); ++it) {
			if (it->lowerBound() <= val) {
				changeScaleDiv(&*it);
				break;
			}
		}
	} else if (val > current_div->upperBound()) {
		for (auto it = divs.cbegin(); it != divs.cend(); ++it) {
			if (it->upperBound() >= val) {
				changeScaleDiv(&*it);
				break;
			}
		}
	}
}

void AutoScaler::changeScaleDiv(const QwtScaleDiv *div)
{
	current_div = div;
	Q_EMIT updateScale(*div);
}

void AutoScaler::startTimer()
{
	min = divs[0].lowerBound();
	max = divs[0].upperBound();
	timer.start();
}

void AutoScaler::stopTimer()
{
	timer.stop();
}

void AutoScaler::timeout()
{
	for (auto it = divs.cbegin(); it != divs.cend(); ++it) {
		if (it->lowerBound() <= min && it->upperBound() >= max) {
			if (&*it != current_div)
				changeScaleDiv(&*it);
			break;
		}
	}

	startTimer();
}

void AutoScaler::setTimeout(int timeout_ms)
{
	timer.setInterval(timeout_ms);

	/* restart timer */
	if (timer.isActive())
		timer.start();
}
