/*
 * Copyright 2017 Analog Devices, Inc.
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

#ifndef DBGRAPH_HPP
#define DBGRAPH_HPP

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "customFifo.hpp"

namespace adiscope {
	class dBgraph : public QwtPlot
	{
		Q_OBJECT

		Q_PROPERTY(int numSamples
				READ getNumSamples
				WRITE setNumSamples
		)

		Q_PROPERTY(QColor color
				READ getColor
				WRITE setColor
		)

	public:
		explicit dBgraph(QWidget *parent = nullptr);
		~dBgraph();

		void setAxesScales(double xmin, double xmax,
				double ymin, double ymax);
		void setAxesTitles(const QString& x, const QString& y);

		int getNumSamples() const;
		void setNumSamples(int num);

		const QColor& getColor() const;
		void setColor(const QColor& color);

	public Q_SLOTS:
		void plot(double x, double y);
		void reset();

	private:
		QwtPlotCurve curve;
		unsigned int numSamples;
		QColor color;

		CustomFifo<double> xdata, ydata;
	};
}

#endif /* DBGRAPH_HPP */
