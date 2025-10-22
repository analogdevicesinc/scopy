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

#include "plotmanager/datamanager.h"
#include "extprocutils.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DATA_MANAGER, "DataManager");

using namespace scopy::extprocplugin;

DataManager *DataManager::pinstance_ = nullptr;

DataManager *DataManager::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new DataManager();
	}
	return pinstance_;
}

DataManager::DataManager(QObject *parent)
	: QObject(parent)
	, m_samplingFreq(0)
	, m_sampleCount(0)
	, m_computeFFT(true)
{
	m_dataProcessor = new DataProcessor(this);
	setupConnections();
}

void DataManager::setupConnections()
{
	connect(&m_plotsData, &DataManagerMap::keysChanged, this, &DataManager::newDataEntries);
}

DataManager::~DataManager() {}

void DataManager::registerData(const QString &key, const QVector<float> &data)
{
	m_plotsData.insert(key, data);
	Q_EMIT dataIsReady(QStringList() << key);
}

void DataManager::registerData(const QMap<QString, QVector<float>> &data)
{
	QStringList keys;
	for(auto it = data.begin(); it != data.end(); ++it) {
		m_plotsData.insert(it.key(), it.value());
		keys << it.key();
	}
	Q_EMIT dataIsReady(keys);
}

void DataManager::setSamplingFreq(int samplingFreq) { m_samplingFreq = samplingFreq; }

void DataManager::setSampleCount(int sampleCount) { m_sampleCount = sampleCount; }

QVector<float> DataManager::dataForKey(const QString &key) { return m_plotsData.get(key, {}); }

void DataManager::clearData()
{
	computeFFT(bufferData);
	m_plotsData.clear();
	m_sampleCount = 0;
}

bool DataManager::hasDataForKey(const QString &key) const { return m_plotsData.contains(key); }
void DataManager::onFftEnabled(bool en) { m_computeFFT = en; }

QStringList DataManager::availableKeys() const { return QStringList(); }

void DataManager::computeFFT(QVector<QVector<float>> bufferData)
{
	if(!m_computeFFT) {
		return;
	}
	if(bufferData.size() == 2) {
		FFTResult fftResult = m_dataProcessor->computeComplexFFT(bufferData[0], bufferData[1], m_samplingFreq);
		m_plotsData.insert(DataManagerKeys::FFT_FREQUENCY, fftResult.frequency);
		m_plotsData.insert(DataManagerKeys::FFT_MAGNITUDE_DB, fftResult.magnitudeDB);
	} else {
		for(int chIdx = 0; chIdx < bufferData.size(); chIdx++) {
			FFTResult fftResult = m_dataProcessor->computeFFT(bufferData[chIdx], m_samplingFreq);
			QString channelSuffix = QString::number(chIdx);
			m_plotsData.insert(DataManagerKeys::FFT_FREQUENCY + channelSuffix, fftResult.frequency);
			m_plotsData.insert(DataManagerKeys::FFT_MAGNITUDE_DB + channelSuffix, fftResult.magnitudeDB);
		}
	}
}

double DataManager::samplingFreq() const { return m_samplingFreq; }

double DataManager::sampleCount() const { return m_sampleCount; }
