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

#ifndef SMOOTHCURVEFITTER_H
#define SMOOTHCURVEFITTER_H

#include <memory>

#include <qwt_curve_fitter.h>
#include <qwt_spline.h>
#include "scopygui_export.h"

namespace adiscope {
class SCOPYGUI_EXPORT SmoothCurveFitter : public QwtCurveFitter
{
public:
	SmoothCurveFitter();

	virtual QPolygonF fitCurve( const QPolygonF &points ) const;
	virtual QPainterPath fitCurvePath( const QPolygonF &points ) const;

private:
        std::unique_ptr<QwtSpline> d_spline;
};
}

#endif // SMOOTHCURVEFITTER_H
