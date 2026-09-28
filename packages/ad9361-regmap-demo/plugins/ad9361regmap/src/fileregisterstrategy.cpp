/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "fileregisterstrategy.h"

#include <QFile>
#include <QLoggingCategory>
#include <QTextStream>

Q_LOGGING_CATEGORY(CAT_AD9361REGMAP_FILE, "Ad9361RegmapFile")

using namespace scopy::ad9361regmap;

RegisterValuesFile::RegisterValuesFile(const QString &path)
	: m_path(path)
{}

QString RegisterValuesFile::path() const { return m_path; }

bool RegisterValuesFile::load()
{
	QFile file(m_path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning(CAT_AD9361REGMAP_FILE) << "Can't open" << m_path << file.errorString();
		return false;
	}

	m_values.clear();
	QTextStream in(&file);
	while(!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if(line.isEmpty() || line.startsWith('#')) {
			continue;
		}
		QStringList fields = line.split(',');
		if(fields.size() < 2) {
			continue;
		}
		bool addrOk = false, valOk = false;
		uint32_t address = fields[0].trimmed().toUInt(&addrOk, 16);
		uint32_t value = fields[1].trimmed().toUInt(&valOk, 16);
		if(addrOk && valOk) {
			m_values.insert(address, value);
		}
	}
	return true;
}

bool RegisterValuesFile::save() const
{
	QFile file(m_path);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		qWarning(CAT_AD9361REGMAP_FILE) << "Can't write" << m_path << file.errorString();
		return false;
	}

	QTextStream out(&file);
	out << "# ad9361-regmap-demo register values\n";
	out << "# format: <address>,<value> (hex). Read and written by the demo package instead of the device.\n";
	for(auto it = m_values.cbegin(); it != m_values.cend(); ++it) {
		out << QString("0x%1,0x%2\n").arg(it.key(), 3, 16, QChar('0')).arg(it.value(), 2, 16, QChar('0'));
	}
	return true;
}

bool RegisterValuesFile::read(uint32_t address, uint32_t &value)
{
	if(!load() || !m_values.contains(address)) {
		return false;
	}
	value = m_values.value(address);
	return true;
}

bool RegisterValuesFile::write(uint32_t address, uint32_t value)
{
	load();
	m_values.insert(address, value);
	return save();
}

FileRegisterReadStrategy::FileRegisterReadStrategy(QSharedPointer<RegisterValuesFile> file)
	: m_file(file)
{}

void FileRegisterReadStrategy::read(uint32_t address)
{
	uint32_t value = 0;
	if(!m_file->read(address, value)) {
		qWarning(CAT_AD9361REGMAP_FILE)
			<< "No value for address" << Qt::hex << address << "in" << m_file->path();
		Q_EMIT readError("address not found in values file");
		return;
	}
	qDebug(CAT_AD9361REGMAP_FILE) << "read" << Qt::hex << address << "=" << value;
	Q_EMIT readDone(address, value);
}

FileRegisterWriteStrategy::FileRegisterWriteStrategy(QSharedPointer<RegisterValuesFile> file)
	: m_file(file)
{}

void FileRegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	if(!m_file->write(address, val)) {
		Q_EMIT writeError("can't write values file");
		return;
	}
	qDebug(CAT_AD9361REGMAP_FILE) << "write" << Qt::hex << address << "=" << val;
	Q_EMIT writeSuccess(address);
}
