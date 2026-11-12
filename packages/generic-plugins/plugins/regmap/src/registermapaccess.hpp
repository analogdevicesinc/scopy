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

#ifndef REGISTERMAPACCESS_HPP
#define REGISTERMAPACCESS_HPP

#include "scopy-regmap_export.h"

#include <QObject>
#include <QPair>
#include <QPointer>
#include <QString>
#include <QVector>

#include <component/regmap/iregisterreadstrategy.h>
#include <component/regmap/iregisterwritestrategy.h>
#include <qcoro/qcorotask.h>

#include <optional>

namespace scopy::regmap {
class RegisterMapDataStore;

// Every register call of one device: PENDING, the strategy call, the store, write-then-read, ranges, write from
// file, dump, the JS waits and the status bar. The strategies belong to the device controller: they are only
// held, and can be gone (a null strategy fails the call).
class SCOPY_REGMAP_EXPORT RegisterMapAccess : public QObject
{
	Q_OBJECT
public:
	// readBack: a write is read back, when the reader and the writer are the same type
	RegisterMapAccess(component::IRegisterReadStrategy *reader, component::IRegisterWriteStrategy *writer,
			  bool readBack, const QString &deviceName, QObject *parent = nullptr);

	RegisterMapDataStore *store() const;
	bool readBack() const;

	// A pending register gets no other call: these skip it (logged) and return Error -EBUSY.
	// The WhenIdle / waitWhenBusy variants (the JS API) wait until it is free, then run.
	QCoro::Task<component::RegisterData> read(uint32_t address);
	QCoro::Task<component::RegisterData> readWhenIdle(uint32_t address);
	// from start to end, in one strategy call
	QCoro::Task<QVector<component::RegisterData>> readRange(uint32_t start, uint32_t end,
								bool waitWhenBusy = false);

	// returns the write's results; only the read-back goes to the store
	QCoro::Task<component::RegisterData> writeThenRead(uint32_t address, uint32_t value);
	QCoro::Task<component::RegisterData> writeThenReadWhenIdle(uint32_t address, uint32_t value);
	// every "<address>,<value>" line of the file, in file order, in one write range
	QCoro::Task<void> writeFromFile(QString path, bool waitWhenBusy = false);
	// writes the Ok entries of the store to the file; with readFirst, that range is read first
	QCoro::Task<void> dump(QString path, std::optional<QPair<uint32_t, uint32_t>> readFirst = std::nullopt);

	// "<message> (<code>)", as in the status bar
	static QString errorText(const scopy::Error &error);

private:
	QCoro::Task<QVector<component::RegisterData>> readAll(QVector<uint32_t> addresses, bool waitWhenBusy);
	QCoro::Task<QVector<component::RegisterData>> writeAll(QVector<component::RegisterData> values,
							       bool waitWhenBusy);
	void reportErrors(const QString &operation, const QVector<component::RegisterData> &results);
	void writeDumpFile(const QString &path);

	QPointer<component::IRegisterReadStrategy> m_reader;
	QPointer<component::IRegisterWriteStrategy> m_writer;
	bool m_readBack;
	QString m_deviceName;
	RegisterMapDataStore *m_store;
};
} // namespace scopy::regmap
#endif // REGISTERMAPACCESS_HPP
