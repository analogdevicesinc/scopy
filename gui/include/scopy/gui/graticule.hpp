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

#ifndef GRATICULE_H
#define GRATICULE_H

#include <QObject>
#include <qwt_plot.h>
#include <qwt_plot_scaleitem.h>

namespace scopy {
namespace gui {

class Graticule : public QObject
{
	Q_OBJECT
public:
	Graticule(QwtPlot* m_plot);
	~Graticule();

	void toggleGraticule();
	void enableGraticule(bool enable);

public Q_SLOTS:
	void onCanvasSizeChanged();

private:
	bool m_enabled;
	QwtPlot* m_plot;
	QwtPlotScaleItem* m_vertScale;
	QwtPlotScaleItem* m_horizScale;
	QwtPlotScaleItem* m_vertScale2;
	QwtPlotScaleItem* m_horizScale2;
};

class GraticulePlotScaleItem : public QwtPlotScaleItem
{
public:
	explicit GraticulePlotScaleItem(QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale, const double pos = 0.0);

	virtual void updateScaleDiv(const QwtScaleDiv&, const QwtScaleDiv&);
};

} // namespace gui
} // namespace scopy

#endif // GRATICULE_H
