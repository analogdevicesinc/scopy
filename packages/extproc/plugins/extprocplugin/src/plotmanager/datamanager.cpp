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

DataManager::DataManager(QObject *parent)
	: QObject(parent)
{
	m_dataReader = new DataReader(this);
	m_dataReader->openFile(ExtProcUtils::dataOutPath());
	setupConnections();
}

void DataManager::setupConnections()
{
	connect(m_dataReader, &DataReader::dataReady, this, &DataManager::onDataReady);
	connect(&m_plotsData, &DataManagerMap::keysChanged, this, &DataManager::newDataEntries);
}

DataManager::~DataManager() {}

void DataManager::config(const QStringList &chnlsName, const QStringList &chnlsFormat, const int channelCount)
{
	m_dataReader->setChannelsName(chnlsName);
	m_dataReader->setChannelFormat(chnlsFormat);
	m_dataReader->setChannelCount(channelCount);
}

void DataManager::onConfigAnalysis(const QString &type, const QVariantMap &config, const OutputInfo &info)
{
	if(m_dataReader->channelCount() == 0) {
		m_dataReader->setChannelCount(info.channelCount());
	}
	if(m_dataReader->channelFormat().isEmpty()) {
		m_dataReader->setChannelFormat(info.channelFormat());
	}
	if(m_dataReader->channelsName().isEmpty()) {
		m_dataReader->setChannelsName(info.channelNames());
	}
}

void DataManager::readData(int64_t startSample, int64_t sampleCount)
{
	m_dataReader->readData(startSample, sampleCount);
}

void DataManager::setSamplingFreq(int samplingFreq) { m_samplingFreq = samplingFreq; }

QVector<float> DataManager::dataForKey(const QString &key) { return m_plotsData.get(key, {}); }

void DataManager::onDataReady(QMap<QString, QVector<float>> &data)
{
	for(auto it = data.begin(); it != data.end(); it++) {
		m_plotsData.insert(it.key(), it.value());
	}
	Q_EMIT dataIsReady();
}

void DataManager::onInputData(QVector<QVector<float>> bufferData)
{
	int sampleCount = 0;
	for(int chIdx = 0; chIdx < bufferData.size(); chIdx++) {
		QString inName = DataManagerKeys::INPUT + QString::number(chIdx);
		m_plotsData.insert(inName, bufferData[chIdx]);
		if(sampleCount == 0) {
			sampleCount = bufferData[chIdx].size();
		}
	}
	if(sampleCount != m_sampleCount) {
		m_sampleCount = sampleCount;
		computeXTime(m_samplingFreq, sampleCount);
	}
}

void DataManager::computeXTime(int samplingFreq, int samples)
{
	QVector<float> xTime;
	for(int i = 0; i < samples; i++) {
		xTime.push_back((float)i / m_samplingFreq);
	}
	m_plotsData.insert(DataManagerKeys::TIME, xTime);
}

double DataManager::samplingFreq() const { return m_samplingFreq; }

double DataManager::sampleCount() const { return m_sampleCount; }
