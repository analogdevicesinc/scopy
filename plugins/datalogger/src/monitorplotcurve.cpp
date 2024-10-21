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

#include "monitorplotcurve.hpp"

#include <datamonitorutils.hpp>
#include <plotaxis.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlotCurve::MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent)
	: m_dataMonitorModel(dataMonitorModel)
	, m_plot(plot)
{
	QPen chpen = QPen(dataMonitorModel->getColor(), 1);

	m_plotch = new PlotChannel(dataMonitorModel->getName(), chpen, plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(m_plotch);
	m_plotch->setEnabled(true);

	m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(), m_dataMonitorModel->getYdata()->data(),
					 m_dataMonitorModel->getYdata()->size());

	connect(dataMonitorModel, &DataMonitorModel::valueUpdated, plot, [=, this]() {
		m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(),
						 m_dataMonitorModel->getYdata()->data(),
						 m_dataMonitorModel->getYdata()->size());
		plot->replot();
	});
}

MonitorPlotCurve::~MonitorPlotCurve() { m_plot->removePlotChannel(m_plotch); }

void MonitorPlotCurve::clearCurveData()
{
	m_plotch->curve()->setRawSamples(m_dataMonitorModel->getXdata()->data(), m_dataMonitorModel->getYdata()->data(),
					 m_dataMonitorModel->getYdata()->size());
}

void MonitorPlotCurve::refreshCurve()
{
	m_plotch->detach();
	m_plotch->attach();
}

void MonitorPlotCurve::toggleActive(bool toggled) { m_plotch->setEnabled(toggled); }

PlotChannel *MonitorPlotCurve::plotch() const { return m_plotch; }

double MonitorPlotCurve::curveMinVal() { return m_dataMonitorModel->minValue(); }

double MonitorPlotCurve::curveMaxVal() { return m_dataMonitorModel->maxValue(); }

#include "moc_monitorplotcurve.cpp"
