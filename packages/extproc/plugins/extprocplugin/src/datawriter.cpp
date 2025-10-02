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

#include "datawriter.h"
#include <QLoggingCategory>
#include <QDataStream>

Q_LOGGING_CATEGORY(CAT_DATA_WRITER, "DataWriter");

using namespace scopy::extprocplugin;

DataWriter::DataWriter(QObject *parent)
	: QObject(parent)
{}

DataWriter::DataWriter(bool withHeader, QObject *parent)
	: QObject(parent)
	, m_withHeader(withHeader)
{}

DataWriter::~DataWriter() { unmap(); }

uchar *DataWriter::mappedData() { return m_mappedDataOnly; }

bool DataWriter::openFile(const QString &path, int64_t dataSize)
{
	unmap();
	m_file.setFileName(path);
	if(!m_file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		return false;
	}

	m_dataSize = dataSize;
	int64_t headerSize = m_withHeader ? getHeaderSize() : 0;
	m_totalFileSize = headerSize + dataSize;
	// Resize file to accommodate header + data
	if(m_file.size() < m_totalFileSize) {
		if(!m_file.resize(m_totalFileSize)) {
			m_file.close();
			return false;
		}
	}

	// Write header if enabled
	if(m_withHeader && !writeHeader()) {
		m_file.close();
		return false;
	}

	// Memory map the entire file
	m_mappedFileData = m_file.map(0, m_totalFileSize);
	if(!m_mappedFileData) {
		m_file.close();
		return false;
	}

	// Point to data section (after header)
	m_mappedDataOnly = m_mappedFileData + headerSize;

	m_file.close();
	return true;
}

void DataWriter::unmap()
{
	if(m_mappedFileData) {
		m_file.unmap(m_mappedFileData);
		m_mappedFileData = nullptr;
		m_mappedDataOnly = nullptr;
	}
	m_file.close();
}

QFileInfo DataWriter::getFileInfo() { return QFileInfo(m_file); }

void DataWriter::setWithHeader(bool withHeader) { m_withHeader = withHeader; }

bool DataWriter::hasHeader() const { return m_withHeader; }

void DataWriter::setIQHeader(const IQBinHeader &header) { m_iqHeader = header; }

IQBinHeader DataWriter::getIQHeader() const { return m_iqHeader; }

bool DataWriter::writeHeader()
{
	if(!m_file.isOpen()) {
		return false;
	}

	m_file.seek(0);
	QDataStream stream(&m_file);
	stream.setByteOrder(QDataStream::LittleEndian);

	stream << m_iqHeader.version;
	stream << m_iqHeader.num_points;
	stream << m_iqHeader.sample_rate;
	stream << m_iqHeader.start_time;
	stream << m_iqHeader.center_freq;

	if(stream.status() != QDataStream::Ok) {
		qWarning(CAT_DATA_WRITER) << "Failed to write IQ header";
		return false;
	}

	// Fill remaining header space with zeros for version 2
	if(m_iqHeader.version == 2) {
		int64_t currentPos = m_file.pos();
		int64_t remainingBytes = getHeaderSize() - currentPos;
		if(remainingBytes > 0) {
			QByteArray padding(remainingBytes, 0);
			m_file.write(padding);
		}
	}

	return true;
}

int64_t DataWriter::getHeaderSize() const
{
	if(!m_withHeader) {
		return 0;
	}

	int64_t baseHeaderSize = sizeof(IQBinHeader);
	if(m_iqHeader.version == 2) {
		return baseHeaderSize + HEADER_EXTRA_V2_SIZE;
	}
	return baseHeaderSize;
}
