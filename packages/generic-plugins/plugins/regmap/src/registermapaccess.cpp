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

#include "registermapaccess.hpp"

#include "logging_categories.h"
#include "registermapdatastore.hpp"

#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <component/backends/file/regmap/registervaluesfile.h>
#include <pluginbase/statusbarmanager.h>
#include <qcoro/qcorotimer.h>

#include <cerrno>
#include <chrono>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::regmap;

namespace {
constexpr std::chrono::milliseconds BUSY_POLL_INTERVAL{10};

QString hex(uint32_t address) { return QString("0x%1").arg(address, 0, 16); }

QVector<uint32_t> addressesOf(const QVector<RegisterData> &data)
{
	QVector<uint32_t> addresses;
	addresses.reserve(data.size());
	for(const RegisterData &d : data) {
		addresses.append(d.address);
	}
	return addresses;
}

QVector<RegisterData> toData(const QVector<uint32_t> &addresses)
{
	QVector<RegisterData> data;
	data.reserve(addresses.size());
	for(uint32_t address : addresses) {
		data.append(RegisterData{address});
	}
	return data;
}

QVector<RegisterData> failAll(const QVector<uint32_t> &addresses, const Error &error)
{
	const QDateTime now = QDateTime::currentDateTime();
	QVector<RegisterData> results;
	results.reserve(addresses.size());
	for(uint32_t address : addresses) {
		results.append(RegisterData::failed(address, error, now));
	}
	return results;
}
} // namespace

RegisterMapAccess::RegisterMapAccess(IRegisterReadStrategy *reader, IRegisterWriteStrategy *writer, bool readBack,
				     const QString &deviceName, QObject *parent)
	: QObject(parent)
	, m_reader(reader)
	, m_writer(writer)
	, m_readBack(readBack)
	, m_deviceName(deviceName)
	, m_store(new RegisterMapDataStore(this))
{}

RegisterMapDataStore *RegisterMapAccess::store() const { return m_store; }

bool RegisterMapAccess::readBack() const { return m_readBack; }

QString RegisterMapAccess::errorText(const Error &error)
{
	QString text = error.errorString();
	if(!error.message.isEmpty() && error.code != 0) {
		text += QString(" (%1)").arg(error.code);
	}
	return text;
}

QCoro::Task<RegisterData> RegisterMapAccess::read(uint32_t address)
{
	const QVector<uint32_t> addresses{address};
	QVector<RegisterData> results = co_await readAll(addresses, false);
	co_return results.first();
}

QCoro::Task<RegisterData> RegisterMapAccess::readWhenIdle(uint32_t address)
{
	const QVector<uint32_t> addresses{address};
	QVector<RegisterData> results = co_await readAll(addresses, true);
	co_return results.first();
}

QCoro::Task<QVector<RegisterData>> RegisterMapAccess::readRange(uint32_t start, uint32_t end, bool waitWhenBusy)
{
	if(end < start) {
		qWarning(CAT_REGMAP) << m_deviceName << "read range" << hex(start) << "-" << hex(end) << "is empty";
		co_return QVector<RegisterData>();
	}
	QVector<uint32_t> addresses;
	for(uint64_t address = start; address <= end; address++) {
		addresses.append(static_cast<uint32_t>(address));
	}
	co_return co_await readAll(addresses, waitWhenBusy);
}

QCoro::Task<QVector<RegisterData>> RegisterMapAccess::readAll(QVector<uint32_t> addresses, bool waitWhenBusy)
{
	// the register map can be deleted while a call runs (disconnect, reconfigure)
	QPointer<RegisterMapAccess> self(this);
	// A running call always ends and frees its registers. The wait polls instead of awaiting pendingChanged: a
	// signal await resumes queued, so another call could take the register in between, and it never resumes if
	// the store is deleted.
	while(waitWhenBusy && self && m_store->isAnyPending(addresses)) {
		co_await QCoro::sleepFor(BUSY_POLL_INTERVAL);
	}
	if(!self) {
		co_return failAll(addresses, Error{-ECANCELED, "register map deleted"});
	}

	// from here to the strategy call there is no suspension, so the registers are still free
	QPointer<IRegisterReadStrategy> reader = m_reader;
	if(!reader) {
		QVector<RegisterData> results = failAll(addresses, Error{-ENOTSUP, "no read strategy"});
		reportErrors("read", results);
		co_return results;
	}
	if(!m_store->setPending(addresses)) {
		qInfo(CAT_REGMAP) << m_deviceName << "register busy, read skipped:" << addresses;
		co_return failAll(addresses, Error{-EBUSY, "register busy"});
	}

	const QVector<RegisterData> request = toData(addresses);
	QVector<RegisterData> results;
	if(request.size() == 1) {
		results.append(co_await reader->read(request.first()));
	} else {
		results = co_await reader->readRange(request);
	}
	if(!self) {
		co_return results;
	}
	m_store->store(results);
	// no register stays pending, even if the strategy answered for fewer than it was asked
	m_store->restore(addresses);
	reportErrors("read", results);
	co_return results;
}

QCoro::Task<RegisterData> RegisterMapAccess::writeThenRead(uint32_t address, uint32_t value)
{
	const QVector<RegisterData> values{RegisterData{address, value}};
	QVector<RegisterData> results = co_await writeAll(values, false);
	co_return results.first();
}

QCoro::Task<RegisterData> RegisterMapAccess::writeThenReadWhenIdle(uint32_t address, uint32_t value)
{
	const QVector<RegisterData> values{RegisterData{address, value}};
	QVector<RegisterData> results = co_await writeAll(values, true);
	co_return results.first();
}

QCoro::Task<QVector<RegisterData>> RegisterMapAccess::writeAll(QVector<RegisterData> values, bool waitWhenBusy)
{
	QPointer<RegisterMapAccess> self(this);
	const QVector<uint32_t> addresses = addressesOf(values);
	while(waitWhenBusy && self && m_store->isAnyPending(addresses)) {
		co_await QCoro::sleepFor(BUSY_POLL_INTERVAL);
	}
	if(!self) {
		co_return failAll(addresses, Error{-ECANCELED, "register map deleted"});
	}

	QPointer<IRegisterWriteStrategy> writer = m_writer;
	QPointer<IRegisterReadStrategy> reader = m_reader;
	const bool readBack = m_readBack;
	if(!writer) {
		QVector<RegisterData> results = failAll(addresses, Error{-ENOTSUP, "no write strategy"});
		reportErrors("write", results);
		co_return results;
	}
	// the registers stay pending until the read-back is stored
	if(!m_store->setPending(addresses)) {
		qInfo(CAT_REGMAP) << m_deviceName << "register busy, write skipped:" << addresses;
		co_return failAll(addresses, Error{-EBUSY, "register busy"});
	}

	QVector<RegisterData> written;
	if(values.size() == 1) {
		written.append(co_await writer->write(values.first()));
	} else {
		written = co_await writer->writeRange(values);
	}
	if(!self) {
		co_return written;
	}

	// A write isn't stored. Each register written Ok is read back, when the reader is the same type as the
	// writer: a read from another source wouldn't show what was written.
	QVector<uint32_t> writtenOk;
	for(const RegisterData &w : std::as_const(written)) {
		if(w.isOk()) {
			writtenOk.append(w.address);
		}
	}
	if(readBack && reader && !writtenOk.isEmpty()) {
		const QVector<RegisterData> request = toData(writtenOk);
		QVector<RegisterData> readResults;
		if(request.size() == 1) {
			readResults.append(co_await reader->read(request.first()));
		} else {
			readResults = co_await reader->readRange(request);
		}
		if(!self) {
			co_return written;
		}
		m_store->store(readResults);
		reportErrors("read", readResults);
	}
	// what wasn't read back goes back to what it was: the failed writes, or all of them for a mixed pair
	m_store->restore(addresses);
	reportErrors("write", written);
	co_return written;
}

QCoro::Task<void> RegisterMapAccess::writeFromFile(QString path, bool waitWhenBusy)
{
	const auto entries = file::RegisterValuesFile(path).readAll();
	if(!entries) {
		StatusBarManager::pushMessage("Register map " + m_deviceName + ": " + errorText(entries.error()));
		co_return;
	}
	// file order, which can matter on a device; the addresses don't have to be contiguous
	QVector<RegisterData> values;
	for(const auto &entry : entries.value()) {
		values.append(RegisterData{entry.first, entry.second});
	}
	if(values.isEmpty()) {
		co_return;
	}
	co_await writeAll(values, waitWhenBusy);
}

QCoro::Task<void> RegisterMapAccess::dump(QString path, std::optional<QPair<uint32_t, uint32_t>> readFirst)
{
	QPointer<RegisterMapAccess> self(this);
	if(readFirst) {
		// the file gets the values just read, not the ones from before
		co_await readRange(readFirst->first, readFirst->second);
		if(!self) {
			co_return;
		}
	}
	writeDumpFile(path);
}

void RegisterMapAccess::writeDumpFile(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly)) {
		StatusBarManager::pushMessage("Can't open file!", 3000);
		return;
	}
	QTextStream out(&file);
	for(uint32_t address : m_store->addresses()) {
		const RegisterData *entry = m_store->get(address);
		// the Ok values only: an error or a register never read has no value to dump
		if(entry && entry->value && entry->status != RegisterAccessStatus::Error) {
			out << QString::number(address, 16) << "," << QString::number(*entry->value, 16) << Qt::endl;
		}
	}
}

void RegisterMapAccess::reportErrors(const QString &operation, const QVector<RegisterData> &results)
{
	QStringList failed;
	const RegisterData *firstFailed = nullptr;
	for(const RegisterData &r : results) {
		if(r.isError()) {
			failed.append(hex(r.address));
			if(!firstFailed) {
				firstFailed = &r;
			}
		}
	}
	if(!firstFailed) {
		return;
	}
	QString message;
	if(results.size() == 1) {
		message = QString("Register map %1 %2: %3")
				  .arg(m_deviceName, hex(firstFailed->address),
				       errorText(firstFailed->error.value_or(Error{})));
	} else {
		message = QString("Register map %1: the %2 failed for the following registers: %3")
				  .arg(m_deviceName, operation, failed.join(", "));
	}
	qWarning(CAT_REGMAP) << message;
	StatusBarManager::pushMessage(message);
}
