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

#pragma once

#include "scopy-core_export.h"

#include "block.h"
#include "datakey.h"
#include "datastore.h"

#include <atomic>
#include <QList>
#include <QMutex>
#include <QString>

namespace scopy {
namespace acq {

// Transforms DataStore streams: reads its watchedKeys, writes derived keys.
//
// The engine runs process() once per cycle, after every watched key has been
// written that cycle, so a processor chain is expressed purely by which keys each
// link watches. A processor whose inputs never arrive is skipped for the cycle.
//
// process() runs on the worker thread; configuration set from the GUI must be
// atomic or mutex-guarded. The watched-key list already is: read it through
// watchedKeys(), which returns a snapshot.
class SCOPY_CORE_EXPORT ProcessorBlock : public Block
{
	Q_OBJECT
public:
	explicit ProcessorBlock(const QString &name, QObject *parent = nullptr);

	// Thread-safe snapshot. Safe to call from any thread while running.
	QList<DataKey> watchedKeys() const;
	virtual void setWatchedKeys(const QList<DataKey> &keys) { storeWatchedKeys(keys); }

	virtual void process(DataStore *store) = 0;

	// Called on the worker before each run so stateful processors can drop carry-over.
	virtual void reset() {}

	// Run after every other processor in the cycle, so this one observes everything
	// they wrote — e.g. a trigger snapshotting the whole store.
	bool runsLast() const { return m_runsLast.load(std::memory_order_relaxed); }
	void setRunsLast(bool on) { m_runsLast.store(on, std::memory_order_relaxed); }

protected:
	// The non-virtual store behind setWatchedKeys(), for subclasses whose override
	// does more than assign.
	void storeWatchedKeys(const QList<DataKey> &keys);

private:
	mutable QMutex m_keysMutex;
	QList<DataKey> m_watchedKeys;
	std::atomic<bool> m_runsLast{false};
};

} // namespace acq
} // namespace scopy
