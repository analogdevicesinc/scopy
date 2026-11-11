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

#ifndef COMMANDLOGMODEL_H
#define COMMANDLOGMODEL_H

#include "scopy-dcdebugger_export.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QList>
#include <QString>

#include <core/callrecord.h>

namespace scopy::dcdebugger {

// What the log shows; empty fields match everything (case-insensitive substrings).
struct CommandLogFilter
{
	QString component; // origin path
	QString library;   // any call's library
	QString function;  // any call's function
	bool errorsOnly = false;

	bool isEmpty() const;
	bool matches(const CommandLogEntry &entry) const;
};

// The command log: a ring buffer of CommandLogEntry (oldest dropped when full) shown as a two-level
// table. Top rows are commands; identical consecutive commands (same origin, calls with the same
// function / handle / args / rc) stay one row each until the run reaches collapseAfter(), then fold
// into one row "×N" with min / avg / max duration. Children: the calls of a multi-call command, or
// the commands of a folded run. Folding is display only: the buffer keeps every entry.
// Pause freezes the rows (entries keep being buffered); resume shows everything buffered.
class SCOPY_DCDEBUGGER_EXPORT CommandLogModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum Column
	{
		TimeColumn = 0,
		ComponentColumn,
		OperationColumn,
		ArgsColumn,
		ValueColumn,
		DurationColumn,
		ColumnCount
	};
	enum Role
	{
		FailedRole = Qt::UserRole + 1,
		RepeatRole,	 // entries folded into a top row (1 for a plain row)
		OriginRole,	 // origin path of the row's command
		BackendRole,	 // backend id of the row's command
		OriginObjectRole // quintptr of the node that issued it
	};

	explicit CommandLogModel(QObject *parent = nullptr);
	~CommandLogModel() override;

	// Ring buffer size (entries); shrinking drops the oldest.
	int capacity() const { return m_capacity; }
	void setCapacity(int entries);
	// Fold from the n-th identical consecutive command; 0 = never.
	int collapseAfter() const { return m_collapseAfter; }
	void setCollapseAfter(int n);

	// Only entries of this connection are kept (empty: all).
	QString uri() const { return m_uri; }
	void setUri(const QString &uri);

	CommandLogFilter filter() const { return m_filter; }
	void setFilter(const CommandLogFilter &filter);

	bool isPaused() const { return m_paused; }
	void setPaused(bool paused);

	void setErrorColor(const QColor &color) { m_errorColor = color; }

	int entryCount() const { return m_entries.size(); }
	const CommandLogEntry &entryAt(int index) const { return m_entries.at(index); }
	// Buffered entries that pass the filter, oldest first.
	QList<CommandLogEntry> filteredEntries() const;
	void clear();

	// One line per call of every filtered entry (entries without calls get one line).
	static QStringList csvColumns();
	QString toCsv() const;

	// The entries behind `rows`, oldest first: a top row gives every entry it shows, a folded
	// row's child its entry, a call row (child of a multi-call command) that call only (several
	// call rows of one command: those calls, in order; the command row too: the whole command).
	QList<CommandLogEntry> entriesFor(const QModelIndexList &rows) const;
	// The entries behind every row (filtered; frozen while paused), oldest first.
	QList<CommandLogEntry> visibleEntries() const;

	// A row as shown, one text per column (folded: "×N" and min / avg / max); empty if invalid.
	QStringList rowFields(const QModelIndex &index) const;
	// One tab-separated line of rowFields() per row, in display order (any column of a row counts once).
	QString rowsText(const QModelIndexList &rows) const;

	static QString timeString(qint64 ns);
	static QString durationString(qint64 ns);
	static bool sameCommand(const CommandLogEntry &a, const CommandLogEntry &b);

	// QAbstractItemModel
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent = {}) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
	void append(const scopy::CommandLogEntry &entry);

Q_SIGNALS:
	void entryCountChanged(int count);

private:
	struct Row
	{
		quint64 id = 0;	    // stable while the row lives (child indexes point at it)
		QList<qint64> seqs; // buffered entries shown by this row (one unless folded)
		bool folded = false;
		qint64 minNs = 0, maxNs = 0, sumNs = 0;
	};

	const CommandLogEntry *entryBySeq(qint64 seq) const;
	static qint64 durationOf(const CommandLogEntry &e);
	void addToRows(qint64 seq, bool notify);
	void rebuildRows();
	void dropOldest();
	void recomputeStats(Row &row) const;
	const Row *rowById(quint64 id) const;
	int childCount(const Row &row) const;
	QVariant entryData(const CommandLogEntry &e, int column, int role) const;
	QVariant callData(const CallRecord &c, int column, int role) const;
	QVariant foldedData(const Row &row, int column, int role) const;

	QList<CommandLogEntry> m_entries;
	qint64 m_firstSeq = 0; // seq of m_entries.first()
	QList<Row> m_rows;
	quint64 m_nextRowId = 1;
	int m_runLength = 0; // trailing plain rows identical to the last one

	int m_capacity = 10000;
	int m_collapseAfter = 10;
	QString m_uri;
	CommandLogFilter m_filter;
	bool m_paused = false;
	QColor m_errorColor{Qt::red};
};

} // namespace scopy::dcdebugger

#endif // COMMANDLOGMODEL_H
