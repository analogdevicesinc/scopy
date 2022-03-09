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

#include <QPen>
#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>
#include "plot_utils.hpp"
#include <math.h>
#include <cstdlib>

using namespace scopy;

Sismograph::Sismograph(QWidget *parent) : QwtPlot(parent)
      , curve("data"), sampleRate(10)
      , m_currentScale(-Q_INFINITY)
      , m_currentMaxValue(-Q_INFINITY)
      , interval(10)
      , autoscale(true)
{
	setAxisVisible(QwtAxis::XBottom, false);
	setAxisVisible(QwtAxis::XTop, true);

	setAxisTitle(QwtAxis::XTop, tr("Voltage (V)"));
	setAxisTitle(QwtAxis::YLeft, tr("Time (s)"));

	setAxisAutoScale(QwtAxis::YLeft, false);

	setAxisAutoScale(QwtAxis::XTop, false);
	setAxisScale(QwtAxis::XTop, -0.1, +0.1);

	QVector<QwtScaleDiv> divs;
	QwtScaleEngine *engine = axisScaleEngine(QwtAxis::XTop);
	divs.push_back(engine->divideScale(-0.1, +0.1, 5, 5));

	scaler = new AutoScaler(this, divs);
	connect(scaler, &AutoScaler::updateScale, this, [=](QwtScaleDiv div){
		if (autoscale) {
			updateScale();
		} else {
			updateScale(div);
		}
	});

	plotLayout()->setAlignCanvasToScales(true);
	scaleLabel = new CustomQwtScaleDraw();
	scaleLabel->setUnitOfMeasure(m_unitOfMeasureSymbol);
	this->setAxisScaleDraw(QwtAxis::XTop,scaleLabel);
	curve.attach(this);
	curve.setXAxis(QwtAxis::XTop);
}

Sismograph::~Sismograph()
{
	delete scaler;
}

void Sismograph::plot(double sample)
{
	xdata.push_back(sample);

	if (xdata.size() == numSamples + 2){
		xdata.pop_front();
	}

	if (sample > m_currentMaxValue && autoscale) {
		updateScale();
	}

	scaler->setValue(sample);
	curve.setRawSamples(xdata.data(), ydata.data() + (ydata.size() - xdata.size()), xdata.size());
	replot();
}

double Sismograph::findMaxInFifo()
{
	double max = -Q_INFINITY;
	for (int i = 0 ; i < xdata.size(); i++) {
		if (abs(xdata.at(i)) > max) {
			max = abs(xdata.at(i));
		}
	}
	return max;
}

bool Sismograph::getAutoscale() const
{
	return autoscale;
}

void Sismograph::setAutoscale(bool newAutoscale)
{
	autoscale = newAutoscale;
}

void Sismograph::addScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize)
{
	QwtScaleEngine *scaleEngine = axisScaleEngine(QwtAxis::XTop);
	scaler->addScaleDivs(scaleEngine->divideScale(x1, x2, maxMajorSteps, maxMinorSteps,stepSize));
}

void Sismograph::updateScale()
{
	double sample = findMaxInFifo();
	/// compute scale
	int digits = 0;
	double num = sample;
	if (num != 0) {
		if (int(num) == 0) {
			while ((int)num*10 == 0) {
				num *= 10;
				digits++;
			}
			digits = -1 * (digits - 1);
		} else {

			while ((int)num) {
				num /= 10;
				digits++;
			}
		}
	}
	double scale = pow(10 , digits);

	////update scale
	MetricPrefixFormatter m_prefixFormater;
	QString formatedPrefix = m_prefixFormater.getFormatedMeasureUnit(sample);
	setPlotAxisXTitle(formatedPrefix + m_unitOfMeasureName + "(" + formatedPrefix + m_unitOfMeasureSymbol + ")");
	scaleLabel->setUnitOfMeasure(m_unitOfMeasureSymbol);

	QwtScaleEngine *scaleEngine = axisScaleEngine(QwtAxis::XTop);
	updateScale(scaleEngine->divideScale((-1*scale),scale,5,10));
	m_currentScale = scale;
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
	setAxisScale(QwtAxis::YLeft, (double) numSamples / sampleRate, 0.0);
	replot();
	scaler->setTimeout((double) numSamples * 1000.0 / sampleRate);

	for (unsigned int i = 0; i <= numSamples; i++)
		ydata[i] = (double)(numSamples - i) / sampleRate;
}

void Sismograph::updateYScale(double max, double min)
{
	reset();
	ydata.resize(numSamples + 1);
	xdata.reserve(numSamples + 1);
	setSampleRate(sampleRate);
	setAxisScale(QwtAxis::YLeft, min,max);
	replot();
}

void Sismograph::setHistoryDuration(double time)
{
	interval = time;
	setNumSamples(interval * sampleRate);
}

double Sismograph::getSampleRate() const
{
	return sampleRate;
}

void Sismograph::setSampleRate(double rate)
{
	sampleRate = rate;
	setNumSamples(interval * sampleRate);
}

void Sismograph::reset()
{
	xdata.clear();
	scaler->startTimer();
}

void Sismograph::setColor(const QColor& color)
{
	QPen pen(curve.pen());
	pen.setColor(color);
	curve.setPen(pen);
	replot();
}

void Sismograph::updateScale(const QwtScaleDiv div)
{
	setAxisScale(QwtAxis::XTop, div.lowerBound(), div.upperBound());
	setAxisScaleDraw(QwtAxis::XTop,scaleLabel);
}

void Sismograph::setLineWidth(qreal width)
{
        QPen pen(curve.pen());
        pen.setWidthF(width);
        curve.setPen(QPen(pen));
}

void Sismograph::setLineStyle(Qt::PenStyle lineStyle)
{
	QPen pen(curve.pen());
	pen.setStyle(lineStyle);
	curve.setPen(QPen(pen));
	replot();
}

void Sismograph::setUnitOfMeasure(QString unitOfMeasureName,QString unitOfMeasureSymbol)
{
	m_unitOfMeasureName = unitOfMeasureName;
	m_unitOfMeasureSymbol = unitOfMeasureSymbol;
}

void Sismograph::setPlotAxisXTitle(QString title)
{
	setAxisTitle(QwtAxis::XTop, title);
}
#include "moc_sismograph.cpp"
