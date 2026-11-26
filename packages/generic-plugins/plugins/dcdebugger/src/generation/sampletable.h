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

#ifndef SAMPLETABLE_H
#define SAMPLETABLE_H

#include "scopy-dcdebugger_export.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>

namespace scopy::dcdebugger {

// Columns of samples read from CSV / text: comma, semicolon, tab or whitespace separated, one column
// per channel. An optional first line with a non-numeric cell is the header; a first column named
// "sample" or "index" (as Raw Acquisition exports) is dropped. Empty lines and "#" lines are skipped;
// short rows are padded with 0. Values are raw codes: rounded to integers.
struct SCOPY_DCDEBUGGER_EXPORT ParsedSamples
{
	QStringList header;
	QVector<QVector<double>> columns;
	int rows() const { return columns.isEmpty() ? 0 : int(columns.first().size()); }
};

// False (and `error` set, with the line) when a cell is not a number or there is no data.
SCOPY_DCDEBUGGER_EXPORT bool parseSamples(const QString &text, ParsedSamples &out, QString *error = nullptr);

// Editable table of raw codes: one column per channel, one row per sample (rows numbered from 0).
class SCOPY_DCDEBUGGER_EXPORT SampleTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SampleTableModel(const QStringList &channels, int rows = 0, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	QStringList channels() const { return m_channels; }
	const QVector<double> &column(int c) const { return m_columns[c]; }
	double value(int row, int column) const;
	// New rows are 0; dropped rows are lost.
	void setRows(int rows);
	// Column k of `columns` fills channel k (extra ones ignored, missing channels 0); the row count
	// becomes the longest column's.
	void setColumns(const QVector<QVector<double>> &columns);
	// `block` written from (row, column) on, growing the rows as needed; cells past the last
	// channel are ignored.
	void paste(int row, int column, const QVector<QVector<double>> &block);
	// CSV: "sample,<channels>" then one line per row (loads back with parseSamples()).
	QString toCsv() const;
	// Rows [firstRow, lastRow] of columns [firstColumn, lastColumn], tab separated (spreadsheet
	// cells; pastes back with paste()).
	QString toTsv(int firstRow, int lastRow, int firstColumn, int lastColumn) const;

Q_SIGNALS:
	// Any change of the values or of the row count.
	void samplesChanged();

private:
	QStringList m_channels;
	QVector<QVector<double>> m_columns;
	int m_rows = 0;
};

} // namespace scopy::dcdebugger

#endif // SAMPLETABLE_H
