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

#include "component/backends/file/regmap/registervaluesfile.h"

#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <cerrno>

using namespace scopy;
using namespace scopy::component::file;

namespace {
Error fileError(const QString &path, const QString &what)
{
	qWarning() << what << path;
	return Error{-EIO, what + " " + path};
}

Result<QStringList> readLines(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return Unexpected(fileError(path, "can't open the register values file"));
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

int hexDigits(const QString &field)
{
	QString hex = field.trimmed();
	if(hex.startsWith("0x", Qt::CaseInsensitive)) {
		hex = hex.mid(2);
	}
	return hex.size();
}

QString formatLine(uint32_t address, uint32_t value, int addrDigits, int valueDigits)
{
	return QString("0x%1,0x%2").arg(address, addrDigits, 16, QChar('0')).arg(value, valueDigits, 16, QChar('0'));
}

// keeps the digit widths of the line it replaces, e.g. 32 bit values stay 8 digits
void setValue(QStringList &lines, uint32_t address, uint32_t value)
{
	for(QString &line : lines) {
		uint32_t lineAddress = 0, lineValue = 0;
		if(parseLine(line, lineAddress, lineValue) && lineAddress == address) {
			const QStringList fields = line.trimmed().split(',');
			line = formatLine(address, value, hexDigits(fields[0]), hexDigits(fields[1]));
			return;
		}
	}
	lines.append(formatLine(address, value, 3, 2));
}
} // namespace

RegisterValuesFile::RegisterValuesFile(const QString &path)
	: m_path(path)
{}

QString RegisterValuesFile::path() const { return m_path; }

Result<uint32_t> RegisterValuesFile::read(uint32_t address) const
{
	auto lines = readLines(m_path);
	if(!lines) {
		return Unexpected(lines.error());
	}
	for(const QString &line : std::as_const(lines.value())) {
		uint32_t lineAddress = 0, lineValue = 0;
		if(parseLine(line, lineAddress, lineValue) && lineAddress == address) {
			return lineValue;
		}
	}
	return Unexpected(Error{-ENOENT, QString("no value for address 0x%1 in %2").arg(address, 0, 16).arg(m_path)});
}

Result<void> RegisterValuesFile::write(uint32_t address, uint32_t value) { return write({{address, value}}); }

Result<void> RegisterValuesFile::write(const QVector<QPair<uint32_t, uint32_t>> &values)
{
	auto read = readLines(m_path);
	if(!read) {
		return Unexpected(read.error());
	}
	QStringList lines = read.value();
	for(const auto &entry : values) {
		setValue(lines, entry.first, entry.second);
	}

	QFile file(m_path);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		return Unexpected(fileError(m_path, "can't write the register values file"));
	}
	QTextStream out(&file);
	for(const QString &line : std::as_const(lines)) {
		out << line << "\n";
	}
	return {};
}

Result<QVector<QPair<uint32_t, uint32_t>>> RegisterValuesFile::readAll() const
{
	auto lines = readLines(m_path);
	if(!lines) {
		return Unexpected(lines.error());
	}
	QVector<QPair<uint32_t, uint32_t>> values;
	for(const QString &line : std::as_const(lines.value())) {
		uint32_t address = 0, value = 0;
		if(parseLine(line, address, value)) {
			values.append({address, value});
		}
	}
	return values;
}
