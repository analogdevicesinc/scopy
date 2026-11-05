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

#ifndef REGISTERSESSION_H
#define REGISTERSESSION_H

#include "scopy-dcdebugger_export.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QString>

#include <core/result.h>

namespace scopy::component {
class RegisterReader;
class RegisterWriter;
} // namespace scopy::component

namespace scopy::dcdebugger {

struct RegisterEntry
{
	QDateTime time;
	QString op; // "read" | "write"
	uint32_t address = 0;
	uint32_t value = 0;
	bool ok = true;
	QString error;
};

// Register access over the generic RegisterReader / RegisterWriter capabilities of one node:
// single reads and writes, range dumps, and the history of everything done. Operations are
// queued and run one at a time on the node's own executor, so a dump keeps the bus order.
class SCOPY_DCDEBUGGER_EXPORT RegisterSession : public QObject
{
	Q_OBJECT
public:
	explicit RegisterSession(QObject *parent = nullptr);

	// Either may be null (read-only or write-only node). Pending operations are dropped.
	void setTarget(component::RegisterReader *reader, component::RegisterWriter *writer);
	component::RegisterReader *reader() const;
	component::RegisterWriter *writer() const;
	bool canRead() const;
	bool canWrite() const;
	int addressBits() const;
	int valueBits() const;

	bool isBusy() const { return m_inFlight || !m_queue.isEmpty(); }
	// Explicit read: the result is also reported through readDone().
	void read(uint32_t address);
	void write(uint32_t address, uint32_t value);
	// `count` consecutive reads from `from`; results go to the history only.
	void dump(uint32_t from, int count);
	// Drops what is still queued; the operation in flight completes.
	void cancel();

	const QList<RegisterEntry> &history() const { return m_history; }
	void clearHistory();

	static uint32_t mask(int bits);
	static QString hex(uint32_t value, int bits);
	// "0x1F", "1F", "1f" -> 31; false on anything else or on overflow of `bits`.
	static bool parseHex(const QString &text, int bits, uint32_t &out);

Q_SIGNALS:
	void entryAdded(const RegisterEntry &entry);
	void readDone(uint32_t address, uint32_t value);
	void busyChanged(bool busy);
	void dumpFinished(int okCount, int total);

private:
	struct Op
	{
		bool isWrite = false;
		bool explicitRead = true;
		uint32_t address = 0;
		uint32_t value = 0;
	};
	void enqueue(const Op &op);
	void pump();
	void finish(const Op &op, uint32_t value, const scopy::Error *error);

	QPointer<component::RegisterReader> m_reader;
	QPointer<component::RegisterWriter> m_writer;
	QQueue<Op> m_queue;
	bool m_inFlight = false;
	int m_dumpTotal = 0;
	int m_dumpDone = 0;
	int m_dumpOk = 0;
	QList<RegisterEntry> m_history;
};

} // namespace scopy::dcdebugger

Q_DECLARE_METATYPE(scopy::dcdebugger::RegisterEntry)

#endif // REGISTERSESSION_H
