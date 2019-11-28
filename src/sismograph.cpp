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

#include "sismograph.hpp"

#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>

using namespace adiscope;

Sismograph::Sismograph(QWidget *parent) : QwtPlot(parent),
	curve("data"), sampleRate(10.0)
{
	enableAxis(QwtPlot::xBottom, false);
	enableAxis(QwtPlot::xTop, true);

	setAxisTitle(QwtPlot::xTop, "Voltage (V)");
	setAxisTitle(QwtPlot::yLeft, "Time (s)");

	setAxisAutoScale(QwtPlot::yLeft, false);

	setAxisAutoScale(QwtPlot::xTop, false);
	setAxisScale(QwtPlot::xTop, -0.1, +0.1);

	QVector<QwtScaleDiv> divs;
	QwtScaleEngine *engine = axisScaleEngine(QwtPlot::xTop);
	divs.push_back(engine->divideScale(-0.1, +0.1, 5, 5));
	divs.push_back(engine->divideScale(-1.0, +1.0, 5, 5));
	divs.push_back(engine->divideScale(-5.0, +5.0, 10, 2));
	divs.push_back(engine->divideScale(-25.0, +25.0, 10, 5));

	scaler = new AutoScaler(this, divs);

	connect(scaler, SIGNAL(updateScale(const QwtScaleDiv)),
			this, SLOT(updateScale(const QwtScaleDiv)));

	setNumSamples(100);

	plotLayout()->setAlignCanvasToScales(true);

	curve.attach(this);
	curve.setXAxis(QwtPlot::xTop);
}

Sismograph::~Sismograph()
{
	delete scaler;
}

void Sismograph::plot(double sample)
{
	if (xdata.size() == numSamples + 1)
		xdata.pop();

	xdata.push(sample);
	scaler->setValue(sample);

	curve.setRawSamples(xdata.data(), ydata.data() + (ydata.size() -
				xdata.size()), xdata.size());
	replot();
}

int Sismograph::getNumSamples() const
{
	return numSamples;
}

void Sismograph::setNumSamples(int num)
{
	numSamples = (unsigned int) num;

	reset();
	ydata.resize(numSamples + 1);
	xdata.reserve(numSamples + 1);

	setAxisScale(QwtPlot::yLeft, (double) numSamples / sampleRate, 0.0);

	setSampleRate(sampleRate);
	replot();

	scaler->setTimeout((double) numSamples * 1000.0 / sampleRate);
}

double Sismograph::getSampleRate() const
{
	return sampleRate;
}

void Sismograph::setSampleRate(double rate)
{
	sampleRate = rate;

	for (unsigned int i = 0; i <= numSamples; i++)
		ydata[i] = (double)(numSamples - i) / sampleRate;
}

void Sismograph::reset()
{
	xdata.clear();
	scaler->startTimer();
}

void Sismograph::setColor(const QColor& color)
{
	curve.setPen(QPen(color));
}

void Sismograph::updateScale(const QwtScaleDiv div)
{
	setAxisScale(QwtPlot::xTop, div.lowerBound(), div.upperBound());
}
