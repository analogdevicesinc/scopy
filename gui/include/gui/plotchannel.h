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

#ifndef PLOTCHANNEL_H
#define PLOTCHANNEL_H

#include "scopy-gui_export.h"

#include <QPen>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotMarker>
#include <QwtSymbol>

namespace scopy {
class PlotAxis;
class PlotWidget;
class PlotAxisHandle;
class SCOPY_GUI_EXPORT PlotChannel : public QObject
{
	Q_OBJECT
public:
	typedef enum
	{
		PCS_LINES = 0,
		PCS_DOTS,
		PCS_STEPS,
		PCS_STICKS,
		PCS_SMOOTH
	} PlotCurveStyle;

	PlotChannel(QString name, QPen pen, PlotAxis *xAxis, PlotAxis *yAxis, QObject *parent = nullptr);
	~PlotChannel();

	QwtPlotCurve *curve() const;
	PlotAxis *xAxis() const;
	PlotAxis *yAxis() const;

	PlotAxisHandle *handle() const;
	void setHandle(PlotAxisHandle *newHandle);

	QList<QwtPlotMarker *> markers();
	QwtPlotMarker *buildMarker(QString str, QwtSymbol::Style shape, double x, double y);
	void clearMarkers();
	void removeMarker(QwtPlotMarker *m);
	void addMarker(QwtPlotMarker *m);
	void setSamples(const float *xData, const float *yData, size_t size, bool copy = true);

	QString name() const;

	void init();
	void deinit();
	int thickness() const;
	void setThickness(int newThickness);

	int style() const;
	void setStyle(int newStyle);

	void setYAxis(PlotAxis *newYAxis);

	void setXAxis(PlotAxis *newXAxis);
	double getValueAt(double pos);

	bool isEnabled() const;

public Q_SLOTS:
	void raise();
	void attach();
	void detach();
	void setEnabled(bool b);
	void enable();
	void disable();

private:
	void setThicknessInternal(int);
	void setStyleInternal(int);

Q_SIGNALS:
	void attachCurve(QwtPlotCurve *curve);
	void doReplot();
	void newData(const float *xData, const float *yData, size_t size, bool);

	void thicknessChanged();

	void styleChanged();

private:
	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;
	PlotAxisHandle *m_handle;
	QwtPlotCurve *m_curve;
	QList<QwtPlotMarker *> m_markers;
	QwtSymbol *symbol;
	QPen m_pen;
	float *m_data;
	QString m_name;

	int m_thickness;
	int m_style;

	bool m_isEnabled;

	Q_PROPERTY(int thickness READ thickness WRITE setThickness NOTIFY thicknessChanged);
	Q_PROPERTY(int style READ style WRITE setStyle NOTIFY styleChanged);
};
} // namespace scopy

#endif // PLOTCHANNEL_H
