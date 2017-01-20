/*
 * Copyright 2017 Analog Devices, Inc.
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

#include "dbgraph.hpp"

#include <qwt_plot_layout.h>

using namespace adiscope;

dBgraph::dBgraph(QWidget *parent) : QwtPlot(parent),
	curve("data")
{
	enableAxis(QwtPlot::xBottom, false);
	enableAxis(QwtPlot::xTop, true);

	setAxisAutoScale(QwtPlot::yLeft, false);
	setAxisAutoScale(QwtPlot::xTop, false);

	plotLayout()->setAlignCanvasToScales(true);

	curve.attach(this);
	curve.setXAxis(QwtPlot::xTop);
}

dBgraph::~dBgraph()
{
}

void dBgraph::setAxesScales(double xmin, double xmax, double ymin, double ymax)
{
	setAxisScale(QwtPlot::xTop, xmin, xmax);
	setAxisScale(QwtPlot::yLeft, ymin, ymax);
}

void dBgraph::setAxesTitles(const QString& x, const QString& y)
{
	setAxisTitle(QwtPlot::xTop, x);
	setAxisTitle(QwtPlot::yLeft, y);
}

void dBgraph::plot(double x, double y)
{
	if (xdata.size() == numSamples + 1)
		return;

	xdata.push(x);
	ydata.push(y);

	curve.setRawSamples(xdata.data(), ydata.data(), xdata.size());
	replot();
}

int dBgraph::getNumSamples() const
{
	return numSamples;
}

void dBgraph::setNumSamples(int num)
{
	numSamples = (unsigned int) num;

	reset();
	ydata.reserve(numSamples + 1);
	xdata.reserve(numSamples + 1);

	replot();
}

void dBgraph::reset()
{
	xdata.clear();
	ydata.clear();
}

void dBgraph::setColor(const QColor& color)
{
	this->color = color;
	curve.setPen(QPen(color));
}

const QColor& dBgraph::getColor() const
{
	return this->color;
}
