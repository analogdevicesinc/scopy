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

#include <readwrite/iregisterreadstrategy.hpp>
#include <readwrite/iregisterwritestrategy.hpp>

#include <QFile>
#include <QPointer>
#include <QTextStream>
#include <QDebug>

#include <pluginbase/statusbarmanager.h>

#include <cerrno>

using namespace scopy;
using namespace regmap;

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

QCoro::Task<Result<QVector<uint32_t>>> RegisterMapValues::read(uint32_t address, int count)
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

	QPointer<RegisterMapValues> self(this);
	auto w = co_await writeStrategy->write(address, values);
	if(!self) {
		co_return Unexpected(Error{-ECANCELED, "register map deleted"});
	}
	if(!w) {
		// a write that failed changes nothing in the cache
		Q_EMIT accessFailed(address, w.error());
		co_return Unexpected(w.error());
	}
	// only the read-back updates the cache
	co_return co_await read(address, values.size());
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
