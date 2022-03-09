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

#include "customScale.hpp"

#include <qwt_scale_engine.h>

#include <math.h>

using namespace adiscope;

CustomScale::CustomScale(QWidget *parent) :
	QwtThermo(parent),
	m_currentScale(0),
	autoScale(true)
{
	QVector<QwtScaleDiv> divs;

	divs.push_back(scaleEngine()->divideScale(-0.1, +0.1, 5, 5));

	scaler = new AutoScaler(this, divs);

	connect(scaler, SIGNAL(updateScale(const QwtScaleDiv)),
		this, SLOT(updateScale(const QwtScaleDiv)));
}

CustomScale::~CustomScale()
{
	delete scaler;
}

void CustomScale::addScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize)
{
	scaler->addScaleDivs(scaleEngine()->divideScale(x1, x2, maxMajorSteps, maxMinorSteps,stepSize));
}

void CustomScale::updateScale(const QwtScaleDiv div)
{
	setScale(div);
}

bool CustomScale::getAutoScaler() const
{
	return autoScale;
}

void CustomScale::setAutoScaler(bool newAutoScaler)
{
	autoScale = newAutoScaler;
}

void CustomScale::start()
{
	scaler->startTimer();
}

void CustomScale::stop()
{
	scaler->stopTimer();
}

int numDigits(double num)
{
	if (int(num) == 0) {
		return -1;
	}

	int digits = 0;

	while ((int)num) {
		num /= 10;
		digits++;
	}

	return digits;
}

void CustomScale::setValue(double value)
{
	if (autoScale) {
		// update scale
		int n = numDigits(value);
		double scale = pow(10 , n);
		if (scale < value) {
			scale *= 10;
		}
		if (scale != m_currentScale) {
			updateScale(scaleEngine()->divideScale((-1*scale),scale,5,10));
			m_currentScale = scale;
		}
	}

	scaler->setValue(value);
	QwtThermo::setValue(value);
}
