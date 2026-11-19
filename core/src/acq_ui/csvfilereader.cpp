/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "csvfilereader.h"

#include <gui/filemanager.h>

#include <algorithm>
#include <vector>

#include <QFile>
#include <QFileInfo>

namespace scopy {
namespace adc {

namespace {

constexpr qint64 MAX_LINE_BYTES = 1 << 18;
constexpr int PROBE_ROWS = 1000;
constexpr int SCOPY_HEADER_LINES = 8;
constexpr int SCOPY_RATE_ROW = 4;
constexpr int SCOPY_NAMES_ROW = 7;

QByteArrayView chomp(QByteArrayView line)
{
	while(!line.isEmpty() && (line.back() == '\n' || line.back() == '\r')) {
		line.chop(1);
	}
	return line;
}

bool isComment(QByteArrayView line) { return !line.isEmpty() && line.front() == ';'; }

template <typename Sink>
void forEachCell(QByteArrayView line, char sep, Sink sink)
{
	qsizetype start = 0;
	int col = 0;
	for(qsizetype i = 0; i <= line.size(); ++i) {
		if(i == line.size() || line[i] == sep) {
			sink(col++, line.sliced(start, i - start).trimmed());
			start = i + 1;
		}
	}
}

QStringList splitRow(QByteArrayView line, char sep)
{
	QStringList out;
	forEachCell(line, sep, [&out](int, QByteArrayView cell) { out.append(QString::fromUtf8(cell)); });
	return out;
}

bool isNumericRow(QByteArrayView line, char sep)
{
	bool numeric = true;
	forEachCell(line, sep, [&numeric](int, QByteArrayView cell) {
		bool ok = false;
		(void)cell.toFloat(&ok);
		numeric = numeric && (ok || cell.isEmpty());
	});
	return numeric;
}

int countCells(QByteArrayView line, char sep)
{
	int count = 0;
	forEachCell(line, sep, [&count](int col, QByteArrayView) { count = col + 1; });
	return count;
}

bool isStrictlyIncreasing(const QVector<float> &v)
{
	const qsizetype n = std::min<qsizetype>(v.size(), PROBE_ROWS);
	for(qsizetype i = 1; i < n; ++i) {
		if(v[i] <= v[i - 1]) {
			return false;
		}
	}
	return n >= 2;
}

double medianStep(const QVector<float> &v)
{
	const qsizetype n = std::min<qsizetype>(v.size(), PROBE_ROWS);
	if(n < 2) {
		return 0.0;
	}
	std::vector<double> diffs;
	diffs.reserve(n - 1);
	for(qsizetype i = 1; i < n; ++i) {
		diffs.push_back(double(v[i]) - double(v[i - 1]));
	}
	const std::size_t mid = diffs.size() / 2;
	std::nth_element(diffs.begin(), diffs.begin() + mid, diffs.end());
	return diffs[mid];
}

} // namespace

bool CsvFileReader::open(const QString &path, QString *err)
{
	const QString name = QFileInfo(path).fileName();
	const auto fail = [err](const QString &reason) {
		if(err) {
			*err = reason;
		}
		return false;
	};

	QFile f(path);
	if(!f.open(QIODevice::ReadOnly)) {
		return fail(QStringLiteral("Cannot open %1: %2").arg(path, f.errorString()));
	}

	const char sep = (QFileInfo(path).suffix().toLower() == QStringLiteral("txt")) ? '\t' : ',';

	m_columns.clear();
	m_names.clear();
	m_malformed = 0;
	m_scopyHeader = false;
	m_declaredRate = 0.0;
	m_effectiveRate = 0.0;
	m_timeIsFirst = false;

	QByteArray buf(MAX_LINE_BYTES, Qt::Uninitialized);
	qint64 len = 0;
	const auto readLine = [&] { return (len = f.readLine(buf.data(), buf.size())) > 0; };
	const auto currentLine = [&] { return chomp(QByteArrayView(buf.constData(), len)); };

	QList<QStringList> head;
	while(head.size() < SCOPY_HEADER_LINES && readLine()) {
		head.append(splitRow(currentLine(), sep));
	}
	if(head.isEmpty()) {
		return fail(QStringLiteral("%1 is empty").arg(name));
	}

	QVector<QVector<QString>> probe;
	for(const QStringList &row : head) {
		probe.append(QVector<QString>(row.cbegin(), row.cend()));
	}
	m_scopyHeader = head.size() == SCOPY_HEADER_LINES && ScopyFileHeader::hasValidHeader(probe);

	int skipLines = 0;
	if(m_scopyHeader) {
		skipLines = SCOPY_HEADER_LINES;
		m_names = head[SCOPY_NAMES_ROW];
		if(head[SCOPY_RATE_ROW].size() > 1) {
			bool ok = false;
			const double sr = head[SCOPY_RATE_ROW][1].toDouble(&ok);
			m_declaredRate = (ok && sr > 0.0) ? sr : 0.0;
		}
	} else {
		if(!f.seek(0)) {
			return fail(QStringLiteral("Cannot rewind %1").arg(name));
		}
		while(readLine()) {
			++skipLines;
			const QByteArrayView line = currentLine();
			if(line.isEmpty() || isComment(line)) {
				continue;
			}
			if(!isNumericRow(line, sep)) {
				m_names = splitRow(line, sep);
			} else {
				--skipLines;
			}
			break;
		}
	}

	if(!f.seek(0)) {
		return fail(QStringLiteral("Cannot rewind %1").arg(name));
	}
	for(int i = 0; i < skipLines && readLine(); ++i) {
	}

	const qint64 dataStart = f.pos();
	int columnCount = 0;

	while(readLine()) {
		const QByteArrayView line = currentLine();
		if(line.isEmpty() || isComment(line)) {
			continue;
		}

		if(columnCount == 0) {
			columnCount = countCells(line, sep);
			const qsizetype estimate = (f.size() - dataStart) / std::max<qint64>(len, 1) + 1;
			for(int c = 0; c < columnCount; ++c) {
				m_columns.append(QVector<float>());
				m_columns.last().reserve(estimate);
			}
		}

		int seen = 0;
		forEachCell(line, sep, [&](int col, QByteArrayView cell) {
			if(col >= columnCount) {
				return;
			}
			seen = col + 1;
			bool ok = false;
			const float v = cell.toFloat(&ok);
			if(!ok && !cell.isEmpty()) {
				++m_malformed;
			}
			m_columns[col].append(ok ? v : 0.0f);
		});
		for(int col = seen; col < columnCount; ++col) {
			m_columns[col].append(0.0f);
		}
	}

	if(m_columns.isEmpty() || m_columns.first().isEmpty()) {
		return fail(QStringLiteral("No numeric data found in %1").arg(name));
	}

	while(m_names.size() < m_columns.size()) {
		m_names.append(QStringLiteral("ch%1").arg(m_names.size()));
	}
	m_names = m_names.mid(0, m_columns.size());

	resolveFirstColumn();
	return true;
}

void CsvFileReader::setFirstColumnMode(FirstColumnMode mode)
{
	if(m_mode != mode) {
		m_mode = mode;
		resolveFirstColumn();
	}
}

void CsvFileReader::resolveFirstColumn()
{
	const bool canDrop = m_columns.size() >= 2;

	switch(m_mode) {
	case FirstColumnMode::Data:
		m_timeIsFirst = false;
		break;
	case FirstColumnMode::Time:
		m_timeIsFirst = canDrop;
		break;
	case FirstColumnMode::Auto:
		m_timeIsFirst = canDrop && (m_scopyHeader || isStrictlyIncreasing(m_columns.first()));
		break;
	}

	m_effectiveRate = m_declaredRate;
	if(m_effectiveRate > 0.0 || !m_timeIsFirst) {
		return;
	}

	const QVector<float> &x = m_columns.first();
	const double step = medianStep(x);
	const bool isSampleIndex = qFuzzyCompare(step, 1.0) && qFuzzyIsNull(x.first());
	if(step > 0.0 && !isSampleIndex) {
		m_effectiveRate = 1.0 / step;
	}
}

QStringList CsvFileReader::channels() const { return m_timeIsFirst ? m_names.mid(1) : m_names; }

const QVector<float> &CsvFileReader::samples(int index) const
{
	const int column = m_timeIsFirst ? index + 1 : index;
	if(index < 0 || column >= m_columns.size()) {
		return emptyChannel();
	}
	return m_columns.at(column);
}

} // namespace adc
} // namespace scopy
