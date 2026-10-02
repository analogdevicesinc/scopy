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

#include <QList>
#include <QString>

namespace scopy {
namespace acq {

// Transforms DataStore streams: reads its watchedKeys, writes derived keys.
//
// The engine runs process() once per cycle, but only after every watched key has
// been written that cycle — that ordering constraint is the whole scheduling
// contract, so a processor chain is expressed purely by which keys each link
// watches. A processor whose inputs never arrive is skipped for the cycle.
//
// process() runs on the worker thread. Configuration set from the GUI must
// therefore be either atomic or mutex-guarded.
class SCOPY_CORE_EXPORT ProcessorBlock : public Block
{
	Q_OBJECT
public:
	explicit ProcessorBlock(const QString &name, QObject *parent = nullptr);

	virtual const QList<DataKey> &watchedKeys() const { return m_watchedKeys; }
	virtual void setWatchedKeys(const QList<DataKey> &keys) { m_watchedKeys = keys; }

	virtual void process(DataStore *store) = 0;

	// Called before each run so stateful processors can drop carry-over.
	virtual void reset() {}

protected:
	QList<DataKey> m_watchedKeys;
};

} // namespace acq
} // namespace scopy
