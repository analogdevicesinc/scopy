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

#ifndef DBGRAPH_HPP
#define DBGRAPH_HPP

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

#include "customFifo.hpp"
#include "symbol_controller.h"
#include "plot_line_handle.h"
#include "cursor_readouts.h"
#include "DisplayPlot.h"

namespace adiscope {
class OscScaleDraw;
class PrefixFormatter;
class OscScaleZoomer;

class dBgraph : public DisplayPlot
{
	friend class NetworkAnalyzer_API;
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
	explicit dBgraph(QWidget *parent = nullptr, bool isdBgraph = true);

	~dBgraph();

	void setAxesScales(double xmin, double xmax,
			   double ymin, double ymax);
	void setAxesTitles(const QString& x, const QString& y);

	int getNumSamples() const;

	bool eventFilter(QObject *, QEvent *);

	QString getScaleValueFormat(double value, QwtAxisId scale) const;
	QString getScaleValueFormat(double value, QwtAxisId scale, int precision) const;

	void setShowZero(bool en);
	const QwtScaleWidget *getAxisWidget(QwtAxisId id);

	const QColor& getColor() const;
	double getThickness();
	QString xTitle() const;
	QString yTitle() const;

	QString cursorIntersection(qreal text);
	QVector<double> getXAxisData();
	QVector<double> getYAxisData();

	void enableFrequencyBar(bool enable);
	void setYAxisInterval(double min, double max, double correction);

	void setPlotBarEnabled(bool enabled);
	void parametersOverrange(bool enable);

	void enableXaxisLabels();
	void enableYaxisLabels();
	QString formatXValue(double value, int precision) const;
	QString formatYValue(double value, int precision) const;

	void replot();
Q_SIGNALS:

	void resetZoom();
	void frequencySelected(double);
	void frequencyBarPositionChanged(int);

public Q_SLOTS:
	void plot(double x, double y);
	void reset();

	void setNumSamples(int num);
	void setColor(const QColor& color);
	void setThickness(int index);
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
	void useDeltaLabel(bool use_delta);
	void sweepDone();

	void scaleDivChanged();
	void mousePressEvent(QMouseEvent *event);
	void onResetZoom();

	void onFrequencyCursorPositionChanged(int pos);
	void onFrequencyBarMoved(double frequency);

	void addReferenceWaveform(QVector<double> xData, QVector<double> yData);
	void removeReferenceWaveform();
	bool addReferenceWaveformFromPlot();

private Q_SLOTS:
	void onVCursor1Moved(double);
	void onVCursor2Moved(double);
protected Q_SLOTS:
	void showEvent(QShowEvent *event);

private:
	QwtPlotCurve curve;
	QwtPlotCurve reference;
	unsigned int numSamples;
	double xmin, xmax, ymin, ymax;
	QColor color;
	double thickness;
	bool log_freq;
	bool delta_label;
	bool d_plotBarEnabled;

	OscScaleDraw *draw_x, *draw_y;

	OscScaleZoomer *zoomer;

	QVector<double> xdata, ydata;
	unsigned int d_plotPosition;

	VertBar *d_plotBar;
	VertBar *d_frequencyBar;
	PrefixFormatter *d_formatter;

	void setupVerticalBars();
	void setupReadouts();
};
}

#endif /* DBGRAPH_HPP */
