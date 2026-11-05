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

#include "registersession.h"

#include <QTimer>
#include <algorithm>
#include <cerrno>

#include <component/registerreader.h>
#include <component/registerwriter.h>
#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;

RegisterSession::RegisterSession(QObject *parent)
	: QObject(parent)
{
	qRegisterMetaType<RegisterEntry>();
}

void RegisterSession::setTarget(RegisterReader *reader, RegisterWriter *writer)
{
	cancel();
	m_reader = reader;
	m_writer = writer;
}

RegisterReader *RegisterSession::reader() const { return m_reader; }

RegisterWriter *RegisterSession::writer() const { return m_writer; }

bool RegisterSession::canRead() const { return !m_reader.isNull(); }

bool RegisterSession::canWrite() const { return !m_writer.isNull(); }

int RegisterSession::addressBits() const
{
	if(m_reader) {
		return m_reader->addressBits();
	}
	return m_writer ? m_writer->addressBits() : 32;
}

int RegisterSession::valueBits() const
{
	if(m_reader) {
		return m_reader->valueBits();
	}
	return m_writer ? m_writer->valueBits() : 32;
}

void RegisterSession::read(uint32_t address) { enqueue({false, true, address, 0}); }

void RegisterSession::write(uint32_t address, uint32_t value) { enqueue({true, false, address, value}); }

void RegisterSession::dump(uint32_t from, int count)
{
	if(count <= 0) {
		return;
	}
	m_dumpTotal += count;
	for(int i = 0; i < count; ++i) {
		// Addresses past the end of the space wrap like the counter would.
		enqueue({false, false, static_cast<uint32_t>(from + static_cast<uint32_t>(i)), 0});
	}
}

void RegisterSession::cancel()
{
	const bool was = isBusy();
	m_queue.clear();
	m_dumpTotal = 0;
	m_dumpDone = 0;
	m_dumpOk = 0;
	if(was && !isBusy()) {
		Q_EMIT busyChanged(false);
	}
}

void RegisterSession::clearHistory() { m_history.clear(); }

uint32_t RegisterSession::mask(int bits)
{
	if(bits >= 32) {
		return 0xFFFFFFFFu;
	}
	if(bits <= 0) {
		return 0;
	}
	return (1u << bits) - 1;
}

QString RegisterSession::hex(uint32_t value, int bits)
{
	const int digits = std::max(1, (std::clamp(bits, 1, 32) + 3) / 4);
	return QStringLiteral("0x") + QString::number(value, 16).toUpper().rightJustified(digits, '0');
}

bool RegisterSession::parseHex(const QString &text, int bits, uint32_t &out)
{
	QString t = text.trimmed();
	if(t.startsWith("0x", Qt::CaseInsensitive)) {
		t = t.mid(2);
	}
	if(t.isEmpty()) {
		return false;
	}
	bool ok = false;
	const qulonglong v = t.toULongLong(&ok, 16);
	if(!ok || v > mask(bits)) {
		return false;
	}
	out = static_cast<uint32_t>(v);
	return true;
}

void RegisterSession::enqueue(const Op &op)
{
	const bool was = isBusy();
	m_queue.enqueue(op);
	if(!was) {
		Q_EMIT busyChanged(true);
	}
	pump();
}

void RegisterSession::pump()
{
	if(m_inFlight || m_queue.isEmpty()) {
		return;
	}
	const Op op = m_queue.dequeue();
	m_inFlight = true;
	if(op.isWrite) {
		if(!m_writer) {
			const Error e{-ENOTSUP, "no register writer on this node"};
			finish(op, 0, &e);
			return;
		}
		QCoro::connect(m_writer->writeAsync(op.address, op.value), this,
			       [this, op](const CommandResponse<void> &r) {
				       if(!r) {
					       const Error e = r.error();
					       finish(op, 0, &e);
				       } else {
					       finish(op, op.value, nullptr);
				       }
			       });
		return;
	}
	if(!m_reader) {
		const Error e{-ENOTSUP, "no register reader on this node"};
		finish(op, 0, &e);
		return;
	}
	QCoro::connect(m_reader->readAsync(op.address), this, [this, op](const CommandResponse<uint32_t> &r) {
		if(!r) {
			const Error e = r.error();
			finish(op, 0, &e);
		} else {
			finish(op, r.value(), nullptr);
		}
	});
}

void RegisterSession::finish(const Op &op, uint32_t value, const Error *error)
{
	m_inFlight = false;
	RegisterEntry entry;
	entry.time = QDateTime::currentDateTime();
	entry.op = op.isWrite ? QStringLiteral("write") : QStringLiteral("read");
	entry.address = op.address;
	entry.value = value;
	entry.ok = error == nullptr;
	if(error) {
		entry.error = error->errorString();
	}
	m_history.append(entry);
	Q_EMIT entryAdded(entry);
	if(!op.isWrite && op.explicitRead && entry.ok) {
		Q_EMIT readDone(op.address, value);
	}
	if(!op.isWrite && !op.explicitRead && m_dumpTotal > 0) {
		m_dumpDone++;
		if(entry.ok) {
			m_dumpOk++;
		}
		if(m_dumpDone >= m_dumpTotal) {
			const int ok = m_dumpOk;
			const int total = m_dumpTotal;
			m_dumpTotal = m_dumpDone = m_dumpOk = 0;
			Q_EMIT dumpFinished(ok, total);
		}
	}
	if(m_queue.isEmpty()) {
		Q_EMIT busyChanged(false);
	} else {
		// Yield to the event loop so synchronous leaves cannot recurse through the queue.
		QTimer::singleShot(0, this, &RegisterSession::pump);
	}
}

#include "moc_registersession.cpp"
