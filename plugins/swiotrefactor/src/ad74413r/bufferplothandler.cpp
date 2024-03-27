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

#include "ad74413r/bufferplothandler.h"

#include "ad74413r/ad74413r.h"
#include "swiot_logging_categories.h"

#include <QFileDialog>
#include <QGridLayout>

#include <gui/filemanager.h>
#include <unistd.h>

using namespace scopy::swiotrefactor;

BufferPlotHandler::BufferPlotHandler(QWidget *parent, int plotChnlsNo)
	: QWidget(parent)
{
	m_lock = new QMutex();
	m_plotChnlsNo = plotChnlsNo;
}
BufferPlotHandler::~BufferPlotHandler()
{
	if(m_dataPoints.size() > 0) {
		m_dataPoints.clear();
	}
}

// bufferCounter is used only for debug
void BufferPlotHandler::onBufferRefilled(QMap<int, std::vector<double>> bufferData, int bufferCounter)
{
	int plotSampleNumber = m_plotSamplingFreq * m_timespan;
	int bufferDataSize = bufferData.size();
	bool rolling = false;
	m_lock->lock();
	resetDataPoints();
	if(!(m_singleCapture && (m_bufferIndex == m_buffersNumber))) {
		if(bufferDataSize > 0) {
			int currentPlotDataSamplesNumber = 0;
			QList<int> keys = bufferData.keys();
			for(const auto &key : keys) {
				if(m_bufferIndex == m_buffersNumber) {
					int dequeSize = m_dataPointsDeque[key].size();
					if(dequeSize > 0) {
						m_dataPointsDeque[key].pop_front();
						rolling = true;
					}
				} else {
					int lastBuffer = plotSampleNumber % m_bufferSize;
					if((m_bufferIndex == (m_buffersNumber - 1)) && lastBuffer != 0) {
						currentPlotDataSamplesNumber =
							(m_bufferIndex * m_bufferSize) + lastBuffer;
					} else {
						currentPlotDataSamplesNumber = (m_bufferIndex + 1) * m_bufferSize;
					}
				}
				m_dataPointsDeque[key].push_back(bufferData[key]);
			}
		}
		m_bufferIndex = (rolling) ? m_bufferIndex : m_bufferIndex + 1;
		drawPlot();
	}
	if(m_singleCapture && (m_bufferIndex == m_buffersNumber)) {
		Q_EMIT singleCaptureFinished();
	}
	m_lock->unlock();
}

int BufferPlotHandler::getRequiredBuffersNumber() { return m_buffersNumber; }

void BufferPlotHandler::drawPlot()
{
	int plotSampleNumber = m_plotSamplingFreq * m_timespan;
	int dataPointsNumber = m_bufferIndex * m_bufferSize;
	int lastBufferData = plotSampleNumber % m_bufferSize;
	if((m_bufferIndex == m_buffersNumber) && (lastBufferData != 0)) {
		dataPointsNumber = (dataPointsNumber - m_bufferSize) + lastBufferData;
	}
	QList<int> keys = m_dataPointsDeque.keys();
	for(const auto &key : keys) {
		m_dataPoints[key] = std::vector<double>(dataPointsNumber);
		int dequeSize = m_dataPointsDeque[key].size();
		if(dequeSize > 0) {
			for(int j = 0; j < dequeSize; j++) {
				if(j == (m_buffersNumber - 1) && (lastBufferData != 0)) {
					m_dataPoints[key].insert(m_dataPoints[key].begin() + (j * m_bufferSize),
								 m_dataPointsDeque[key][j].begin(),
								 m_dataPointsDeque[key][j].begin() + lastBufferData);
				} else {
					m_dataPoints[key].insert(m_dataPoints[key].begin() + (j * m_bufferSize),
								 m_dataPointsDeque[key][j].begin(),
								 m_dataPointsDeque[key][j].end());
				}
			}
		}
	}
	Q_EMIT bufferDataReady(m_dataPoints);
}

void BufferPlotHandler::onTimespanChanged(double value)
{
	m_timespan = value;
	resetPlotParameters();
}

void BufferPlotHandler::onSamplingFrequencyComputed(double samplingFrequency)
{
	m_plotSamplingFreq = samplingFrequency;
	resetPlotParameters();
}

void BufferPlotHandler::setSingleCapture(bool en) { m_singleCapture = en; }

void BufferPlotHandler::resetPlotParameters()
{
	m_lock->lock();
	auto plotSampleNumber = m_plotSamplingFreq * m_timespan;
	m_bufferSize = (m_plotSamplingFreq > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	m_buffersNumber = (((int)plotSampleNumber % m_bufferSize) == 0) ? (plotSampleNumber / m_bufferSize)
									: ((plotSampleNumber / m_bufferSize) + 1);
	m_bufferIndex = 0;
	resetDeque();

	qDebug(CAT_SWIOT_AD74413R) << "Plot samples number: " << QString::number(plotSampleNumber) << " "
				   << QString::number(m_buffersNumber) + " " +
			QString::number(plotSampleNumber / m_bufferSize) + " ";
	m_lock->unlock();
}

void BufferPlotHandler::resetDataPoints() { m_dataPoints.clear(); }

void BufferPlotHandler::resetDeque()
{
	resetDataPoints();
	for(int i = 0; i < m_dataPointsDeque.size(); i++) {
		m_dataPointsDeque[i].clear();
	}
	m_dataPointsDeque.clear();
	for(int i = 0; i < MAX_CURVES_NUMBER; i++) {
		m_dataPointsDeque.insert(i, std::deque<std::vector<double>>());
	}
}

bool BufferPlotHandler::singleCapture() const { return m_singleCapture; }
