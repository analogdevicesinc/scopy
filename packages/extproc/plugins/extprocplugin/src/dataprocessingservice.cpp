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

#include "dataprocessingservice.h"
#include "extprocutils.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DATA_PROCESSING_SERVICE, "DataProcessingService");

using namespace scopy::extprocplugin;

DataProcessingService::DataProcessingService(QObject *parent)
	: QObject(parent)
	, m_fftEnabled(true)
	, m_samplingFreq(0)
{
	m_dataProcessor = new DataProcessor(this);
}

DataProcessingService::~DataProcessingService() {}

void DataProcessingService::setSamplingFreq(double samplingFreq) { m_samplingFreq = samplingFreq; }

void DataProcessingService::setFFTEnabled(bool enabled) { m_fftEnabled = enabled; }

void DataProcessingService::processBufferData(QVector<QVector<float>> bufferData)
{
	if(bufferData.isEmpty()) {
		return;
	}
	int sampleCount = bufferData.first().size();
	if(sampleCount <= 0) {
		return;
	}
	// Compute time axis
	computeXTime(m_samplingFreq, sampleCount);

	// Compute FFT if enabled
	if(m_fftEnabled) {
		computeFFT(bufferData);
	}
}

void DataProcessingService::computeXTime(int samplingFreq, int samples)
{
	QVector<float> xTime;
	for(int i = 0; i < samples; i++) {
		xTime.push_back((float)i / m_samplingFreq);
	}
	DataManager::GetInstance()->registerData(DataManagerKeys::TIME, xTime);
}

void DataProcessingService::computeFFT(QVector<QVector<float>> bufferData)
{
	if(bufferData.size() == 2) {
		// Complex FFT for I/Q data
		FFTResult fftResult = m_dataProcessor->computeComplexFFT(bufferData[0], bufferData[1], m_samplingFreq);
		DataManager::GetInstance()->registerData(DataManagerKeys::FFT_FREQUENCY, fftResult.frequency);
		DataManager::GetInstance()->registerData(DataManagerKeys::FFT_MAGNITUDE_DB, fftResult.magnitudeDB);
	} else {
		// Real FFT for each channel
		for(int chIdx = 0; chIdx < bufferData.size(); chIdx++) {
			FFTResult fftResult = m_dataProcessor->computeFFT(bufferData[chIdx], m_samplingFreq);
			QString channelSuffix = QString::number(chIdx);
			DataManager::GetInstance()->registerData(DataManagerKeys::FFT_FREQUENCY + channelSuffix,
								 fftResult.frequency);
			DataManager::GetInstance()->registerData(DataManagerKeys::FFT_MAGNITUDE_DB + channelSuffix,
								 fftResult.magnitudeDB);
		}
	}
}
