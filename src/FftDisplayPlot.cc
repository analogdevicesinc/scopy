/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "FftDisplayPlot.h"
#include "spectrumUpdateEvents.h"
#include "signal_generator.hpp"

using namespace adiscope;

FftDisplayPlot::FftDisplayPlot(int nplots, QWidget *parent) :
	DisplayPlot(nplots, parent),
	d_start_frequency(0),
	d_stop_frequency(1000)
{
	for (unsigned int i = 0; i < nplots; i++) {
		auto plot = new QwtPlotCurve(QString("Data %1").arg(i));

		plot->setPen(QPen(d_CurveColors[i]));
		plot->attach(this);

		d_plot_curve.push_back(plot);
		y_data.push_back(nullptr);
	}
	d_numPoints = 1024;
	x_data = new double[d_numPoints];

	dBFormatter.setTwoDecimalMode(false);
	freqFormatter.setTwoDecimalMode(true);

	OscScaleDraw *yScaleDraw = new OscScaleDraw(&dBFormatter, "dB");
	setAxisScaleDraw(QwtPlot::yLeft, yScaleDraw);
	yScaleDraw->setFloatPrecision(2);

	OscScaleDraw *xScaleDraw = new OscScaleDraw(&freqFormatter, "Hz");
	setAxisScaleDraw(QwtPlot::xBottom, xScaleDraw);
	xScaleDraw->setFloatPrecision(2);

	_resetXAxisPoints();

	setMinXaxisDivision(1);     // A minimum division of 1 Hz
	setMaxXaxisDivision(5E6);   // A maximum division of 5 MHz
	setMinYaxisDivision(1E-3);  // A minimum division of 1 mdB
	setMaxYaxisDivision(100); // A maximum division of 100 dB
	setVertUnitsPerDiv(20);
	setVertOffset(-VertUnitsPerDiv() * 5);
}

FftDisplayPlot::~FftDisplayPlot()
{
	if (x_data)
		delete[] x_data;

	for (unsigned int i = 0; i < d_nplots; i++) {
		if (y_data[i])
			delete[] y_data[i];
	}
}

void FftDisplayPlot::replot()
{
	QwtPlot::replot();
}

void FftDisplayPlot::plotData(const std::vector<double *> pts,
		uint64_t num_points)
{
	uint64_t halfNumPoints = num_points / 2;

	if (d_stop || halfNumPoints == 0)
		return;

	if (halfNumPoints != d_numPoints) {
		d_numPoints = halfNumPoints;

		if (x_data)
			delete []x_data;

		x_data = new double[halfNumPoints];

		for (unsigned int i = 0; i < d_nplots; i++) {
			if (y_data[i])
				delete[] y_data[i];

			y_data[i] = new double[halfNumPoints];

#if QWT_VERSION < 0x060000
			d_plot_curve[i]->setRawData(x_data,
					y_data[i], halfNumPoints);
#else
			d_plot_curve[i]->setRawSamples(x_data,
					y_data[i], halfNumPoints);
#endif
		}
	}

	for (unsigned int i = 0; i < d_nplots; i++)
		memcpy(y_data[i], pts[i], halfNumPoints * sizeof(double));

	_resetXAxisPoints();

	replot();
}

void FftDisplayPlot::_resetXAxisPoints()
{
	double fft_bin_size = (d_stop_frequency - d_start_frequency)
		/ static_cast<double>(d_numPoints);

	double freqValue = d_start_frequency;
	for(int64_t loc = 0; loc < d_numPoints; loc++) {
		x_data[loc] = freqValue;
		freqValue += fft_bin_size;
	}

	setAxisScale(QwtPlot::xBottom, d_start_frequency, d_stop_frequency);
}

void FftDisplayPlot::customEvent(QEvent *e)
{
	if (e->type() == TimeUpdateEvent::Type()) {
		TimeUpdateEvent *ev = static_cast<TimeUpdateEvent *>(e);

		this->plotData(ev->getTimeDomainPoints(),
				ev->getNumTimeDomainDataPoints());
	}
}

void FftDisplayPlot::setSampleRate(double sr, double units,
	const std::string &strunits)
{
	d_start_frequency = 0;
	d_stop_frequency = sr / 2;

	_resetXAxisPoints();
}
