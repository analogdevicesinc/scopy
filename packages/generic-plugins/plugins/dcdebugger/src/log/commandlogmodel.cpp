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

#include "commandlogmodel.h"

#include <QDateTime>

#include <limits>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

bool anyCallFailed(const CommandLogEntry &e)
{
	for(const CallRecord &c : e.calls) {
		if(c.failed()) {
			return true;
		}
	}
	return false;
}

bool entryFailed(const CommandLogEntry &e) { return e.failed() || anyCallFailed(e); }

QString shortClass(const QString &cls)
{
	const qsizetype sep = cls.lastIndexOf(QLatin1String("::"));
	return sep >= 0 ? cls.mid(sep + 2) : cls;
}

QString callValue(const CallRecord &c)
{
	QString s = QStringLiteral("rc=%1").arg(c.rc);
	const QString out = c.outString();
	if(!out.isEmpty()) {
		s += QLatin1Char(' ') + out;
	}
	return s;
}

QString csvField(QString s)
{
	if(s.contains(QLatin1Char(',')) || s.contains(QLatin1Char('"')) || s.contains(QLatin1Char('\n'))) {
		s.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		return QLatin1Char('"') + s + QLatin1Char('"');
	}
	return s;
}

} // namespace

bool CommandLogFilter::isEmpty() const
{
	return component.isEmpty() && library.isEmpty() && function.isEmpty() && !errorsOnly;
}

bool CommandLogFilter::matches(const CommandLogEntry &entry) const
{
	if(!component.isEmpty() && !entry.origin.contains(component, Qt::CaseInsensitive)) {
		return false;
	}
	if(errorsOnly && !entryFailed(entry)) {
		return false;
	}
	auto anyCall = [&entry](auto pred) {
		for(const CallRecord &c : entry.calls) {
			if(pred(c)) {
				return true;
			}
		}
		return false;
	};
	if(!library.isEmpty() &&
	   !anyCall([this](const CallRecord &c) { return c.library.contains(library, Qt::CaseInsensitive); })) {
		return false;
	}
	if(!function.isEmpty() &&
	   !anyCall([this](const CallRecord &c) { return c.function.contains(function, Qt::CaseInsensitive); })) {
		return false;
	}
	return true;
}

CommandLogModel::CommandLogModel(QObject *parent)
	: QAbstractItemModel(parent)
{}

CommandLogModel::~CommandLogModel() = default;

void CommandLogModel::setCapacity(int entries)
{
	m_capacity = qMax(1, entries);
	const int before = m_entries.size();
	while(m_entries.size() > m_capacity) {
		dropOldest();
	}
	if(m_entries.size() != before) {
		Q_EMIT entryCountChanged(m_entries.size());
	}
}

void CommandLogModel::setCollapseAfter(int n)
{
	// Folding a run of one is meaningless: 1 behaves as 2.
	const int value = n <= 0 ? 0 : qMax(2, n);
	if(value == m_collapseAfter) {
		return;
	}
	m_collapseAfter = value;
	rebuildRows();
}

void CommandLogModel::setUri(const QString &uri) { m_uri = uri; }

void CommandLogModel::setFilter(const CommandLogFilter &filter)
{
	m_filter = filter;
	rebuildRows();
}

void CommandLogModel::setPaused(bool paused)
{
	if(paused == m_paused) {
		return;
	}
	m_paused = paused;
	if(!m_paused) {
		// Everything buffered while paused shows up now.
		rebuildRows();
	}
}

QList<CommandLogEntry> CommandLogModel::filteredEntries() const
{
	QList<CommandLogEntry> out;
	for(const CommandLogEntry &e : m_entries) {
		if(m_filter.matches(e)) {
			out.append(e);
		}
	}
	return out;
}

void CommandLogModel::clear()
{
	beginResetModel();
	m_firstSeq += m_entries.size();
	m_entries.clear();
	m_rows.clear();
	m_runLength = 0;
	endResetModel();
	Q_EMIT entryCountChanged(0);
}

QStringList CommandLogModel::csvColumns()
{
	return {QStringLiteral("time"),	       QStringLiteral("uri"),
		QStringLiteral("backend"),     QStringLiteral("component"),
		QStringLiteral("command"),     QStringLiteral("library"),
		QStringLiteral("function"),    QStringLiteral("handle"),
		QStringLiteral("args"),	       QStringLiteral("rc"),
		QStringLiteral("value"),       QStringLiteral("value_size"),
		QStringLiteral("duration_ns"), QStringLiteral("command_duration_ns"),
		QStringLiteral("error")};
}

QString CommandLogModel::toCsv() const
{
	QString csv = csvColumns().join(QLatin1Char(',')) + QLatin1Char('\n');
	for(const CommandLogEntry &e : m_entries) {
		if(!m_filter.matches(e)) {
			continue;
		}
		const QString error = e.failed() ? e.error.errorString() : QString();
		const QStringList head = {
			QDateTime::fromMSecsSinceEpoch(e.startNs / 1000000).toString(Qt::ISODateWithMs), e.uri,
			e.backendId, e.origin, shortClass(e.commandClass)};
		auto line = [&](const QStringList &call) {
			QStringList fields = head + call;
			fields << QString::number(e.durationNs) << error;
			for(QString &f : fields) {
				f = csvField(f);
			}
			csv += fields.join(QLatin1Char(',')) + QLatin1Char('\n');
		};
		if(e.calls.isEmpty()) {
			line({QString(), QString(), QString(), QString(), QString(), QString(), QString(), QString()});
			continue;
		}
		for(const CallRecord &c : e.calls) {
			line({c.library, c.function, QStringLiteral("0x%1").arg(c.handle, 0, 16), c.argsString(),
			      QString::number(c.rc), c.outString(),
			      c.outSize >= 0 ? QString::number(c.outSize) : QString(), QString::number(c.durationNs)});
		}
	}
	return csv;
}

QString CommandLogModel::timeString(qint64 ns)
{
	return QDateTime::fromMSecsSinceEpoch(ns / 1000000).toString(QStringLiteral("hh:mm:ss.zzz"));
}

QString CommandLogModel::durationString(qint64 ns)
{
	if(ns < 1000) {
		return QStringLiteral("%1 ns").arg(ns);
	}
	if(ns < 1000000) {
		return QStringLiteral("%1 µs").arg(ns / 1e3, 0, 'f', 1);
	}
	if(ns < 1000000000) {
		return QStringLiteral("%1 ms").arg(ns / 1e6, 0, 'f', 2);
	}
	return QStringLiteral("%1 s").arg(ns / 1e9, 0, 'f', 3);
}

bool CommandLogModel::sameCommand(const CommandLogEntry &a, const CommandLogEntry &b)
{
	if(a.origin != b.origin || a.backendId != b.backendId || a.originObject != b.originObject ||
	   a.commandClass != b.commandClass || a.error.code != b.error.code || a.calls.size() != b.calls.size()) {
		return false;
	}
	for(qsizetype i = 0; i < a.calls.size(); ++i) {
		if(!a.calls[i].sameCall(b.calls[i])) {
			return false;
		}
	}
	return true;
}

void CommandLogModel::append(const CommandLogEntry &entry)
{
	if(!m_uri.isEmpty() && entry.uri != m_uri) {
		return;
	}
	m_entries.append(entry);
	const qint64 seq = m_firstSeq + m_entries.size() - 1;
	while(m_entries.size() > m_capacity) {
		dropOldest();
	}
	if(!m_paused) {
		addToRows(seq, true);
	}
	Q_EMIT entryCountChanged(m_entries.size());
}

const CommandLogEntry *CommandLogModel::entryBySeq(qint64 seq) const
{
	const qint64 i = seq - m_firstSeq;
	if(i < 0 || i >= m_entries.size()) {
		return nullptr;
	}
	return &m_entries[i];
}

qint64 CommandLogModel::durationOf(const CommandLogEntry &e)
{
	return e.calls.size() == 1 ? e.calls.first().durationNs : e.durationNs;
}

void CommandLogModel::recomputeStats(Row &row) const
{
	row.sumNs = 0;
	row.minNs = std::numeric_limits<qint64>::max();
	row.maxNs = 0;
	for(qint64 seq : std::as_const(row.seqs)) {
		const CommandLogEntry *e = entryBySeq(seq);
		const qint64 d = e ? durationOf(*e) : 0;
		row.sumNs += d;
		row.minNs = qMin(row.minNs, d);
		row.maxNs = qMax(row.maxNs, d);
	}
	if(row.seqs.isEmpty()) {
		row.minNs = 0;
	}
}

void CommandLogModel::addToRows(qint64 seq, bool notify)
{
	const CommandLogEntry *e = entryBySeq(seq);
	if(!e || !m_filter.matches(*e)) {
		return;
	}
	if(!m_rows.isEmpty()) {
		Row &last = m_rows.last();
		const CommandLogEntry *ref = entryBySeq(last.seqs.first());
		if(ref && sameCommand(*ref, *e)) {
			const int lastPos = m_rows.size() - 1;
			if(last.folded) {
				const int child = last.seqs.size();
				if(notify) {
					beginInsertRows(index(lastPos, 0), child, child);
				}
				last.seqs.append(seq);
				const qint64 d = durationOf(*e);
				last.sumNs += d;
				last.minNs = qMin(last.minNs, d);
				last.maxNs = qMax(last.maxNs, d);
				if(notify) {
					endInsertRows();
					Q_EMIT dataChanged(index(lastPos, 0), index(lastPos, ColumnCount - 1));
				}
				return;
			}
			const int run = m_runLength + 1;
			if(m_collapseAfter > 0 && run >= m_collapseAfter) {
				// The trailing run of plain rows plus this entry become one folded row.
				const int firstPos = m_rows.size() - (run - 1);
				Row folded;
				folded.id = m_rows[firstPos].id;
				folded.folded = true;
				for(int i = firstPos; i < m_rows.size(); ++i) {
					folded.seqs += m_rows[i].seqs;
				}
				folded.seqs.append(seq);
				recomputeStats(folded);
				if(notify) {
					beginRemoveRows({}, firstPos, m_rows.size() - 1);
				}
				m_rows.remove(firstPos, m_rows.size() - firstPos);
				if(notify) {
					endRemoveRows();
					beginInsertRows({}, firstPos, firstPos);
				}
				m_rows.append(folded);
				m_nextRowId = folded.id + 1;
				m_runLength = 0;
				if(notify) {
					endInsertRows();
				}
				return;
			}
			m_runLength = run;
		} else {
			m_runLength = 1;
		}
	} else {
		m_runLength = 1;
	}

	Row row;
	row.id = m_nextRowId++;
	row.seqs.append(seq);
	row.minNs = row.maxNs = row.sumNs = durationOf(*e);
	const int pos = m_rows.size();
	if(notify) {
		beginInsertRows({}, pos, pos);
	}
	m_rows.append(row);
	if(notify) {
		endInsertRows();
	}
}

void CommandLogModel::rebuildRows()
{
	beginResetModel();
	m_rows.clear();
	m_runLength = 0;
	for(qint64 i = 0; i < m_entries.size(); ++i) {
		addToRows(m_firstSeq + i, false);
	}
	endResetModel();
}

void CommandLogModel::dropOldest()
{
	if(m_entries.isEmpty()) {
		return;
	}
	const qint64 seq = m_firstSeq;
	// Rows hold increasing seqs: only the first row can show the oldest entry.
	if(!m_rows.isEmpty() && m_rows.first().seqs.first() == seq) {
		Row &first = m_rows.first();
		if(first.seqs.size() == 1) {
			beginRemoveRows({}, 0, 0);
			m_rows.removeFirst();
			m_runLength = qMin(m_runLength, int(m_rows.size()));
			endRemoveRows();
			m_entries.removeFirst();
			++m_firstSeq;
			return;
		}
		beginRemoveRows(index(0, 0), 0, 0);
		first.seqs.removeFirst();
		endRemoveRows();
		m_entries.removeFirst();
		++m_firstSeq;
		recomputeStats(first);
		Q_EMIT dataChanged(index(0, 0), index(0, ColumnCount - 1));
		return;
	}
	m_entries.removeFirst();
	++m_firstSeq;
}

const CommandLogModel::Row *CommandLogModel::rowById(quint64 id) const
{
	if(m_rows.isEmpty() || id < m_rows.first().id) {
		return nullptr;
	}
	const quint64 pos = id - m_rows.first().id;
	if(pos >= quint64(m_rows.size()) || m_rows[pos].id != id) {
		return nullptr;
	}
	return &m_rows[pos];
}

int CommandLogModel::childCount(const Row &row) const
{
	if(row.folded) {
		return row.seqs.size();
	}
	const CommandLogEntry *e = entryBySeq(row.seqs.first());
	return (e && e->calls.size() > 1) ? e->calls.size() : 0;
}

QModelIndex CommandLogModel::index(int row, int column, const QModelIndex &parent) const
{
	if(row < 0 || column < 0 || column >= ColumnCount) {
		return {};
	}
	if(!parent.isValid()) {
		return row < m_rows.size() ? createIndex(row, column, quintptr(0)) : QModelIndex();
	}
	if(parent.internalId() != 0 || parent.row() >= m_rows.size()) {
		return {};
	}
	const Row &p = m_rows[parent.row()];
	return row < childCount(p) ? createIndex(row, column, quintptr(p.id)) : QModelIndex();
}

QModelIndex CommandLogModel::parent(const QModelIndex &child) const
{
	if(!child.isValid() || child.internalId() == 0) {
		return {};
	}
	const Row *r = rowById(child.internalId());
	if(!r) {
		return {};
	}
	return createIndex(int(r - m_rows.constData()), 0, quintptr(0));
}

int CommandLogModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid()) {
		return m_rows.size();
	}
	if(parent.internalId() != 0 || parent.column() != 0 || parent.row() >= m_rows.size()) {
		return 0;
	}
	return childCount(m_rows[parent.row()]);
}

int CommandLogModel::columnCount(const QModelIndex &) const { return ColumnCount; }

QVariant CommandLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation != Qt::Horizontal || role != Qt::DisplayRole) {
		return {};
	}
	switch(section) {
	case TimeColumn:
		return QStringLiteral("Time");
	case ComponentColumn:
		return QStringLiteral("Component");
	case OperationColumn:
		return QStringLiteral("Operation");
	case ArgsColumn:
		return QStringLiteral("Args");
	case ValueColumn:
		return QStringLiteral("Value / rc");
	case DurationColumn:
		return QStringLiteral("Duration");
	default:
		return {};
	}
}

QVariant CommandLogModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return {};
	}
	if(index.internalId() == 0) {
		if(index.row() >= m_rows.size()) {
			return {};
		}
		const Row &r = m_rows[index.row()];
		if(r.folded) {
			return foldedData(r, index.column(), role);
		}
		const CommandLogEntry *e = entryBySeq(r.seqs.first());
		return e ? entryData(*e, index.column(), role) : QVariant();
	}
	const Row *p = rowById(index.internalId());
	if(!p) {
		return {};
	}
	if(p->folded) {
		const CommandLogEntry *e = index.row() < p->seqs.size() ? entryBySeq(p->seqs[index.row()]) : nullptr;
		return e ? entryData(*e, index.column(), role) : QVariant();
	}
	const CommandLogEntry *e = entryBySeq(p->seqs.first());
	if(!e || index.row() >= e->calls.size()) {
		return {};
	}
	return callData(e->calls[index.row()], index.column(), role);
}

QVariant CommandLogModel::entryData(const CommandLogEntry &e, int column, int role) const
{
	const bool failed = entryFailed(e);
	switch(role) {
	case FailedRole:
		return failed;
	case RepeatRole:
		return 1;
	case OriginRole:
		return e.origin;
	case BackendRole:
		return e.backendId;
	case OriginObjectRole:
		return QVariant::fromValue(e.originObject);
	case Qt::ForegroundRole:
		return failed ? QVariant(m_errorColor) : QVariant();
	case Qt::DisplayRole:
	case Qt::ToolTipRole:
		break;
	default:
		return {};
	}

	const bool single = e.calls.size() == 1;
	switch(column) {
	case TimeColumn:
		return timeString(e.startNs);
	case ComponentColumn:
		return e.origin;
	case OperationColumn:
		if(single) {
			return role == Qt::ToolTipRole ? e.calls.first().toString() : e.calls.first().function;
		}
		if(e.calls.isEmpty()) {
			return shortClass(e.commandClass);
		}
		return QStringLiteral("%1 (%2 calls)").arg(shortClass(e.commandClass)).arg(e.calls.size());
	case ArgsColumn:
		if(single) {
			return e.calls.first().argsString();
		}
		return {};
	case ValueColumn: {
		QString v = single ? callValue(e.calls.first()) : (e.failed() ? QString() : QStringLiteral("ok"));
		if(e.failed()) {
			v += (v.isEmpty() ? QString() : QStringLiteral(" · ")) + e.error.errorString();
		}
		return v;
	}
	case DurationColumn:
		return durationString(durationOf(e));
	default:
		return {};
	}
}

QVariant CommandLogModel::foldedData(const Row &row, int column, int role) const
{
	const CommandLogEntry *e = entryBySeq(row.seqs.first());
	if(!e) {
		return {};
	}
	if(role == RepeatRole) {
		return int(row.seqs.size());
	}
	if(role != Qt::DisplayRole && role != Qt::ToolTipRole) {
		return entryData(*e, column, role);
	}
	switch(column) {
	case ValueColumn:
		return entryData(*e, column, role).toString() + QStringLiteral(" ×%1").arg(row.seqs.size());
	case DurationColumn: {
		const qint64 avg = row.seqs.isEmpty() ? 0 : row.sumNs / row.seqs.size();
		return QStringLiteral("%1 / %2 / %3")
			.arg(durationString(row.minNs), durationString(avg), durationString(row.maxNs));
	}
	case TimeColumn:
		if(role == Qt::ToolTipRole) {
			const CommandLogEntry *last = entryBySeq(row.seqs.last());
			return QStringLiteral("%1 … %2").arg(timeString(e->startNs),
							     timeString(last ? last->startNs : 0));
		}
		return timeString(e->startNs);
	default:
		return entryData(*e, column, role);
	}
}

QVariant CommandLogModel::callData(const CallRecord &c, int column, int role) const
{
	switch(role) {
	case FailedRole:
		return c.failed();
	case Qt::ForegroundRole:
		return c.failed() ? QVariant(m_errorColor) : QVariant();
	case Qt::DisplayRole:
	case Qt::ToolTipRole:
		break;
	default:
		return {};
	}
	switch(column) {
	case TimeColumn:
		return timeString(c.startNs);
	case OperationColumn:
		return role == Qt::ToolTipRole ? c.toString() : c.function;
	case ArgsColumn:
		return c.argsString();
	case ValueColumn:
		return callValue(c);
	case DurationColumn:
		return durationString(c.durationNs);
	default:
		return {};
	}
}

#include "moc_commandlogmodel.cpp"
