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

#include "nyquistGraph.hpp"

#include <QList>
#include <QPainter>
#include <QPointF>
#include <QRect>
#include <QApplication>

#include <QDebug>

#include <qwt_legend.h>
#include <qwt_point_polar.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>

#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>

namespace adiscope {
	class NyquistSamplesArray : public QwtArraySeriesData<QwtPointPolar>
	{
	public:
		NyquistSamplesArray() : QwtArraySeriesData<QwtPointPolar>() {}

		void addSample(const QwtPointPolar& point) {
			d_samples.push_back(point);
		}
		void clear() { d_samples.clear(); }
		void reserve(unsigned int nb) { d_samples.reserve(nb); }
		QRectF boundingRect() const;

		QwtPointPolar sample(size_t index) const {
			return d_samples.at(index);
		}
	};
}


using namespace adiscope;

const QwtInterval radialInterval( 0.0, 10.0 );
const QwtInterval azimuthInterval( 0.0, 360.0 );

QRectF NyquistSamplesArray::boundingRect() const
{
	double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0;

	for (auto it = d_samples.begin();
			it != d_samples.end(); ++it) {
		double point_x = it->radius() * cos(it->azimuth());
		double point_y = it->radius() * sin(it->azimuth());

		if (point_x < xmin)
			xmin = point_x;
		else if (point_x > xmax)
			xmax = point_x;
		if (point_y < ymin)
			ymin = point_y;
		else if (point_y > ymax)
			ymax = point_y;
	}

	return QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax));
}

NyquistGraph::NyquistGraph(QWidget *parent) : QwtPolarPlot(parent),
	mag_min(0.0), mag_max(0.0), samples(new NyquistSamplesArray),
	grid(new QwtPolarGrid), numSamples(0)
{
	setAutoReplot(false);
	setScale(QwtPolar::Azimuth, 0.0, 360.0, 45.0);
	grid->setPen(QPen(Qt::white));

	for (unsigned int i = 0; i < QwtPolar::ScaleCount; i++)
	{
		grid->showGrid(i);
		grid->showMinorGrid(i);

		QPen minorPen(Qt::gray);
		minorPen.setStyle(Qt::DotLine);
		grid->setMinorGridPen(i, minorPen);
	}

	grid->showAxis(QwtPolar::AxisAzimuth, true);
	grid->showAxis(QwtPolar::AxisLeft, false);
	grid->showAxis(QwtPolar::AxisRight, true);
	grid->showAxis(QwtPolar::AxisTop, true);
	grid->showAxis(QwtPolar::AxisBottom, false);
	grid->showGrid(QwtPolar::Azimuth, true);
	grid->showGrid(QwtPolar::Radius, true);
	grid->attach(this);

	panner = new QwtPolarPanner(this->canvas());
	zoomer = new NyquistPlotZoomer(this->canvas());
	panner->setEnabled(false);
	zoomer->setEnabled(false);

	curve.setData(samples);
	curve.attach(this);
}

NyquistGraph::~NyquistGraph()
{
}

void NyquistGraph::enableZooming(QPushButton *btnZoomIn, QPushButton *btnZoomOut)
{
	connect(btnZoomIn, &QPushButton::clicked, [=](){
		zoomer->zoomIn();
		if (zoomer->isZoomed())
			QApplication::setOverrideCursor(Qt::OpenHandCursor);
		else
			QApplication::setOverrideCursor(Qt::CrossCursor);
	});
	connect(btnZoomOut, &QPushButton::clicked, [=](){
		zoomer->zoomOut();
		if (zoomer->isZoomed())
			QApplication::setOverrideCursor(Qt::OpenHandCursor);
		else
			QApplication::setOverrideCursor(Qt::CrossCursor);
	});

	zoomer->setEnabled(true);
	panner->setEnabled(true);
	zoomer->setWheelFactor(1);
}

const QColor NyquistGraph::getColor() const
{
	return curve.pen().color();
}

const QColor& NyquistGraph::getBgColor() const
{
	return plotBackground().color();
}

void NyquistGraph::setColor(const QColor& color)
{
	QPen pen(color);
	pen.setWidthF(2.0);
	curve.setPen(pen);
}

void NyquistGraph::setBgColor(const QColor& color)
{
	setPlotBackground(QBrush(color));
}

void NyquistGraph::plot(double azimuth, double radius)
{
	if (curve.dataSize() == numSamples + 1)
		return;

	samples->addSample(QwtPointPolar(azimuth, radius));
	replot();
}

int NyquistGraph::getNumSamples() const
{
	return numSamples;
}

void NyquistGraph::setNumSamples(int num)
{
	numSamples = (unsigned int) num;

	reset();
	samples->reserve(numSamples + 1);

	replot();
}

void NyquistGraph::reset()
{
	samples->clear();
}

void NyquistGraph::setMin(double min)
{
	mag_min = min;
	setScale(QwtPolar::Radius, mag_max, min);
	replot();
}

void NyquistGraph::setMax(double max)
{
	mag_max = max;
	setScale(QwtPolar::Radius, max, mag_min);
	replot();
}

QFont NyquistGraph::getFontAzimuth() const
{
	return grid->axisFont(QwtPolar::AxisAzimuth);
}

QFont NyquistGraph::getFontRadius() const
{
	return grid->axisFont(QwtPolar::AxisRight);
}

void NyquistGraph::setFontAzimuth(const QFont& font)
{
	grid->setAxisFont(QwtPolar::AxisAzimuth, font);
}

void NyquistGraph::setFontRadius(const QFont& font)
{
	grid->setAxisFont(QwtPolar::AxisRight, font);
	grid->setAxisFont(QwtPolar::AxisTop, font);
}

void NyquistGraph::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		zoomer->cancelZoom();
		QApplication::setOverrideCursor(Qt::CrossCursor);
		QwtPolarPlot::mousePressEvent(event);
		return;
	}

	if (zoomer->isZoomed())
		QApplication::setOverrideCursor(Qt::ClosedHandCursor);

	QwtPolarPlot::mousePressEvent(event);
}

void NyquistGraph::mouseReleaseEvent(QMouseEvent *event)
{
	if (zoomer->isZoomed())
		QApplication::setOverrideCursor(Qt::OpenHandCursor);
	QwtPolarPlot::mouseReleaseEvent(event);
}

void NyquistGraph::enterEvent(QEvent *event)
{
	if (zoomer->isZoomed())
		QApplication::setOverrideCursor(Qt::OpenHandCursor);
	else
		QApplication::setOverrideCursor(Qt::CrossCursor);
	QwtPolarPlot::enterEvent(event);
}

void NyquistGraph::leaveEvent(QEvent *event)
{
	QApplication::setOverrideCursor(Qt::ArrowCursor);
	QwtPolarPlot::leaveEvent(event);
}
