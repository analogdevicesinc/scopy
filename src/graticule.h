/*
 * Copyright 2018 Analog Devices, Inc.
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

#ifndef GRATICULE_H
#define GRATICULE_H

#include <QObject>
#include <qwt_plot.h>
#include <qwt_plot_scaleitem.h>

namespace adiscope {

static QwtScaleDiv getGraticuleScaleDiv(const QwtScaleDiv& from_scaleDiv);

class Graticule : public QObject
{
	Q_OBJECT
public:
	Graticule(QwtPlot *plot);
	void toggleGraticule();
	void enableGraticule(bool enable);
public Q_SLOTS:
	void onCanvasSizeChanged();

private:
	bool enabled;
	QwtPlot *plot;
	QwtPlotScaleItem *vertScale;
	QwtPlotScaleItem *horizScale;
	QwtPlotScaleItem *vertScale2;
	QwtPlotScaleItem *horizScale2;
};

class GraticulePlotScaleItem: public QwtPlotScaleItem
{
public:
	explicit GraticulePlotScaleItem(
	        QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale,
	        const double pos = 0.0);
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};

}


#endif // GRATICULE_H
