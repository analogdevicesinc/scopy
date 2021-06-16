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

#include <QPainterPath>
#include <QPolygonF>
#include <qwt_spline_cubic.h>
#include <qwt_spline_parametrization.h>

#include <scopy/gui/smooth_curve_fitter.hpp>

using namespace scopy::gui;

SmoothCurveFitter::SmoothCurveFitter()
	: QwtCurveFitter(QwtCurveFitter::Path)
	, m_spline(new QwtSplineCubic())
{
	m_spline->setParametrization(QwtSplineParametrization::ParameterUniform);
}

QPolygonF SmoothCurveFitter::fitCurve(const QPolygonF& points) const { return m_spline->polygon(points, 0.5); }

QPainterPath SmoothCurveFitter::fitCurvePath(const QPolygonF& points) const { return m_spline->painterPath(points); }
