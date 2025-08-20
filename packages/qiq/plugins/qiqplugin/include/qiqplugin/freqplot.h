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

#ifndef FREQPLOT_H
#define FREQPLOT_H

#include "iplot.h"
#include <gui/plotwidget.h>
namespace scopy::qiqplugin {

class FreqPlot : public IPlot
{
public:
	FreqPlot();
	~FreqPlot();

	int id() override;
	void init(QIQPlotInfo info, int samplingFreq) override;
	void updateData(QList<CurveData> curveData) override;
	void setSamplingFreq(int samplingFreq) override;
	QWidget *widget() override;

private:
	void computeXAxis(int size);
	void initAxis();

	int m_samplingFreq;
	qiqplugin::QIQPlotInfo m_plotInfo;
	PlotWidget *m_plot = nullptr;
	QVector<double> m_xFreq;
};

} // namespace scopy::qiqplugin

#endif // FREQPLOT_H
