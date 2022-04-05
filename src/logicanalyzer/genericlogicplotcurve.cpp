/*
 * Copyright (c) 2020 Analog Devices Inc.
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


#include "genericlogicplotcurve.h"


GenericLogicPlotCurve::GenericLogicPlotCurve(const QString &name, const QString &id, LogicPlotCurveType type, double pixelOffset,
					     double traceHeight, double sampleRate,
					     double timeTriggerOffset, uint64_t bufferSize):
	QObject(),
	QwtPlotCurve(),
	m_name(name),
	m_id(id),
	m_pixelOffset(pixelOffset),
	m_traceHeight(traceHeight),
	m_traceColor("dodgerblue"),
	m_sampleRate(sampleRate),
	m_timeTriggerOffset(timeTriggerOffset),
	m_bufferSize(bufferSize),
	m_type(type)
{
}

QString GenericLogicPlotCurve::getName() const
{
	return m_name;
}

QString GenericLogicPlotCurve::getId() const
{
	return m_id;
}

QColor GenericLogicPlotCurve::getTraceColor() const
{
	return m_traceColor;
}


double GenericLogicPlotCurve::getPixelOffset() const
{
	return m_pixelOffset;
}

double GenericLogicPlotCurve::getTraceHeight() const
{
	return m_traceHeight;
}

double GenericLogicPlotCurve::getSampleRate() const
{
	return m_sampleRate;
}

double GenericLogicPlotCurve::getTimeTriggerOffset() const
{
	return m_timeTriggerOffset;
}

uint64_t GenericLogicPlotCurve::getBufferSize() const
{
	return m_bufferSize;
}

LogicPlotCurveType GenericLogicPlotCurve::getType() const
{
	return m_type;
}

void GenericLogicPlotCurve::setName(const QString &name)
{
	if (m_name != name) {
		m_name = name;
		Q_EMIT nameChanged(name);
	}
}

void GenericLogicPlotCurve::setId(const QString &id)
{
	if (m_id != id) {
		m_id = id;
	}
}

void GenericLogicPlotCurve::setPixelOffset(double pixelOffset)
{
	if (m_pixelOffset != pixelOffset) {
		m_pixelOffset = pixelOffset;

		setBaseline(m_pixelOffset + m_traceHeight);

		Q_EMIT pixelOffsetChanged(m_pixelOffset);
	}
}

void GenericLogicPlotCurve::setTraceHeight(double traceHeight)
{
	if (m_traceHeight != traceHeight) {
		m_traceHeight = traceHeight;

		setBaseline(m_pixelOffset + m_traceHeight);
	}
}

void GenericLogicPlotCurve::setTraceColor(const QColor traceColor)
{
	m_traceColor = traceColor;
}

void GenericLogicPlotCurve::setSampleRate(double sampleRate)
{
	if (m_sampleRate != sampleRate) {
		m_sampleRate = sampleRate;
	}
}

void GenericLogicPlotCurve::setTimeTriggerOffset(double timeTriggerOffset)
{
	if (m_timeTriggerOffset != timeTriggerOffset) {
		m_timeTriggerOffset = timeTriggerOffset;
	}
}

void GenericLogicPlotCurve::setBufferSize(uint64_t bufferSize)
{
	if (m_bufferSize != bufferSize) {
		m_bufferSize = bufferSize;
	}
}

uint64_t GenericLogicPlotCurve::fromTimeToSample(double time) const
{
	const double totalTime = static_cast<double>(m_bufferSize) / m_sampleRate;
	const double tmin = (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double tmax = totalTime + (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double smin = 0;
	const double smax = m_bufferSize;

	if (time > tmax) {
		time = tmax;
	}

	if (time < tmin) {
		time = tmin;
	}

	return (time - tmin) / (tmax - tmin) * (smax - smin) + smin;
}

double GenericLogicPlotCurve::fromSampleToTime(uint64_t sample) const
{
	const double totalTime = static_cast<double>(m_bufferSize) / m_sampleRate;
	const double tmin = (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double tmax = totalTime + (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double smin = 0;
	const double smax = m_bufferSize;

	if (sample > smax) {
		sample = smax;
	}

	return (sample - smin) / (smax - smin) * (tmax - tmin) + tmin;
}
