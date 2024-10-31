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

#ifndef POLARPLOTCHANNEL_H
#define POLARPLOTCHANNEL_H

#include "scopy-gui_export.h"
#include <QObject>
#include <qwt_polar_curve.h>
#include <qwt_polar_marker.h>

namespace scopy {
class PolarPlotWidget;
class SCOPY_GUI_EXPORT PolarPlotChannel : public QObject
{
	Q_OBJECT
public:
	PolarPlotChannel(QString name, QPen pen, PolarPlotWidget *plot, QObject *parent = nullptr);
	~PolarPlotChannel();

	QwtPolarCurve *curve() const;
	void setSamples(QVector<QwtPointPolar> samples);

public Q_SLOTS:
	void raise();
	void attach();
	void detach();
	void setEnabled(bool b);
	void enable();
	void disable();

	void setThickness(int);

private:
	QwtPolarCurve *m_curve;
	QwtSymbol *m_symbol;
	QwtPolarPlot *m_plot;
	QPen m_pen;
	QwtArraySeriesData<QwtPointPolar> *m_data;
};
} // namespace scopy

#endif // POLARPLOTCHANNEL_H
