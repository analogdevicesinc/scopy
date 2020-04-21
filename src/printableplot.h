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

#ifndef PRINTABLEPLOT_H
#define PRINTABLEPLOT_H

/*Qwt includes*/
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_renderer.h>

/*Qt includes*/
#include <QWidget>

namespace adiscope {
class PrintablePlot : public QwtPlot {
	Q_OBJECT
public:
	PrintablePlot(QWidget *parent);

	void dropBackground(bool drop);
	void setUseNativeDialog(bool nativeDialog);

public Q_SLOTS:
	void printPlot(const QString &toolName = "");

private:
	QwtPlotRenderer d_plotRenderer;
	QwtLegend *legendDisplay;
	bool d_useNativeDialog;
};
} // namespace adiscope

#endif // PRINTABLEPLOT_H
