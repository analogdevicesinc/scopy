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

#ifndef REGISTERMAPVALUES_HPP
#define REGISTERMAPVALUES_HPP

#include "scopy-regmap_export.h"

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QVector>

#include <core/result.h>
#include <qcoro/qcorotask.h>

namespace scopy::regmap {
class IRegisterWriteStrategy;
class IRegisterReadStrategy;

// a value that was read from the device; written values are never cached
struct CacheEntry
{
	uint32_t value = 0;
	QDateTime completedAt;
};

class SCOPY_REGMAP_EXPORT RegisterMapValues : public QObject
{
	Q_OBJECT
public:
	explicit RegisterMapValues(QObject *parent = nullptr);
	~RegisterMapValues();

	// reads count registers; on success each one goes to the cache, on error accessFailed is emitted
	QCoro::Task<scopy::Result<QVector<uint32_t>>> read(uint32_t address, int count = 1);
	// writes, then reads the registers back; only the read-back updates the cache
	QCoro::Task<scopy::Result<QVector<uint32_t>>> writeThenRead(uint32_t address, QVector<uint32_t> values);

	uint32_t getValueOfRegister(uint32_t address);
	bool hasValue(uint32_t address);
	void setReadStrategy(IRegisterReadStrategy *readStrategy);
	void setWriteStrategy(IRegisterWriteStrategy *writeStrategy);
	void registerDump(QString path);
	// used in the read/write error messages
	void setDeviceName(const QString &deviceName);

	IRegisterReadStrategy *getReadStrategy() const;

	IRegisterWriteStrategy *getWriteStrategy() const;

Q_SIGNALS:
	void registerValueChanged(uint32_t address, uint32_t value);
	void accessFailed(uint32_t address, const scopy::Error &error);
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);

private:
	IRegisterReadStrategy *readStrategy = nullptr;
	IRegisterWriteStrategy *writeStrategy = nullptr;
	QMap<uint32_t, CacheEntry> m_cache;
	QString m_deviceName;
};
} // namespace scopy::regmap
#endif // REGISTERMAPVALUES_HPP
