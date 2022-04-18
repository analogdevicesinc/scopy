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

#ifndef NYQUISTGRAPH_HPP
#define NYQUISTGRAPH_HPP

#include "customFifo.hpp"
#include "dbgraph.hpp"
#include "nyquistplotzoomer.h"

#include <qwt_polar_curve.h>
#include <qwt_polar_plot.h>
#include <qwt_polar_panner.h>

#include <QPushButton>
#include <QMouseEvent>

class QwtPolarGrid;

namespace adiscope {
	class NyquistSamplesArray;

	class NyquistGraph : public QwtPolarPlot
	{
		Q_OBJECT

		Q_PROPERTY(int numSamples
				READ getNumSamples
				WRITE setNumSamples
		);

		Q_PROPERTY(double min MEMBER mag_min WRITE setMin);
		Q_PROPERTY(double max MEMBER mag_max WRITE setMax);
		Q_PROPERTY(QColor color READ getColor WRITE setColor);
		Q_PROPERTY(QColor bg_color READ getBgColor WRITE setBgColor);
		Q_PROPERTY(QFont font_azimuth
				READ getFontAzimuth WRITE setFontAzimuth);
		Q_PROPERTY(QFont font_radius
				READ getFontRadius WRITE setFontRadius);

	public:
		explicit NyquistGraph(QWidget *parent = nullptr);
		~NyquistGraph();

		const QColor getColor() const;
		const QColor& getBgColor() const;
		int getNumSamples() const;
		double getThickness() const;

		QFont getFontAzimuth() const;
		QFont getFontRadius() const;

		void enableZooming(QPushButton *bntZoomIn, QPushButton *btnZoomOut);

	public Q_SLOTS:
		void setColor(const QColor& color);
		void setBgColor(const QColor& color);
		void setNumSamples(int num);
		void plot(double x, double y);
		void reset();
		void setThickness(int value);

		void setMin(double min);
		void setMax(double max);

		void setFontAzimuth(const QFont& font);
		void setFontRadius(const QFont& font);

		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void enterEvent(QEnterEvent *event);
		void leaveEvent(QEvent *event);

	private:
		double mag_min, mag_max;
		unsigned int numSamples;
		NyquistSamplesArray *samples;
		QwtPolarGrid *grid;
		QwtPolarCurve curve;
		QwtPolarPanner *panner;
		NyquistPlotZoomer *zoomer;
		double m_thickness;

	};
}

#endif /* NYQUISTGRAPH_HPP */
