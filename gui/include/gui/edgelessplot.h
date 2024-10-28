/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#ifndef EDGELESSPLOT_H
#define EDGELESSPLOT_H

#include "scopy-gui_export.h"

#include <QwtPlotGrid>
#include <QwtPlotScaleItem>

namespace scopy {
/*
 * EdgelessPlotScaleItem class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotScaleItem : public QwtPlotScaleItem
{
public:
	explicit EdgelessPlotScaleItem(QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale, const double pos = 0.0);
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};

/*
 * EdgelessPlotGrid class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotGrid : public QwtPlotGrid
{
public:
	explicit EdgelessPlotGrid();
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};
} // namespace scopy

#endif // EDGELESSPLOT_H
