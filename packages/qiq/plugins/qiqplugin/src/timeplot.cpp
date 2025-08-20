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

#include "timeplot.h"
#include "plotaxis.h"
#include "stylehelper.h"

using namespace scopy::qiqplugin;

TimePlot::TimePlot() { m_plot = new PlotWidget(); }

TimePlot::~TimePlot()
{
	if(m_plot) {
		delete m_plot;
	}
}

int TimePlot::id() { return m_plotInfo.id; }

void TimePlot::init(QIQPlotInfo info, int samplingFreq)
{
	m_plotInfo = info;
	m_samplingFreq = samplingFreq;
	m_plot->plot()->setTitle(info.title);
	initAxis();
	int chCount = info.channels.size();
	for(int i = 0; i < chCount; i++) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		PlotChannel *ch = new PlotChannel(chId, pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->setEnabled(true);
	}
}

void TimePlot::updateData(QList<CurveData> curveData)
{
	const QList<PlotChannel *> channels = m_plot->getChannels();
	if(curveData.size() != channels.size()) {
		qWarning() << "Dataset size must be the same as the channels size!";
		return;
	}
	for(PlotChannel *ch : channels) {
		CurveData curve = curveData.takeFirst();
		if(curve.x.isEmpty()) {
			computeXAxis(curve.y.size());
			ch->curve()->setSamples(m_xTime, curve.y);
		} else {
			ch->curve()->setSamples(curve.x, curve.y);
		}
	}
	m_plot->replot();
}

void TimePlot::setSamplingFreq(int samplingFreq) { m_samplingFreq = samplingFreq; }

QWidget *TimePlot::widget() { return m_plot; }

void TimePlot::computeXAxis(int size)
{
	if(m_xTime.size() == size) {
		return;
	}
	m_xTime.clear();
	for(int i = 0; i < size; i++) {
		m_xTime.push_back((double)i / m_samplingFreq);
	}
	m_plot->xAxis()->setInterval(0, (double)size / m_samplingFreq);
}

void TimePlot::initAxis()
{
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot->xAxis()->setUnits(m_plotInfo.xLabel);

	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->xAxis()->setInterval(0, 1);
	m_plot->xAxis()->setVisible(true);

	m_plot->yAxis()->setUnits(m_plotInfo.yLabel);
	m_plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->yAxis()->setInterval(-200, 200);
	m_plot->yAxis()->setVisible(true);

	m_plot->replot();
}
