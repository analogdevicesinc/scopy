/*
 * Copyright 2016 Analog Devices, Inc.
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

#ifndef SISMOGRAPH_HPP
#define SISMOGRAPH_HPP

#include <QVector>
#include <QWidget>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "autoScaler.hpp"
#include "customFifo.hpp"

namespace adiscope {
	class Sismograph : public QwtPlot
	{
		Q_OBJECT

		Q_PROPERTY(int numSamples
				READ getNumSamples
				WRITE setNumSamples
		)

		Q_PROPERTY(double sampleRate
				READ getSampleRate
				WRITE setSampleRate
		)

	public:
		explicit Sismograph(QWidget *parent = nullptr);
		~Sismograph();

		int getNumSamples() const;
		void setNumSamples(int num);

		double getSampleRate() const;
		void setSampleRate(double rate);

	public Q_SLOTS:
		void plot(double sample);
		void reset();
		void setColor(const QColor& color);
		void updateScale(const QwtScaleDiv);

	private:
		QwtPlotCurve curve;
		unsigned int numSamples;
		double sampleRate;
		AutoScaler *scaler;

		QVector<double> ydata;
		CustomFifo<double> xdata;
	};
}

#endif /* SISMOGRAPH_HPP */
