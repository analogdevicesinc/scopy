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

#ifndef PRINTABLEPLOT_H
#define PRINTABLEPLOT_H

/*Qwt includes*/
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_renderer.h>

/*Qt includes*/
#include <QWidget>

namespace scopy {
namespace gui {

class PrintablePlot : public QwtPlot
{
	Q_OBJECT
public:
	PrintablePlot(QWidget* parent);

	void dropBackground(bool drop);
	void setUseNativeDialog(bool nativeDialog);

public Q_SLOTS:
	void printPlot(const QString& toolName = "");

private:
	QwtPlotRenderer m_plotRenderer;
	QwtLegend* m_legendDisplay;
	bool m_useNativeDialog;
};
} // namespace gui
} // namespace scopy

#endif // PRINTABLEPLOT_H
