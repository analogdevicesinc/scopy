/*
 * Copyright 2019 Analog Devices, Inc.
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

#include "smoothcurvefitter.h"

#include <qwt_spline_cubic.h>
#include <qwt_spline_parametrization.h>

using namespace adiscope;

SmoothCurveFitter::SmoothCurveFitter()
	: QwtCurveFitter(QwtCurveFitter::Path), d_spline(new QwtSplineCubic()) {
	d_spline->setParametrization(
		QwtSplineParametrization::ParameterUniform);
}

QPolygonF SmoothCurveFitter::fitCurve(const QPolygonF &points) const {
	return d_spline->polygon(points, 0.5);
}

QPainterPath SmoothCurveFitter::fitCurvePath(const QPolygonF &points) const {
	return d_spline->painterPath(points);
}
