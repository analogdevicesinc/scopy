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

#ifndef POLARPLOTWIDGET_H
#define POLARPLOTWIDGET_H

#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QWidget>
#include <polarplotchannel.h>
#include <qwt_plot_zoomer.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_plot.h>

namespace scopy {
class SCOPY_GUI_EXPORT PolarPlotWidget : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		MGS_LINES = 0,
		MGS_DASH,
		MGS_DOT,
		MGS_DASH_DOT,
		MGS_DASH_DOT_DOT
	} MinorGridStyle;

	PolarPlotWidget(QWidget *parent = nullptr);
	~PolarPlotWidget();

	void addPlotChannel(PolarPlotChannel *ch);
	void removePlotChannel(PolarPlotChannel *ch);

	QwtPolarPlot *plot() const;
	void setAzimuthInterval(double min, double max, double step = 0.0);
	void setRadiusInterval(double min, double max, double step = 0.0);

	void showRadiusAxis(bool left = true, bool right = true, bool top = true, bool bottom = true);
	void showAzimuthAxis(bool show = true);

	void setGridPen(QPen pen);
	void showMinorGrid(bool show = true);
	void setMinorGridPen(int style, QPen pen = QPen(Qt::gray));

	void setBgColor(const QColor &color);

	void setData(QVector<QVector<QwtPointPolar>> data);

public Q_SLOTS:
	void replot();

private:
	void setupZoomer();

	QwtPolarPlot *m_plot;
	QwtPolarGrid *m_grid;
	QList<PolarPlotChannel *> m_plotChannels;
	QGridLayout *m_layout;
};
} // namespace scopy

#endif // POLARPLOTWIDGET_H
