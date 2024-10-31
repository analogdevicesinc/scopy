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

#include "plotautoscaler.h"

#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_TIMEYAUTOSCALE, "Time Y-Autoscale");

using namespace scopy::gui;

PlotAutoscaler::PlotAutoscaler(QObject *parent)
	: QObject(parent)
	, m_timeout(1000)
{
	m_xAxisMode = false;
	m_tolerance = 0;
	// AUTOSCALE
	m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(m_timeout);
	connect(m_autoScaleTimer, &QTimer::timeout, this, &PlotAutoscaler::autoscale);

	m_max = -1000000.0;
	m_min = 1000000.0;
}

PlotAutoscaler::~PlotAutoscaler() {}

void PlotAutoscaler::start() { m_autoScaleTimer->start(); }

void PlotAutoscaler::stop() { m_autoScaleTimer->stop(); }

void PlotAutoscaler::onNewData(const float *xData, const float *yData, size_t size, bool copy)
{
	// this is a little wonky but should work
	if(!m_autoScaleTimer->isActive())
		return;

	for(int i = 0; i < size; i++) {
		qreal sample;
		if(m_xAxisMode) {
			sample = xData[i];
		} else {
			sample = yData[i];
		}
		if(m_max < sample)
			m_max = sample;
		if(m_min > sample)
			m_min = sample;
	}
}

void PlotAutoscaler::setTimeout(int t)
{
	m_timeout = t;
	m_autoScaleTimer->setInterval(t);
}

void PlotAutoscaler::autoscale()
{
	for(PlotChannel *plotCh : qAsConst(m_channels)) {
		auto data = plotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {

			qreal sample;
			if(m_xAxisMode) {
				sample = data->sample(i).x();
			} else {
				sample = data->sample(i).y();
			}
			if(m_max < sample)
				m_max = sample;
			if(m_min > sample)
				m_min = sample;
		}
		qDebug(CAT_TIMEYAUTOSCALE)
			<< "Autoscaling channel " << plotCh->name() << "to (" << m_min << ", " << m_max << ")";
	}

	double minTolerance = m_tolerance * m_min;
	double maxTolerance = m_tolerance * m_max;

	Q_EMIT newMin(m_min - minTolerance);
	Q_EMIT newMax(m_max + maxTolerance);

	m_max = -1000000.0;
	m_min = 1000000.0;
}

void PlotAutoscaler::addChannels(PlotChannel *c)
{
	m_channels.append(c);
	connect(c, &PlotChannel::newData, this, &PlotAutoscaler::onNewData);
}

void PlotAutoscaler::removeChannels(PlotChannel *c)
{
	m_channels.removeAll(c);
	disconnect(c, &PlotChannel::newData, this, &PlotAutoscaler::onNewData);
}

double PlotAutoscaler::tolerance() const { return m_tolerance; }

int PlotAutoscaler::timeout() const { return m_timeout; }

void PlotAutoscaler::setTolerance(double newTolerance)
{
	// tolerance represents a percentage
	m_tolerance = newTolerance / 100;
}

bool PlotAutoscaler::xAxisMode() const { return m_xAxisMode; }

void PlotAutoscaler::setXAxisMode(bool newXAxis) { m_xAxisMode = newXAxis; }

#include "moc_plotautoscaler.cpp"
