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

#include <readwrite/csvregisterstrategy.hpp>

#include "../logging_categories.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

using namespace scopy::regmap;

namespace {
QStringList readLines(const QString &path, bool *ok)
{
	QFile file(path);
	*ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!*ok) {
		qWarning(CAT_REGMAP) << "Can't open" << path << file.errorString();
		return {};
	}
	QStringList lines;
	QTextStream in(&file);
	while(!in.atEnd()) {
		lines.append(in.readLine());
	}
	return lines;
}

// parses "<address>,<value>" in hex; comments and malformed lines give false
bool parseLine(const QString &line, uint32_t &address, uint32_t &value)
{
	const QString trimmed = line.trimmed();
	if(trimmed.isEmpty() || trimmed.startsWith('#')) {
		return false;
	}
	const QStringList fields = trimmed.split(',');
	if(fields.size() < 2) {
		return false;
	}
	bool addrOk = false, valOk = false;
	address = fields[0].trimmed().toUInt(&addrOk, 16);
	value = fields[1].trimmed().toUInt(&valOk, 16);
	return addrOk && valOk;
}
} // namespace

CsvRegisterFile::CsvRegisterFile(const QString &path)
	: m_path(path)
{}

QString CsvRegisterFile::path() const { return m_path; }

bool CsvRegisterFile::read(uint32_t address, uint32_t &value) const
{
	bool ok = false;
	const QStringList lines = readLines(m_path, &ok);
	for(const QString &line : lines) {
		uint32_t lineAddress = 0, lineValue = 0;
		if(parseLine(line, lineAddress, lineValue) && lineAddress == address) {
			value = lineValue;
			return true;
		}
	}
	return false;
}

bool CsvRegisterFile::write(uint32_t address, uint32_t value)
{
	bool ok = false;
	QStringList lines = readLines(m_path, &ok);
	if(!ok) {
		return false;
	}

	// keeps the digit widths of the line it replaces, e.g. 32 bit values stay 8 digits
	auto formatLine = [address, value](int addrDigits, int valueDigits) {
		return QString("0x%1,0x%2")
			.arg(address, addrDigits, 16, QChar('0'))
			.arg(value, valueDigits, 16, QChar('0'));
	};
	auto digits = [](const QString &field) {
		QString hex = field.trimmed();
		if(hex.startsWith("0x", Qt::CaseInsensitive)) {
			hex = hex.mid(2);
		}
		return hex.size();
	};

	bool replaced = false;
	for(QString &line : lines) {
		uint32_t lineAddress = 0, lineValue = 0;
		if(parseLine(line, lineAddress, lineValue) && lineAddress == address) {
			const QStringList fields = line.trimmed().split(',');
			line = formatLine(digits(fields[0]), digits(fields[1]));
			replaced = true;
			break;
		}
	}
	if(!replaced) {
		lines.append(formatLine(3, 2));
	}

	QFile file(m_path);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		qWarning(CAT_REGMAP) << "Can't write" << m_path << file.errorString();
		return false;
	}
	QTextStream out(&file);
	for(const QString &line : std::as_const(lines)) {
		out << line << "\n";
	}
	return true;
}

CsvRegisterReadStrategy::CsvRegisterReadStrategy(QSharedPointer<CsvRegisterFile> file)
	: m_file(file)
{}

void CsvRegisterReadStrategy::read(uint32_t address)
{
	uint32_t value = 0;
	if(!m_file->read(address, value)) {
		qWarning(CAT_REGMAP) << "No value for address" << Qt::hex << Qt::showbase << address << "in"
				     << m_file->path();
		// the receiver copies the text right away (direct connection)
		const QByteArray err =
			QString("no value for address 0x%1 in the register values file").arg(address, 0, 16).toUtf8();
		Q_EMIT readError(err.constData());
		return;
	}
	Q_EMIT readDone(address, value);
}

CsvRegisterWriteStrategy::CsvRegisterWriteStrategy(QSharedPointer<CsvRegisterFile> file)
	: m_file(file)
{}

void CsvRegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	if(!m_file->write(address, val)) {
		Q_EMIT writeError("can't write the register values file");
		return;
	}
	Q_EMIT writeSuccess(address);
}
