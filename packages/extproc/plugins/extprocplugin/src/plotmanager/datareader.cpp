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

#include "plotmanager/datareader.h"
#include <QLoggingCategory>
#include <extprocutils.h>

Q_LOGGING_CATEGORY(CAT_DATA_READER, "DataReader");

using namespace scopy::extprocplugin;

DataReader::DataReader(QObject *parent)
	: QObject(parent)
	, m_data(nullptr)
	, m_dataSize(0)
	, m_channelCount(0)
{}

DataReader::~DataReader() { unmap(); }

bool DataReader::openFile(const QString &path)
{
	m_filePath = path;
	bool isNewFile = false;
	if(!QFile::exists(m_filePath)) {
		createFile(path);
		isNewFile = true;
	}
	m_file.setFileName(path);
	QIODevice::OpenModeFlag mode = isNewFile ? QIODevice::ReadWrite : QIODevice::ReadOnly;
	if(!m_file.open(mode)) {
		if(!isNewFile && !m_file.open(QIODevice::ReadWrite)) {
			qWarning(CAT_DATA_READER) << "Failed to open file:" << path;
			return false;
		}
	}

	double fileSize = m_file.size();
	if(fileSize == 0) {
		qWarning(CAT_DATA_READER) << "Cannot map empty file:" << path;
		m_file.close();
		return false;
	}

	m_dataSize = fileSize;
	m_data = m_file.map(0, m_dataSize);
	if(!m_data) {
		qWarning(CAT_DATA_READER) << "Failed to map file:" << path;
		m_file.close();
		return false;
	}
	m_file.close();
	qDebug(CAT_DATA_READER) << "Successfully opened and mapped file:" << path << "Size:" << m_dataSize;
	return true;
}

void DataReader::unmap()
{
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
	if(m_file.isOpen()) {
		m_file.close();
	}
}

void DataReader::setChannelCount(int count) { m_channelCount = count; }

int DataReader::channelCount() const { return m_channelCount; }

void DataReader::setChannelFormat(const QStringList &format) { m_channelFormat = format; }

QStringList DataReader::channelFormat() const { return m_channelFormat; }

int DataReader::getBytesPerSample() const
{
	int totalBytes = 0;
	for(const QString &format : m_channelFormat) {
		totalBytes += getFormatSize(format);
	}
	return totalBytes;
}

int DataReader::getBytesPerChannel(int channelIndex) const
{
	if(channelIndex < 0 || channelIndex >= m_channelFormat.size()) {
		return 0;
	}
	return getFormatSize(m_channelFormat[channelIndex]);
}

int DataReader::getFormatSize(const QString &format) const
{
	if(format == ChannelFormatTypes::FLOAT32) {
		return sizeof(float);
	}
	if(format == ChannelFormatTypes::FLOAT64) {
		return sizeof(double);
	}
	if(format == ChannelFormatTypes::INT8) {
		return sizeof(int8_t);
	}
	if(format == ChannelFormatTypes::UINT8) {
		return sizeof(uint8_t);
	}
	if(format == ChannelFormatTypes::INT16) {
		return sizeof(int16_t);
	}
	if(format == ChannelFormatTypes::UINT16) {
		return sizeof(uint16_t);
	}
	if(format == ChannelFormatTypes::INT32) {
		return sizeof(int32_t);
	}
	if(format == ChannelFormatTypes::UINT32) {
		return sizeof(uint32_t);
	}
	if(format == ChannelFormatTypes::INT64) {
		return sizeof(int64_t);
	}
	if(format == ChannelFormatTypes::UINT64) {
		return sizeof(uint64_t);
	}

	qWarning(CAT_DATA_READER) << "Unknown format:" << format;
	return 0;
}

void DataReader::readData(int64_t startSample, int64_t sampleCount)
{
	if(!m_data) {
		return;
	}

	if(m_channelCount != m_channelsName.size() || m_channelCount != m_channelFormat.size()) {
		return;
	}

	if(checkForRemapping()) {
		if(!remapFile()) {
			return;
		}
	}

	QMap<QString, QVector<float>> processedData;

	// Initialize vectors for each channel
	for(const QString &ch : qAsConst(m_channelsName)) {
		processedData[ch] = QVector<float>();
		processedData[ch].reserve(sampleCount);
	}

	int bytesPerSample = getBytesPerSample();

	for(int64_t sample = 0; sample < sampleCount; sample++) {
		int64_t sampleOffset = (startSample + sample) * bytesPerSample;

		if(sampleOffset + bytesPerSample > m_dataSize) {
			break;
		}

		int channelOffset = 0;
		for(int ch = 0; ch < m_channelCount; ch++) {
			int channelBytes = getBytesPerChannel(ch);
			QString chName = m_channelsName[ch];
			QByteArray channelData = QByteArray(
				reinterpret_cast<const char *>(m_data + sampleOffset + channelOffset), channelBytes);

			float value = convertToFloat(channelData, m_channelFormat[ch]);
			processedData[chName].append(value);

			channelOffset += channelBytes;
		}
	}
	Q_EMIT dataReady(processedData);
}

bool DataReader::checkForRemapping()
{
	if(m_filePath.isEmpty()) {
		return false;
	}

	QFile file(m_filePath);
	if(!file.exists()) {
		return false;
	}

	int64_t currentSize = file.size();
	return currentSize != m_dataSize;
}

void DataReader::createFile(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly)) {
		qWarning(CAT_DATA_READER) << "Cannot create file:" << path;
		return;
	}

	// Empty files cannot be mapped on windows
	file.write("\0", 1);
	file.flush();
	file.close();

	qDebug(CAT_DATA_READER) << "Created file with initial size:" << path;
}

QStringList DataReader::channelsName() const { return m_channelsName; }

void DataReader::setChannelsName(const QStringList &newChannelsName) { m_channelsName = newChannelsName; }

float DataReader::convertToFloat(const QByteArray &data, const QString &format) const
{
	if(data.isEmpty()) {
		return 0.0;
	}

	const char *ptr = data.constData();

	if(format == ChannelFormatTypes::FLOAT32) {
		return *reinterpret_cast<const float *>(ptr);
	} else if(format == ChannelFormatTypes::INT8) {
		return static_cast<float>(*reinterpret_cast<const int8_t *>(ptr));
	} else if(format == ChannelFormatTypes::UINT8) {
		return static_cast<float>(*reinterpret_cast<const uint8_t *>(ptr));
	} else if(format == ChannelFormatTypes::INT16) {
		return static_cast<float>(*reinterpret_cast<const int16_t *>(ptr));
	} else if(format == ChannelFormatTypes::UINT16) {
		return static_cast<float>(*reinterpret_cast<const uint16_t *>(ptr));
	} else if(format == ChannelFormatTypes::INT32) {
		return static_cast<float>(*reinterpret_cast<const int32_t *>(ptr));
	} else if(format == ChannelFormatTypes::UINT32) {
		return static_cast<float>(*reinterpret_cast<const uint32_t *>(ptr));
	}

	qWarning(CAT_DATA_READER) << "Unknown format for conversion:" << format;
	return 0.0;
}

bool DataReader::remapFile()
{
	if(m_filePath.isEmpty()) {
		return false;
	}

	// Unmap current data
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
	if(m_file.isOpen()) {
		m_file.close();
	}

	// Reopen and remap
	m_file.setFileName(m_filePath);
	if(!m_file.open(QIODevice::ReadWrite)) {
		qWarning(CAT_DATA_READER) << "Failed to reopen file for remapping:" << m_filePath;
		return false;
	}

	double fileSize = m_file.size();

	if(fileSize == 0) {
		qWarning(CAT_DATA_READER) << "Cannot remap empty file:" << m_filePath;
		m_file.close();
		return false;
	}

	m_dataSize = fileSize;
	m_data = m_file.map(0, m_dataSize);

	if(!m_data) {
		qWarning(CAT_DATA_READER) << "Failed to remap file:" << m_filePath;
		m_file.close();
		return false;
	}
	m_file.close();
	qDebug(CAT_DATA_READER) << "Successfully remapped file. New size:" << m_dataSize;

	return true;
}
