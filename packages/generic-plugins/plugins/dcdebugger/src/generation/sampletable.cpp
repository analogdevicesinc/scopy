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

#include "sampletable.h"

#include <QRegularExpression>

#include <algorithm>
#include <cmath>

using namespace scopy::dcdebugger;

namespace {
bool toCode(const QString &text, double &value)
{
	bool ok = false;
	const double v = text.trimmed().toDouble(&ok);
	if(!ok || !std::isfinite(v)) {
		return false;
	}
	value = double(std::llround(v));
	return true;
}

QStringList splitLine(const QString &line, QChar sep)
{
	static const QRegularExpression ws(QStringLiteral("\\s+"));
	QStringList cells = sep.isNull() ? line.trimmed().split(ws, Qt::SkipEmptyParts) : line.split(sep);
	for(QString &c : cells) {
		c = c.trimmed();
	}
	return cells;
}
} // namespace

bool scopy::dcdebugger::parseSamples(const QString &text, ParsedSamples &out, QString *error)
{
	out = ParsedSamples{};
	const QStringList lines = text.split(QRegularExpression(QStringLiteral("\\r\\n|\\n|\\r")));
	QChar sep;
	bool sepKnown = false;
	bool first = true;
	bool dropIndex = false;
	QVector<QVector<double>> rows;
	int columns = 0;
	for(int n = 0; n < lines.size(); ++n) {
		const QString line = lines[n].trimmed();
		if(line.isEmpty() || line.startsWith('#')) {
			continue;
		}
		if(!sepKnown) {
			for(QChar c : {QChar(','), QChar(';'), QChar('\t')}) {
				if(line.contains(c)) {
					sep = c;
					break;
				}
			}
			sepKnown = true;
		}
		const QStringList cells = splitLine(line, sep);
		if(first) {
			first = false;
			double probe = 0;
			const bool header = std::any_of(cells.cbegin(), cells.cend(),
							[&probe](const QString &c) { return !toCode(c, probe); });
			if(header) {
				out.header = cells;
				const QString h0 = cells.value(0).toLower();
				dropIndex = h0 == QLatin1String("sample") || h0 == QLatin1String("index");
				if(dropIndex) {
					out.header.removeFirst();
				}
				continue;
			}
		}
		QVector<double> row;
		row.reserve(cells.size());
		for(int c = dropIndex ? 1 : 0; c < cells.size(); ++c) {
			double v = 0;
			if(cells[c].isEmpty()) {
				row.append(0);
				continue;
			}
			if(!toCode(cells[c], v)) {
				if(error) {
					*error = QStringLiteral("line %1, column %2: \"%3\" is not a number")
							 .arg(n + 1)
							 .arg(c + 1)
							 .arg(cells[c]);
				}
				return false;
			}
			row.append(v);
		}
		columns = std::max(columns, int(row.size()));
		rows.append(row);
	}
	if(rows.isEmpty() || columns == 0) {
		if(error) {
			*error = QStringLiteral("no samples");
		}
		return false;
	}
	out.columns.resize(columns);
	for(QVector<double> &col : out.columns) {
		col.fill(0, rows.size());
	}
	for(int r = 0; r < rows.size(); ++r) {
		for(int c = 0; c < rows[r].size(); ++c) {
			out.columns[c][r] = rows[r][c];
		}
	}
	return true;
}

SampleTableModel::SampleTableModel(const QStringList &channels, int rows, QObject *parent)
	: QAbstractTableModel(parent)
	, m_channels(channels)
	, m_columns(channels.size())
{
	setRows(rows);
}

int SampleTableModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_rows; }

int SampleTableModel::columnCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : int(m_channels.size());
}

double SampleTableModel::value(int row, int column) const
{
	return column >= 0 && column < m_columns.size() ? m_columns[column].value(row) : 0;
}

QVariant SampleTableModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return {};
	}
	if(role == Qt::TextAlignmentRole) {
		return int(Qt::AlignRight | Qt::AlignVCenter);
	}
	if(role != Qt::DisplayRole && role != Qt::EditRole) {
		return {};
	}
	return QString::number(qint64(value(index.row(), index.column())));
}

bool SampleTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	double v = 0;
	if(!index.isValid() || role != Qt::EditRole || !toCode(value.toString(), v)) {
		return false; // not a number: the cell keeps its value
	}
	m_columns[index.column()][index.row()] = v;
	Q_EMIT dataChanged(index, index);
	Q_EMIT samplesChanged();
	return true;
}

QVariant SampleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole) {
		return {};
	}
	return orientation == Qt::Horizontal ? QVariant(m_channels.value(section)) : QVariant(section);
}

Qt::ItemFlags SampleTableModel::flags(const QModelIndex &index) const
{
	return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable : Qt::NoItemFlags;
}

void SampleTableModel::setRows(int rows)
{
	rows = std::max(0, rows);
	if(rows == m_rows) {
		return;
	}
	beginResetModel();
	for(QVector<double> &col : m_columns) {
		col.resize(rows); // new cells are 0
	}
	m_rows = rows;
	endResetModel();
	Q_EMIT samplesChanged();
}

void SampleTableModel::setColumns(const QVector<QVector<double>> &columns)
{
	int rows = 0;
	for(const QVector<double> &c : columns) {
		rows = std::max(rows, int(c.size()));
	}
	beginResetModel();
	for(int c = 0; c < m_columns.size(); ++c) {
		m_columns[c] = c < columns.size() ? columns[c] : QVector<double>();
		m_columns[c].resize(rows);
	}
	m_rows = rows;
	endResetModel();
	Q_EMIT samplesChanged();
}

void SampleTableModel::paste(int row, int column, const QVector<QVector<double>> &block)
{
	if(block.isEmpty() || column < 0 || column >= m_columns.size()) {
		return;
	}
	int needed = m_rows;
	for(const QVector<double> &c : block) {
		needed = std::max(needed, row + int(c.size()));
	}
	beginResetModel();
	if(needed != m_rows) {
		for(QVector<double> &col : m_columns) {
			col.resize(needed);
		}
		m_rows = needed;
	}
	for(int c = 0; c < block.size() && column + c < m_columns.size(); ++c) {
		for(int r = 0; r < block[c].size(); ++r) {
			m_columns[column + c][row + r] = block[c][r];
		}
	}
	endResetModel();
	Q_EMIT samplesChanged();
}

QString SampleTableModel::toTsv(int firstRow, int lastRow, int firstColumn, int lastColumn) const
{
	firstRow = std::max(0, firstRow);
	lastRow = std::min(m_rows - 1, lastRow);
	firstColumn = std::max(0, firstColumn);
	lastColumn = std::min(int(m_columns.size()) - 1, lastColumn);
	QString out;
	for(int r = firstRow; r <= lastRow; ++r) {
		for(int c = firstColumn; c <= lastColumn; ++c) {
			if(c > firstColumn) {
				out += '\t';
			}
			out += QString::number(qint64(m_columns[c][r]));
		}
		out += '\n';
	}
	return out;
}

QString SampleTableModel::toCsv() const
{
	QString csv = QStringLiteral("sample");
	for(const QString &ch : m_channels) {
		csv += "," + ch;
	}
	csv += "\n";
	for(int r = 0; r < m_rows; ++r) {
		csv += QString::number(r);
		for(const QVector<double> &col : m_columns) {
			csv += "," + QString::number(qint64(col[r]));
		}
		csv += "\n";
	}
	return csv;
}

#include "moc_sampletable.cpp"
