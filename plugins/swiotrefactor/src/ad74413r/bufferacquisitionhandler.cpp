/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "ad74413r/bufferacquisitionhandler.h"

#include "ad74413r/ad74413r.h"
#include "swiot_logging_categories.h"

using namespace scopy::swiotrefactor;

BufferAcquisitionHandler::BufferAcquisitionHandler(QObject *parent)
	: QObject(parent)
{
	m_lock = new QMutex();
}
BufferAcquisitionHandler::~BufferAcquisitionHandler()
{
	if(m_dataPoints.size() > 0) {
		m_dataPoints.clear();
	}
}

// bufferCounter is used only for debug
void BufferAcquisitionHandler::onBufferRefilled(QMap<int, QVector<double>> bufferData, int bufferCounter)
{
	int bufferDataSize = bufferData.size();
	bool rolling = false;
	m_lock->lock();
	if(!(m_singleCapture && (m_bufferIndex == m_buffersNumber))) {
		if(bufferDataSize > 0) {
			int samplingFreq = m_plotSamplingFreq * m_timespan;
			QList<int> keys = bufferData.keys();
			for(const auto &key : keys) {
				m_dataPoints[key].append(bufferData[key]);
				if(m_dataPoints[key].size() > samplingFreq) {
					int unnecessarySamples = m_dataPoints[key].size() - samplingFreq;
					m_dataPoints[key].erase(m_dataPoints[key].begin(),
								m_dataPoints[key].begin() + unnecessarySamples);
				}
				rolling = (m_bufferIndex == m_buffersNumber);
			}
		}
		m_bufferIndex = (rolling) ? m_bufferIndex : m_bufferIndex + 1;
		Q_EMIT bufferDataReady(m_dataPoints);
	}
	if(m_singleCapture && (m_bufferIndex == m_buffersNumber)) {
		Q_EMIT singleCaptureFinished();
	}
	m_lock->unlock();
}

int BufferAcquisitionHandler::getRequiredBuffersNumber() { return m_buffersNumber; }

void BufferAcquisitionHandler::onTimespanChanged(double value)
{
	m_timespan = value;
	resetPlotParameters();
}

void BufferAcquisitionHandler::onSamplingFrequencyComputed(double samplingFrequency)
{
	m_plotSamplingFreq = samplingFrequency;
	resetPlotParameters();
}

void BufferAcquisitionHandler::setSingleCapture(bool en) { m_singleCapture = en; }

void BufferAcquisitionHandler::resetPlotParameters()
{
	m_lock->lock();
	auto plotSampleNumber = m_plotSamplingFreq * m_timespan;
	m_bufferSize = (m_plotSamplingFreq > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	m_buffersNumber = (((int)plotSampleNumber % m_bufferSize) == 0) ? (plotSampleNumber / m_bufferSize)
									: ((plotSampleNumber / m_bufferSize) + 1);
	m_bufferIndex = 0;
	resetDataPoints();

	qDebug(CAT_SWIOT_AD74413R) << "Plot samples number: " << QString::number(plotSampleNumber) << " "
				   << QString::number(m_buffersNumber) + " " +
			QString::number(plotSampleNumber / m_bufferSize) + " ";
	m_lock->unlock();
}

void BufferAcquisitionHandler::resetDataPoints()
{
	const QList<int> &chnls = m_dataPoints.keys();
	for(int chnlIdx : chnls) {
		m_dataPoints[chnlIdx].clear();
	}
	m_dataPoints.clear();
	for(int chnlIdx = 0; chnlIdx < MAX_CURVES_NUMBER; chnlIdx++) {
		m_dataPoints.insert(chnlIdx, QVector<double>());
	}
}

bool BufferAcquisitionHandler::singleCapture() const { return m_singleCapture; }
