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

// Registers the sources that cannot register themselves. SnapshotSource lives in core/ and
// should not know about this plugin's registry; every source under src/ registers at the bottom
// of its own .cpp instead.

#include "sourceregistry.h"

#include <core/acq_engine/snapshotsource.h>

using namespace scopy;
using namespace scopy::adc;

// No isAvailable() override on the block: this source freezes streams that already exist, so it
// needs no device and ignores the context.
static const bool s_snapshotSourceRegistered = AcqSourceRegistry::instance().add(
	[](scopy::component::Context *, QObject *parent) -> scopy::acq::SourceBlock * {
		auto *src = new scopy::acq::SnapshotSource(QStringLiteral("snapshot"), parent);
		// One empty slot, so the panel opens on something to configure.
		src->addSlot();
		return src;
	});
