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

#include "registermapvalues.hpp"

#include "logging_categories.h"

#include <readwrite/iregisterreadstrategy.hpp>
#include <readwrite/iregisterwritestrategy.hpp>

#include <QFile>
#include <QPointer>
#include <QTextStream>
#include <QDebug>

#include <pluginbase/statusbarmanager.h>

#include <qcoro/qcorotimer.h>

#include <cerrno>
#include <chrono>

using namespace scopy;
using namespace regmap;

namespace {
constexpr std::chrono::milliseconds BUSY_POLL_INTERVAL{10};
}

RegisterMapValues::RegisterMapValues(QObject *parent)
	: QObject{parent}
{
	// the returned tasks are dropped here: the result reaches the UI through the signals
	QObject::connect(this, &RegisterMapValues::requestRead, this, [this](uint32_t address) { read(address); });
	QObject::connect(this, &RegisterMapValues::requestWrite, this,
			 [this](uint32_t address, uint32_t value) { writeThenRead(address, {value}); });
	QObject::connect(this, &RegisterMapValues::accessFailed, this, [this](uint32_t address, const Error &error) {
		QString text = error.errorString();
		if(!error.message.isEmpty() && error.code != 0) {
			text += QString(" (%1)").arg(error.code);
		}
		StatusBarManager::pushMessage(
			QString("Register map %1 0x%2: %3").arg(m_deviceName).arg(address, 0, 16).arg(text));
	});
}

RegisterMapValues::~RegisterMapValues() {}

bool RegisterMapValues::hasValue(uint32_t address) { return m_cache.contains(address); }

uint32_t RegisterMapValues::getValueOfRegister(uint32_t address) { return m_cache.value(address).value; }

bool RegisterMapValues::isBusy(uint32_t address) const { return m_busy.contains(address); }

bool RegisterMapValues::isRangeBusy(uint32_t address, int count) const
{
	for(int i = 0; i < count; i++) {
		if(m_busy.contains(address + i)) {
			return true;
		}
	}
	return false;
}

bool RegisterMapValues::markBusy(uint32_t address, int count)
{
	if(isRangeBusy(address, count)) {
		return false;
	}
	for(int i = 0; i < count; i++) {
		m_busy.insert(address + i);
		Q_EMIT busyChanged(address + i, true);
	}
	return true;
}

void RegisterMapValues::clearBusy(uint32_t address, int count)
{
	for(int i = 0; i < count; i++) {
		m_busy.remove(address + i);
		Q_EMIT busyChanged(address + i, false);
	}
}

QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::read(uint32_t address, int count)
{
	// one call per register at a time: the running call updates the value anyway
	if(!markBusy(address, count)) {
		qInfo(CAT_REGMAP) << m_deviceName << "register" << QString("0x%1").arg(address, 0, 16)
				  << "is busy, read skipped";
		co_return Unexpected(Error{-EBUSY, "register busy"});
	}

	QPointer<RegisterMapValues> self(this);
	auto r = co_await readAndCache(address, count);
	if(self) {
		self->clearBusy(address, count);
	}
	co_return r;
}

// A running call always ends (OK, error or the controller's timeout) and frees its registers.
// The wait polls instead of awaiting busyChanged: QCoro resumes a signal await queued, so another call could take
// the register in between, and a signal await never resumes if the register map is deleted.
QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::readWhenIdle(uint32_t address, int count)
{
	QPointer<RegisterMapValues> self(this);
	while(self) {
		// the check and read() run without a suspension in between, so the registers are still free
		if(!self->isRangeBusy(address, count)) {
			co_return co_await self->read(address, count);
		}
		co_await QCoro::sleepFor(BUSY_POLL_INTERVAL);
	}
	co_return Unexpected(Error{-ECANCELED, "register map deleted"});
}

QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::writeThenReadWhenIdle(uint32_t address,
										QVector<uint32_t> values)
{
	QPointer<RegisterMapValues> self(this);
	while(self) {
		if(!self->isRangeBusy(address, values.size())) {
			co_return co_await self->writeThenRead(address, values);
		}
		co_await QCoro::sleepFor(BUSY_POLL_INTERVAL);
	}
	co_return Unexpected(Error{-ECANCELED, "register map deleted"});
}

QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::readAndCache(uint32_t address, int count)
{
	if(!readStrategy) {
		Error error{-ENOTSUP, "no read strategy"};
		Q_EMIT accessFailed(address, error);
		co_return Unexpected(error);
	}

	// the register map can be deleted while the read runs (disconnect, reconfigure)
	QPointer<RegisterMapValues> self(this);
	auto r = co_await readStrategy->read(address, count);
	if(!self) {
		co_return r;
	}
	if(!r) {
		// a failed read keeps the last good value
		Q_EMIT accessFailed(address, r.error());
		co_return r;
	}

	const QDateTime completedAt = QDateTime::currentDateTime();
	for(int i = 0; i < r.value().size(); i++) {
		m_cache.insert(address + i, CacheEntry{r.value()[i], completedAt});
		Q_EMIT registerValueChanged(address + i, r.value()[i]);
	}
	co_return r;
}

QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::writeThenRead(uint32_t address, QVector<uint32_t> values)
{
	if(!writeStrategy) {
		Error error{-ENOTSUP, "no write strategy"};
		Q_EMIT accessFailed(address, error);
		co_return Unexpected(error);
	}
	// the registers stay busy until the read-back ends
	const int count = values.size();
	if(!markBusy(address, count)) {
		qInfo(CAT_REGMAP) << m_deviceName << "register" << QString("0x%1").arg(address, 0, 16)
				  << "is busy, write skipped";
		co_return Unexpected(Error{-EBUSY, "register busy"});
	}

	QPointer<RegisterMapValues> self(this);
	auto w = co_await writeStrategy->write(address, values);
	if(!self) {
		co_return Unexpected(Error{-ECANCELED, "register map deleted"});
	}
	if(!w) {
		// a write that failed changes nothing in the cache
		Q_EMIT accessFailed(address, w.error());
		clearBusy(address, count);
		co_return Unexpected(w.error());
	}
	// only the read-back updates the cache
	auto r = co_await readAndCache(address, count);
	if(self) {
		self->clearBusy(address, count);
	}
	co_return r;
}

void RegisterMapValues::setReadStrategy(IRegisterReadStrategy *readStrategy) { this->readStrategy = readStrategy; }

void RegisterMapValues::setWriteStrategy(IRegisterWriteStrategy *writeStrategy) { this->writeStrategy = writeStrategy; }

void RegisterMapValues::setDeviceName(const QString &deviceName) { m_deviceName = deviceName; }

void RegisterMapValues::registerDump(QString path)
{
	QFile file(path);
	if(!file.isOpen()) {
		if(!file.open(QIODevice::WriteOnly)) {
			StatusBarManager::pushMessage("Can't open file!", 3000);
		} else {
			QTextStream out(&file);

			for(auto it = m_cache.cbegin(); it != m_cache.cend(); ++it) {
				out << QString::number(it.key(), 16) << "," << QString::number(it.value().value, 16)
				    << Qt::endl;
			}
		}

		file.close();
	} else {
		qDebug() << "File already opened! ";
	}
}

IRegisterReadStrategy *RegisterMapValues::getReadStrategy() const { return readStrategy; }

IRegisterWriteStrategy *RegisterMapValues::getWriteStrategy() const { return writeStrategy; }
