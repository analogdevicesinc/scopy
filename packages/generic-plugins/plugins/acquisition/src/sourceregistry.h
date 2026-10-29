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

#ifndef ACQ_SOURCEREGISTRY_H
#define ACQ_SOURCEREGISTRY_H

#include <QList>

#include <functional>

class QObject;

namespace scopy {
namespace acq {
class SourceBlock;
} // namespace acq

namespace component {
class Context;
} // namespace component

namespace adc {

// Builds one source block, parented to `parent` — which the host makes the AcquisitionEngine,
// so the block can reach the store through Block::engineStore().
//
// `ctx` is an already-open device-controller context, owned by component::Controller: a factory
// must not open it, close it, or take ownership. It is null when the instrument was opened
// without one. A factory hands it to the block regardless and never probes: whether the device
// is there is the block's answer, given through SourceBlock::isAvailable(), so a device name is
// written once in the whole codebase.
using AcqSourceFactory = std::function<scopy::acq::SourceBlock *(scopy::component::Context *ctx, QObject *parent)>;

// Every source the plugin knows how to build. Each source fills this at static init from its
// own .cpp; the instrument's setup calls each factory once and keeps the blocks that report
// themselves available.
//
// Registering is one statement at file scope in the source block's own .cpp:
//
//   static const bool s_dcRegistered = AcqSourceRegistry::instance().add(
//           [](scopy::component::Context *ctx, QObject *parent) -> scopy::acq::SourceBlock * {
//                   return new DcSource(ctx, "dc", parent);
//           });
//
// The test for the design: adding a source is that block plus nothing else — no edit here, none
// to acqinstrumentcontroller.cpp and none to CMake, which globs src/*.cpp.
class AcqSourceRegistry
{
public:
	static AcqSourceRegistry &instance();

	// Returns whether it was taken, so the caller can be a static initializer. Rejects an
	// empty factory.
	bool add(AcqSourceFactory f);

	// Registration order, which is static-init order and so link order. Stable for a build
	// but not meaningful — nothing here depends on the sequence.
	const QList<AcqSourceFactory> &all() const;

private:
	AcqSourceRegistry() = default;

	QList<AcqSourceFactory> m_sources;
};

} // namespace adc
} // namespace scopy

#endif // ACQ_SOURCEREGISTRY_H
