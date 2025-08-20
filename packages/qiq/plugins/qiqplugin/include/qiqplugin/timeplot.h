/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef TIMEPLOT_H
#define TIMEPLOT_H

#include "iplot.h"
#include <gui/plotwidget.h>
namespace scopy::qiqplugin {

class TimePlot : public IPlot
{
public:
	TimePlot();
	~TimePlot();

	int id() override;
	void init(QIQPlotInfo info, int samplingFreq) override;
	void updateData(QList<CurveData> curveData) override;
	void setSamplingFreq(int samplingFreq) override;
	QWidget *widget() override;

private:
	void computeXAxis(int size);
	void initAxis();

	int m_samplingFreq;
	PlotWidget *m_plot = nullptr;
	qiqplugin::QIQPlotInfo m_plotInfo;
	QVector<double> m_xTime;
};

} // namespace scopy::qiqplugin

#endif // TIMEPLOT_H
