/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "scopy-core_export.h"

#include <QColor>
#include <QVector>

#include <QwtPlotItem>

namespace scopy {
namespace acq {

// Dashed guides for a measurement: vertical lines at times, horizontal ones at levels,
// in the plot's axis units. Drawing only; the mouse goes to whatever is below.
class SCOPY_CORE_EXPORT MeasureMarksItem : public QwtPlotItem
{
public:
	MeasureMarksItem();

	void setMarks(const QVector<double> &times, const QVector<double> &levels);
	void clear();
	void setColor(const QColor &c);

	int rtti() const override { return QwtPlotItem::Rtti_PlotUserItem + 44; }
	void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		  const QRectF &canvasRect) const override;

private:
	QVector<double> m_times;
	QVector<double> m_levels;
	QColor m_color{Qt::white};
};

} // namespace acq
} // namespace scopy
