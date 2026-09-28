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

// Registers the two sources that cannot register themselves: PlutoIIOSource lives in src/sim/,
// shared with the sim instrument, and SnapshotSource lives in core/ — neither should know about
// this plugin's registry. Every source under src/acq/ registers at the bottom of its own .cpp
// instead, so nothing new belongs here unless it is outside this directory too.

#include "PlutoIIOSource.h"
#include "sourceregistry.h"

#include <core/acq_engine/SnapshotSource.h>

using namespace scopy;
using namespace scopy::adc;

// The genalyzer FFT over this source's I/Q is not here: a processor is the instrument's to
// register, and the registry describes sources.
static const bool s_plutoSourceRegistered =
	AcqSourceRegistry::instance().add([](iio_context *ctx, QObject *parent) -> scopy::acq::SourceBlock * {
		auto *src = new sim::PlutoIIOSource(ctx, QStringLiteral("pluto"),
						   QStringLiteral("cf-ad9361-lpc"), parent);
		// The block registers no channels of its own, so I and Q are named here.
		src->enableChannel(QStringLiteral("voltage0"), true);
		src->enableChannel(QStringLiteral("voltage1"), true);
		return src;
	});

// No isAvailable() override on the block: this source freezes streams that already exist, so it
// needs no device and ignores the context.
static const bool s_snapshotSourceRegistered =
	AcqSourceRegistry::instance().add([](iio_context *, QObject *parent) -> scopy::acq::SourceBlock * {
		auto *src = new scopy::acq::SnapshotSource(QStringLiteral("snapshot"), parent);
		// One empty slot, so the panel opens on something to configure.
		src->addSlot();
		return src;
	});
