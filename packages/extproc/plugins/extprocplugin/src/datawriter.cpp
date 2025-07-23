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

Q_LOGGING_CATEGORY(CAT_DATA_WRITER, "DataWriter");

using namespace scopy::extprocplugin;

DataWriter::DataWriter(QObject *parent)
	: QObject(parent)
{}

DataWriter::~DataWriter() { unmap(); }

uchar *DataWriter::mappedData() { return m_data; }

bool DataWriter::openFile(const QString &path, int64_t dataSize)
{
	m_file.setFileName(path);
	if(!m_file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		return false;
	}
	if(m_file.size() < dataSize) {
		if(!m_file.resize(dataSize)) {
			m_file.close();
			return false;
		}
	}
	m_dataSize = dataSize;
	m_data = m_file.map(0, m_dataSize);
	if(!m_data) {
		return false;
	}
	m_file.close();
	return true;
}

void DataWriter::unmap()
{
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
	m_file.close();
}
