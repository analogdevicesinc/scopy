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

#ifndef ACQCHANNELREGISTRY_H
#define ACQCHANNELREGISTRY_H

#include "acqchannel.h"

#include <core/acq_engine/SampleBuffer.h>

#include <functional>

namespace scopy {
namespace adc {

// ReprKind -> a factory, so the manager creates a channel of a kind it has never
// heard of. This is a table, not a hierarchy of factory classes: every kind maps
// 1:1 to exactly one ReprKind, so there is nothing to resolve — no priorities, no
// tie-breaks, and no "figure out the kind from the SampleType" path, because
// ReprKind has no Auto value to resolve (see SampleBuffer.h). Registration is
// therefore just an insert.
//
// The test for the whole design: adding a fifth kind is a new .cpp with one
// REGISTER_ACQ_CHANNEL_KIND at file scope, and no edit to acqplotmanager.cpp.
class AcqChannelRegistry
{
public:
	using Factory = std::function<AcqChannel *(const AcqChannel::Args &)>;

	static AcqChannelRegistry &instance();

	// Registers a kind. There is no declared set of SampleTypes a kind will accept: a
	// kind pointed at a stream it cannot read draws nothing — every conversion
	// (toFloat, toBits, latestAs) returns empty and each kind's readData returns early
	// on that — which for a debug instrument is a better answer than a table deciding
	// in advance which combinations are worth trying.
	//
	// Registering an already-registered kind replaces it, so a plugin can substitute
	// its own curve implementation. Last registration wins, and static-init order
	// across translation units is unspecified — so do not register two factories for
	// one kind and expect a particular winner.
	void registerKind(scopy::acq::ReprKind kind, Factory f);

	// Null when the kind was never registered.
	AcqChannel *create(scopy::acq::ReprKind kind, const AcqChannel::Args &args) const;

	bool isRegistered(scopy::acq::ReprKind kind) const;

private:
	AcqChannelRegistry() = default;

	// QHash<int, ...> rather than QHash<ReprKind, ...> so QHash needs no qHash overload
	// for the enum.
	QHash<int, Factory> m_kinds;
};

// Registers a kind from the subclass's own .cpp, at static-init time, so the
// manager neither includes the subclass's header nor names its type.
//
//   REGISTER_ACQ_CHANNEL_KIND(AcqCurveChannel, ReprKind::Curve)
//
// Use at file scope in a .cpp only: in a header it would register once per including
// translation unit.
#define REGISTER_ACQ_CHANNEL_KIND(Class, Kind)                                                                         \
	namespace {                                                                                                    \
	const bool s_registered_##Class = []() {                                                                       \
		scopy::adc::AcqChannelRegistry::instance().registerKind(                                               \
			Kind,                                                                                          \
			[](const scopy::adc::AcqChannel::Args &a) -> scopy::adc::AcqChannel * {                        \
				return new Class(a);                                                                   \
			});                                                                                            \
		return true;                                                                                           \
	}();                                                                                                           \
	}

} // namespace adc
} // namespace scopy

#endif // ACQCHANNELREGISTRY_H
