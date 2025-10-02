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

#include "iqbinreader.h"
#include <QLoggingCategory>
#include <QDataStream>
#include <qtconcurrentrun.h>

Q_LOGGING_CATEGORY(CAT_IQBIN_READER, "IQBinReader");

using namespace scopy::extprocplugin;

IQBinReader::IQBinReader(QObject *parent)
	: QObject(parent)
{
	m_params = {1024, "name", {"ch0", "ch1"}};
	m_dataReader = new DataReader(this);
	m_dataWriter = new DataWriter(this);
	m_readFw = new QFutureWatcher<void>();

	connect(m_readFw, &QFutureWatcher<void>::finished, this, [this]() {
		if(!m_bufferData.isEmpty()) {
			Q_EMIT dataReady(m_bufferData);
		}
	});
}

IQBinReader::~IQBinReader()
{
	closeFile();
	delete m_readFw;
}

bool IQBinReader::openFile(const QString &filePath)
{
	closeFile();

	// Use DataReader to handle file opening and header detection automatically
	if(!m_dataReader->openFile(filePath)) {
		qWarning(CAT_IQBIN_READER) << "Failed to open file:" << filePath;
		return false;
	}

	// Verify it has IQ header
	if(!m_dataReader->hasIQHeader()) {
		qWarning(CAT_IQBIN_READER) << "File does not contain valid IQ header:" << filePath;
		closeFile();
		return false;
	}

	m_isFileOpen = true;
	m_currentPosition = 0;

	IQBinHeader header = m_dataReader->getIQHeader();
	qInfo(CAT_IQBIN_READER) << "Successfully opened iqbin file:" << filePath;
	qInfo(CAT_IQBIN_READER) << "Version:" << header.version << "Samples:" << header.num_points
				<< "Sample rate:" << header.sample_rate << "Center freq:" << header.center_freq;

	return true;
}

void IQBinReader::closeFile()
{
	if(m_readFw->isRunning()) {
		m_readFw->waitForFinished();
	}

	m_dataReader->unmap();
	m_dataWriter->unmap();

	m_isFileOpen = false;
	m_currentPosition = 0;
}

bool IQBinReader::isFileOpen() const { return m_isFileOpen; }

void IQBinReader::startAcq(bool en)
{
	if(m_readFw->isRunning()) {
		m_readFw->waitForFinished();
	}
	m_readFw->setPaused(!en);

	if(en && m_isFileOpen) {
		onDataRequest();
	}
}

void IQBinReader::onDataRequest()
{
	if(!m_readFw->isRunning() && !m_readFw->isPaused() && m_isFileOpen) {
		QFuture<void> f = QtConcurrent::run(this, &IQBinReader::readData);
		m_readFw->setFuture(f);
	}
}

void IQBinReader::onBufferParamsChanged(const BufferParams &params)
{
	m_params = params;
	if(m_isFileOpen) {
		int64_t bytesToWrite = m_params.samplesCount * 2 * sizeof(float);
		if(!m_dataWriter->openFile(QIQUtils::dataInPath(), bytesToWrite)) {
			qWarning(CAT_IQBIN_READER) << "Failed to open DataWriter file for mapping";
		}
	}
	InputConfig config = createInputConfig();
	Q_EMIT inputFormatChanged(config);
}

void IQBinReader::readData()
{
	if(!m_isFileOpen) {
		return;
	}

	int samplesPerChannel = m_params.samplesCount;
	if(samplesPerChannel <= 0) {
		samplesPerChannel = 1024;
	}

	IQBinHeader header = m_dataReader->getIQHeader();
	int64_t samplesRemaining = header.num_points - m_currentPosition;
	int samplesToRead = qMin(static_cast<int64_t>(samplesPerChannel), samplesRemaining);

	if(samplesToRead <= 0) {
		m_currentPosition = 0;
		samplesToRead = qMin(static_cast<int64_t>(samplesPerChannel), static_cast<int64_t>(header.num_points));
	}

	int64_t bytesToRead = samplesToRead * 2 * sizeof(float);

	// Direct memory access - much more efficient!
	int64_t dataOffset = m_currentPosition * 2 * sizeof(float);
	const uchar *sourceData = m_dataReader->mappedData() + dataOffset;
	uchar *mappedMemory = m_dataWriter->mappedData();
	if(!mappedMemory) {
		qWarning(CAT_IQBIN_READER) << "DataWriter not properly initialized!";
		return;
	}

	if(mappedMemory && sourceData) {
		memcpy(mappedMemory, sourceData, bytesToRead);
	}

	m_bufferData = convertIQData(sourceData, samplesToRead);
	m_currentPosition += samplesToRead;
}

InputConfig IQBinReader::createInputConfig()
{
	IQBinHeader header = m_dataReader->getIQHeader();

	InputConfig config;
	config.setInputFile(QIQUtils::dataInPath());
	config.setInputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	config.setChannelCount(2);
	config.setSampleCount(m_params.samplesCount);
	config.setSamplingFrequency(header.sample_rate);
	config.setFrequencyOffset(header.center_freq);

	QStringList formats;
	formats << ChannelFormatTypes::FLOAT32;
	formats << ChannelFormatTypes::FLOAT32;
	config.setChnlsFormat(formats);

	return config;
}

QVector<QVector<float>> IQBinReader::convertIQData(const uchar *data, int numSamples)
{
	QVector<QVector<float>> result;
	QVector<float> iData, qData;

	iData.reserve(numSamples);
	qData.reserve(numSamples);

	const float *floatData = reinterpret_cast<const float *>(data);

	for(int i = 0; i < numSamples; ++i) {
		iData.append(floatData[i * 2]);
		qData.append(floatData[i * 2 + 1]);
	}

	result.append(iData);
	result.append(qData);

	return result;
}
