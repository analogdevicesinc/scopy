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

#ifndef REGISTERMAPDATASTORE_HPP
#define REGISTERMAPDATASTORE_HPP

#include "scopy-regmap_export.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include <component/regmap/registerdata.h>

namespace scopy::regmap {

// The latest read result of every register of one device: data only, no calls.
// It holds the latest read value, not the live value: to get a fresh value, read the register.
class SCOPY_REGMAP_EXPORT RegisterMapDataStore : public QObject
{
	Q_OBJECT
public:
	explicit RegisterMapDataStore(QObject *parent = nullptr);

	// nullptr if the register was never read and isn't pending. The entry is updated in place: use it, don't
	// keep it (it goes with clear() and with the store).
	const component::RegisterData *get(uint32_t address) const;
	// every address that has an entry, in address order
	QList<uint32_t> addresses() const;

	bool isPending(uint32_t address) const;
	bool isAnyPending(const QVector<uint32_t> &addresses) const;
	// false, and nothing changed, if any of them is already pending. The entries keep their value, error and
	// completedAt; their status becomes Pending.
	bool setPending(const QVector<uint32_t> &addresses);
	// read results only (Ok or Error): dataChanged, then pendingChanged(false)
	void store(const QVector<component::RegisterData> &results);
	// a call that stored nothing (a failed write, no read-back): the pending entries go back to what they were
	void restore(const QVector<uint32_t> &addresses);
	void clear();

Q_SIGNALS:
	// a read result (Ok or Error) was stored for these registers
	void dataChanged(QList<uint32_t> addresses);
	// these registers started (true) or stopped (false) waiting for a call
	void pendingChanged(QList<uint32_t> addresses, bool pending);

private:
	// QMap nodes don't move when other addresses are added, so get() stays valid
	QMap<uint32_t, component::RegisterData> m_data;
	// the entry each pending register had before its call; nullopt: it had none
	QHash<uint32_t, std::optional<component::RegisterData>> m_before;
};
} // namespace scopy::regmap
#endif // REGISTERMAPDATASTORE_HPP
