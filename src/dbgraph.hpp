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
#include "symbol_controller.h"
#include "plot_line_handle.h"
#include "cursor_readouts.h"
#include "plotpickerwrapper.h"

namespace adiscope {
	class OscScaleDraw;
	class PrefixFormatter;
	class OscScaleZoomer;

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

		Q_PROPERTY(QString xaxis_title READ xTitle WRITE setXTitle);
		Q_PROPERTY(QString yaxis_title READ yTitle WRITE setYTitle);

		Q_PROPERTY(double xmin MEMBER xmin WRITE setXMin);
		Q_PROPERTY(double xmax MEMBER xmax WRITE setXMax);
		Q_PROPERTY(double ymin MEMBER ymin WRITE setYMin);
		Q_PROPERTY(double ymax MEMBER ymax WRITE setYMax);

		Q_PROPERTY(QString xunit READ xUnit WRITE setXUnit);
		Q_PROPERTY(QString yunit READ yUnit WRITE setYUnit);

		Q_PROPERTY(bool log_freq MEMBER log_freq WRITE useLogFreq);

	public:
		explicit dBgraph(QWidget *parent = nullptr);
		~dBgraph();

		void setAxesScales(double xmin, double xmax,
				double ymin, double ymax);
		void setAxesTitles(const QString& x, const QString& y);

		int getNumSamples() const;
        void setShowZero(bool en);
        const QwtScaleWidget* getAxisWidget(QwtAxisId id);

	const QColor& getColor() const;
	QString xTitle() const;
	QString yTitle() const;

	void toggleCursors(bool);
	QString cursorIntersection(qreal text);
Q_SIGNALS:
	void VBar1PixelPosChanged(int);
	void VBar2PixelPosChanged(int);
    void resetZoom();

    public Q_SLOTS:
		void plot(double x, double y);
		void reset();

		void setNumSamples(int num);
		void setColor(const QColor& color);
		void setXTitle(const QString& title);
		void setYTitle(const QString& title);
		void setXMin(double val);
		void setXMax(double val);
		void setYMin(double val);
		void setYMax(double val);

		QString xUnit() const;
		QString yUnit() const;
		void setXUnit(const QString& unit);
		void setYUnit(const QString& unit);

		void useLogFreq(bool use_log_freq);

        void onVbar1PixelPosChanged(int pos);
        void onVbar2PixelPosChanged(int pos);

        void onCursor1PositionChanged(int pos);
        void onCursor2PositionChanged(int pos);

        void onCursor1Moved(int);
        void onCursor2Moved(int);

        void scaleDivChanged();
        void mousePressEvent(QMouseEvent *event);
        void onResetZoom();

	private:
		QwtPlotCurve curve;
		unsigned int numSamples;
		double xmin, xmax, ymin, ymax;
		QColor color;
		bool log_freq;

        bool d_cursorsEnabled;

		OscScaleDraw *draw_x, *draw_y;
		PrefixFormatter *formatter;
		OscScaleZoomer *zoomer;

		CustomFifo<double> xdata, ydata;

        SymbolController *d_symbolCtrl;
        VertBar *d_vBar1;
        VertBar *d_vBar2;

        PlotPickerWrapper* picker;

        CursorReadouts *d_cursorReadouts;
	};
}

#endif /* DBGRAPH_HPP */
