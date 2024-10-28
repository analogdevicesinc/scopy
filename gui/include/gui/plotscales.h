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

#ifndef PLOTGRID_H
#define PLOTGRID_H

#include "scopy-gui_export.h"

#include <QObject>
#include <QwtPlot>
#include <plotwidget.h>

class QwtPlotGrid;

class QwtPlotScaleItem;
namespace scopy {

class SCOPY_GUI_EXPORT PlotScales : public QObject
{
	Q_OBJECT
public:
	PlotScales(PlotWidget *plot);
	~PlotScales();

	void setGridEn(bool en);
	bool getGridEn();

	void setGraticuleEn(bool en);
	bool getGraticuleEn();

protected:
	void initGrid();
	void initGraticule();
	void initMarginScales();

	void setMarginScalesEn(bool en);
	bool getMarginScalesEn();

private:
	PlotWidget *m_plot;
	QwtPlotGrid *m_grid;
	QwtPlotScaleItem *m_y1Graticule;
	QwtPlotScaleItem *m_y2Graticule;
	QwtPlotScaleItem *m_x1Graticule;
	QwtPlotScaleItem *m_x2Graticule;
	bool m_gridEn, m_graticuleEn, m_marginScalesEn;
	QList<QwtPlotScaleItem *> *m_marginScales;
	QColor m_color;
};

} // namespace scopy

#endif // PLOTGRID_H
